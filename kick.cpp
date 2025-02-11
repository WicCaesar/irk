/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 03:09:19 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:36:38 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"

void	Server::kick(std::string command, int fd) {
	std::string					user;
	std::string					reason;
	std::vector<std::string>	temp;

	reason = kick_utils(command, temp, user, fd);
	// If the user is not provided, sends error 461.
	if (user.empty()) {
		respond(ERR_NEEDMOREPARAMS(get_client_by_fd(fd)->get_displayname()), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(461, " :Not enough parameters\r\n", get_client_by_fd(fd)->get_displayname(), fd);
		return ;
	};
	for (size_t i = 0; i < temp.size(); i++) {
		if (get_channel_by_name(temp[i])) {
			Channel	*channel = get_channel_by_name(temp[i]);
			// Checks if the client is in the channel.
			if (!channel->get_client_by_fd(fd) && !channel->get_admin_by_fd(fd)) {
				respond(ERR_NOTONCHANNEL(get_client_by_fd(fd)->get_displayname(), channel->get_name()), fd);
				//TODO UM OU OUTRO, TESTAR O DE CIMA
				//senderror(442, ":You're not on that channel\r\n", get_client_by_fd(fd)->get_displayname(), channel->get_name(), fd);
				continue;
			};
			// If the client is an admin of the channel, removes them.
			if (channel->get_admin_by_fd(fd)) {
				// Checks if the member to be expelled is in the channel.
				if (channel->get_client_by_name(user)) {
					// Informs the room about the kick.
					std::stringstream	stream;
					stream << ":" << get_client_by_fd(fd)->get_displayname() 
						<< "!~" << get_client_by_fd(fd)->get_displayname() 
						<< "@localhost KICK #" << temp[i] << " " << user;
					if (reason.empty() == false)
						stream << " :" << reason << "\r\n";
					else
						stream << "\r\n";
					channel->relay(stream.str());

					// Finally removes the client.
					if (channel->get_admin_by_fd(channel->get_client_by_name(user)->get_fd()))
						channel->remove_admin(channel->get_client_by_name(user)->get_fd());
					else
						channel->remove_client(channel->get_client_by_name(user)->get_fd());

					// If the channel is now totally empty, removes it entirely.
					if (channel->get_population() == 0)
						this->channel_list_.erase(this->channel_list_.begin() + i);
				} else {
					respond(ERR_USERNOTINCHANNEL(get_client_by_fd(fd)->get_displayname(), temp[i]), get_client_by_fd(fd)->get_fd());
					//TODO UM OU OUTRO, TESTAR O DE CIMA
					//senderror(441, ":They aren't on that channel\r\n", get_client_by_fd(fd)->get_displayname(), "#" + temp[i], get_client_by_fd(fd)->get_fd());
					continue;
				};
			} else {
				respond(ERR_CHANOPRIVSNEEDED(get_client_by_fd(fd)->get_displayname(), temp[i]), get_client_by_fd(fd)->get_fd());
				//TODO UM OU OUTRO, TESTAR O DE CIMA
				//senderror(482, " :You're not channel operator\r\n", get_client_by_fd(fd)->get_displayname(), "#" + temp[i], get_client_by_fd(fd)->get_fd());
				continue;
			};
		} else
			respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), temp[i]), get_client_by_fd(fd)->get_fd());
			//TODO UM OU OUTRO, TESTAR O DE CIMA
			//senderror(403, " :No such channel\r\n", get_client_by_fd(fd)->get_displayname(), "#" + temp[i], get_client_by_fd(fd)->get_fd());
	};
};

std::string	Server::kick_utils(std::string command, std::vector<std::string> &temp, std::string &user, int fd) {
	// Isolates the reason from the rest of the command.
	std::string reason = get_message(command, temp, 3);

	// If the command is not complete, returns an empty string.
	if (temp.size() < 3)
		return (std::string(""));
	temp.erase(temp.begin());
	std::string string = temp[0];
	std::string channels;
	user = temp[1];
	temp.clear();

	// Splits the first string by ',' to get the channel names.
	for (size_t i = 0; i < string.size(); i++) {
		if (string[i] == ',') {
			temp.push_back(channels);
			channels.clear();
		} else
			channels += string[i];
	};
	temp.push_back(channels);
	// Removes empty strings.
	for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i].empty())
			temp.erase(temp.begin() + i--);
	};

	// Removes the colon.
	if (reason[0] == ':')
		reason.erase(reason.begin());
	else { // Or shrinks to the first space.
		for (size_t i = 0; i < reason.size(); i++) {
			if (reason[i] == ' ') {
				reason = reason.substr(0, i);
				break;
			};
		};
	};

	// Checks if the channels are all valid.
	for (size_t i = 0; i < temp.size(); i++) {
		if (*(temp[i].begin()) == '#')
			temp[i].erase(temp[i].begin());
		else {
			respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), temp[i]), fd);
			//TODO TESTAR O DE CIMA
			//senderror(403, " :No such channel\r\n", get_client_by_fd(fd)->get_displayname(), temp[i], fd);
			temp.erase(temp.begin() + i--);
		};
	};
	return (reason);	
};
