/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 02:55:09 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:36:38 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"

void	Server::topic_utils(std::string &command, int &fd) {
	if (command == "TOPIC ") {
		respond(ERR_NEEDMOREPARAMS(get_client_by_fd(fd)->get_displayname()), fd);
		return ;
	};

	std::vector<std::string> shards = split_command(command);

	if (shards.size() == 1) {
		respond(ERR_NEEDMOREPARAMS(get_client_by_fd(fd)->get_displayname()), fd);
		return ;
	};

	std::string	channel = shards[1].substr(1);

	// If the channel doesn't exist, sends error 403.
	if (!get_channel_by_name(channel)) {
		respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), channel), fd);
		return ;
	};

	// If the client isn't on the channel, sends error 442.
	if (!get_channel_by_name(channel)->get_client_by_fd(fd) && !get_channel_by_name(channel)->get_admin_by_fd(fd)) {
		respond(ERR_NOTONCHANNEL(get_client_by_fd(fd)->get_displayname(), channel), fd);
		return ;
	};

	// If the command has only two parts, checks if it's already set.
	if (shards.size() == 2) {
		// If not set, sends error 331.
		if (get_channel_by_name(channel)->get_topic_name() == "") {
			respond(RPL_NOTOPIC(get_client_by_fd(fd)->get_displayname(), channel), fd);
			return ;
		};

		// If set, displays the topic, the user who set it and the time.
		size_t position = get_channel_by_name(channel)->get_topic_name().find(":");
		if (get_channel_by_name(channel)->get_topic_name() != "" && position == std::string::npos) {
			respond(RPL_TOPIC(get_client_by_fd(fd)->get_displayname(), channel, get_channel_by_name(channel)->get_topic_name()), fd);
			respond(RPL_TOPICWHOTIME(get_client_by_fd(fd)->get_displayname(), channel, get_channel_by_name(channel)->get_topic_name()), fd);
			return ;
		} else {
			// If the topic name starts with spaces, removes them.
			size_t position = get_channel_by_name(channel)->get_topic_name().find(" ");
			if (position == 0)
				get_channel_by_name(channel)->get_topic_name().erase(0, 1);
			respond(RPL_TOPIC(get_client_by_fd(fd)->get_displayname(), channel, get_channel_by_name(channel)->get_topic_name()), fd);
			respond(RPL_TOPICWHOTIME(get_client_by_fd(fd)->get_displayname(), channel, get_channel_by_name(channel)->get_topic_name()), fd);
			return ;
		};
	};

	// If the command has more than two parts, sets the topic.
	if (shards.size() >= 3) {
		// Creates a temporary vector to store the command parts.
		std::vector<std::string> temp;
		int	position = get_position(command);
		if (position == -1 || shards[2][0] != ':') {
			temp.push_back(shards[0]);
			temp.push_back(shards[1]);
			temp.push_back(shards[2]);
		} else {
			temp.push_back(shards[0]);
			temp.push_back(shards[1]);
			temp.push_back(command.substr(get_position(command)));
		};

		// If the topic is nothing other than ":", sends error 331.
		if (temp[2][0] == ':' && temp[2][1] == '\0') {
			respond(RPL_NOTOPIC(get_client_by_fd(fd)->get_displayname(), channel), fd);
			return ;
		};

		// If the topic restricted mode is activated and the client isn't an operator, sends error 482.
		if (get_channel_by_name(channel)->istopic_restricted() && get_channel_by_name(channel)->get_client_by_fd(fd)) {
			respond(ERR_CHANOPRIVSNEEDED(get_client_by_fd(fd)->get_displayname(), channel), fd);
			return ; // If it's topic restricted and the client is operator, defines topic.
		} else if (get_channel_by_name(channel)->istopic_restricted() && get_channel_by_name(channel)->get_admin_by_fd(fd)) {
			get_channel_by_name(channel)->set_creation_given(get_time());
			get_channel_by_name(channel)->set_topic_name(temp[2]);
			std::string	response;
			size_t		position = temp[2].find(":");
			if (position == std::string::npos)
				response = ":" + get_client_by_fd(fd)->get_displayname() + "!" + get_client_by_fd(fd)->get_login() + "@localhost TOPIC #" + channel + " :" + get_channel_by_name(channel)->get_topic_name() + CRLF;
			else
				response = ":" + get_client_by_fd(fd)->get_displayname() + "!" + get_client_by_fd(fd)->get_login() + "@localhost TOPIC #" + channel + " " + get_channel_by_name(channel)->get_topic_name() + CRLF;
			get_channel_by_name(channel)->relay(response);
		} else {
			std::string	response;
			size_t		position = temp[2].find(":");
			if (position == std::string::npos) {
				get_channel_by_name(channel)->set_creation_given(get_time());
				get_channel_by_name(channel)->set_topic_name(temp[2]);
				response = ":" + get_client_by_fd(fd)->get_displayname() + "!" + get_client_by_fd(fd)->get_login() + "@localhost TOPIC #" + channel + " " + get_channel_by_name(channel)->get_topic_name() + CRLF;
			} else {
				size_t	position = temp[2].find(" ");
				get_channel_by_name(channel)->set_topic_name(temp[2]);
				if (position == std::string::npos && temp[2][0] == ':' && temp[2][1] != ':')
					temp[2] = temp[2].substr(1);
				get_channel_by_name(channel)->set_topic_name(temp[2]);
				get_channel_by_name(channel)->set_creation_given(get_time());
				response = ":" + get_client_by_fd(fd)->get_displayname() + "!" + get_client_by_fd(fd)->get_login() + "@localhost TOPIC #" + channel + " " + get_channel_by_name(channel)->get_topic_name() + CRLF;
			};
			get_channel_by_name(channel)->relay(response);
		};
	};
};

std::string	Server::get_topic(std::string &input) {
	size_t	position = input.find(":");
	if (position == std::string::npos)
		return ("");
	return (input.substr(position));
};

// Sets the channel's creation based in seconds since 1970. E.g.: "1643079273".
std::string	Server::get_time(void) {
	std::ostringstream stream;
	std::time_t	now = std::time(0);

	stream << now; // Converts the time to a string.
	return (stream.str());
};

int	Server::get_position(std::string &command) {
	for (int i = 0; i < (int)command.size(); i++) {
		if (command[i] == ':' && command[i - 1] == ' ')
			return (i);
	};
	return (-1);
};
