/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/21 07:11:32 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:36:38 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"
#include <string>

// Sends a private message to a list of users and channels.
void	Server::privmsg(std::string command, int fd) {
	std::vector<std::string>	temp;
	std::string					proto = privmsg_utils(command, temp);

	// If there's no receiver, sends error 411.
	if (temp.size() == 0) {
		respond(ERR_NORECIPIENT(get_client_by_fd(fd)->get_displayname(), command), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(411, get_client_by_fd(fd)->get_displayname(), fd, " :No recipient given (PRIVMSG)\r\n");
		return ;
	};

	// If there's no message, sends error 412.
	if (proto.empty()) {
		respond(ERR_NOTEXTTOSEND(get_client_by_fd(fd)->get_displayname()), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(412, get_client_by_fd(fd)->get_displayname(), fd, " :No text to send\r\n");
		return ;
	};

	validate_receivers(temp, fd);
	for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i][0] == '#') {
			temp[i].erase(temp[i].begin());
			std::string	message = ":" + get_client_by_fd(fd)->get_displayname() + "!~" + get_client_by_fd(fd)->get_login() + "@localhost PRIVMSG #" + temp[i] + " :" + proto + CRLF;
			get_channel_by_name(temp[i])->relay(message, fd);
		} else {
			std::string	message = ":" + get_client_by_fd(fd)->get_displayname() + "!~" + get_client_by_fd(fd)->get_login() + "@localhost PRIVMSG " + temp[i] + " :" + proto + CRLF;
			get_channel_by_name(temp[i])->relay(message, fd);
		};
	};
};

// A series of parsing functions to isolate the message and the receivers.
std::string	Server::privmsg_utils(std::string command, std::vector<std::string> &temp) {
	// Isolates the message from the rest of the command.
	std::string	message = get_message(command, temp, 2);

	if (temp.size() != 2) {
		temp.clear();
		return (std::string(""));
	};
	temp.erase(temp.begin());
	std::string	temp_list = temp[0];	// Another temporary string for parsing.
	std::string	receivers;	// A string that will store the parsed receivers.
	temp.clear();

	// Splits the first string by ',' to get the channel names.
	for (size_t i = 0; i < temp_list.size(); i++) {
		if (temp_list[i] == ',') {
			temp.push_back(receivers);
			receivers.clear();
		} else
			receivers += temp_list[i];
	};

	// Removes empty strings.
	for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i].empty())
			temp.erase(temp.begin() + i--);
	};

	// Removes the colon.
	if (message[0] == ':')
		message.erase(message.begin());
	else { // Or shrinks to the first space.
		for (size_t i = 0; i < message.size(); i++) {
			if (message[i] == ' ') {
				message = message.substr(0, i);
				break;
			};
		};
	};
	return (message);
};

// Checks if the users and channels that were passed as arguments can receive the message.
void	Server::validate_receivers(std::vector<std::string> &temp, int fd) {
	for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i][0] == '#') {
			temp[i].erase(temp[i].begin());
			// If this channel doesn't exist, sends error 403.
			if (!get_channel_by_name(temp[i])) {
				respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), temp[i]), fd);
				//TODO UM OU OUTRO, TESTAR O DE CIMA
				//senderror(403, temp[i], get_client_by_fd(fd)->get_displayname(), " :No such nick/channel\r\n");
				temp.erase(temp.begin() + i);
				i--;
			// If the client is not a member of the channel, sends error 404.
			} else if (!get_channel_by_name(temp[i])->get_client_by_name(get_client_by_fd(fd)->get_displayname())) {
				respond(ERR_CANNOTSENDTOCHAN(get_client_by_fd(fd)->get_displayname(), temp[i]), fd);
				//TODO UM OU OUTRO, TESTAR O DE CIMA
				//senderror(404, get_client_by_fd(fd)->get_displayname(), "#" + temp[i], fd, " :Cannot send to channel\r\n");
				temp.erase(temp.begin() + i);
				i--;
			} else
				temp[i] = "#" + temp[i];
		} else {
			// If the client doesn't exist, sends error 401.
			if (!get_client_by_name(temp[i])) {
				respond(ERR_NOSUCHNICK(temp[i], get_client_by_fd(fd)->get_displayname()), fd);
				//TODO UM OU OUTRO, TESTAR O DE CIMA
				//senderror(401, temp[i], get_client_by_fd(fd)->get_displayname(), " :No such nick/channel\r\n");
				temp.erase(temp.begin() + i);
				i--;
			};
		};
	};
};
