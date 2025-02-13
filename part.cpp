/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/25 13:06:59 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:36:38 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"

void	Server::part(std::string command, int fd) {
	std::vector<std::string>	temp;
	std::string					reason;

	if (!part_utils(command, temp, reason, fd)) {
		respond(ERR_NEEDMOREPARAMS(get_client_by_fd(fd)->get_displayname()), fd);
		return ;
	};

	// Searches the user in each channel.
	for (size_t i = 0; i < temp.size(); i++) {
		bool	flag = false;
		for (size_t j = 0; j < this->channel_list_.size(); j++) {
			// Checks if channel exists.
			if (this->channel_list_[j].get_name() == temp[i]) {
				flag = true;
				// If the client is not a member, moves on.
				if (!this->channel_list_[j].get_client_by_fd(fd) && !this->channel_list_[j].get_admin_by_fd(fd)) {
					respond(ERR_NOTONCHANNEL(get_client_by_fd(fd)->get_displayname(), this->channel_list_[j].get_name()), fd);
					continue;
				};
				// Sends parting message to the channel.
				std::stringstream	stream;
				stream << ":" << get_client_by_fd(fd)->get_displayname() 
					   << "!~" << get_client_by_fd(fd)->get_displayname() 
					   << "@ localhost PART #" << temp[i];
				if (reason.empty() == false)
					stream << " :" << reason << "\r\n";
				else
					stream << "\r\n";
				this->channel_list_[j].relay(stream.str());

				// Finally removes the client.
				if (this->channel_list_[j].get_admin_by_fd(channel_list_[j].get_client_by_name(get_client_by_fd(fd)->get_displayname())->get_fd()))
					channel_list_[j].remove_admin(channel_list_[j].get_client_by_name(get_client_by_fd(fd)->get_displayname())->get_fd());
				else
					channel_list_[j].remove_client(channel_list_[j].get_client_by_name(get_client_by_fd(fd)->get_displayname())->get_fd());

				// If the channel is now totally empty, removes it entirely.
				if (channel_list_[j].get_population() == 0)
					this->channel_list_.erase(channel_list_.begin() + j);
			};
		};

		// If the channel does not exist, sends error 403.
		if (flag == false)
			respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), temp[i]), fd);
	};	
};

bool	Server::part_utils(std::string command, std::vector<std::string> &temp, std::string &reason, int fd) {
	// Isolates the reason from the rest of the command.
	reason = get_message(command, temp, 2);

	// If the command is not complete, returns false.
	if (temp.size() < 2) {
		temp.clear();
		return (false);
	};
	temp.erase(temp.begin());
	std::string	string = temp[0];
	std::string	channels;
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

	// Checks if the channels are all valid. Otherwise, sends error 403.
	for (size_t i = 0; i < temp.size(); i++) {
		if (*(temp[i].begin()) == '#')
			temp[i].erase(temp[i].begin());
		else {
			respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), temp[i]), fd);
			temp.erase(temp.begin() + i--);
		};
	};
	return (true);
};
