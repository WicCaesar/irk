/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 00:25:22 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 10:58:48 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"

void	Server::quit(std::string command, int fd) {
	std::string	reason = get_message(command);

	for (size_t i = 0; i < channel_list_.size(); i++) {
		if (channel_list_[i].get_client_by_fd(fd)) {
			channel_list_[i].remove_client(fd);
			// If the channel is now totally empty, removes it entirely.
			if (channel_list_[i].get_population() == 0)
				channel_list_.erase(channel_list_.begin() + i);
			else {
				// Sends the QUIT message to all clients in the channel.
				std::string message = ":" + get_client_by_fd(fd)->get_displayname() + "!~" + get_client_by_fd(fd)->get_login() + "@localhost QUIT " + reason + "\r\n";
				channel_list_[i].relay(message);
			};
		} else if (channel_list_[i].get_admin_by_fd(fd)) {
			channel_list_[i].remove_admin(fd);
			// If the channel is now totally empty, removes it entirely.
			if (channel_list_[i].get_population() == 0)
				channel_list_.erase(channel_list_.begin() + i);
			else {
				// Sends the QUIT message to all clients in the channel.
				std::string message = ":" + get_client_by_fd(fd)->get_displayname() + "!~" + get_client_by_fd(fd)->get_login() + "@localhost QUIT " + reason + "\r\n";
				channel_list_[i].relay(message);
			};
		};
	};

	// Removes the client from the list and prints a message.
	remove_from_all_channels(fd);
	remove_client_from_list(fd);
	remove_fd_from_list(fd);	
	close(fd);
	std::cout << fd << " desconectou." << std::endl;
};
