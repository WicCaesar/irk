/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/15 19:47:56 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:36:38 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"
#include <vector>

void	Server::invite_utils(std::string &command, int &fd) {
	// Splits the command in individual parts. If less than 3, sends error 461.
	std::vector<std::string> kintsugi = split_command(command);
	if (kintsugi.size() < 3) {
		respond(ERR_NEEDMOREPARAMS(get_client_by_fd(fd)->get_displayname()), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(461, " :Not enough parameters\r\n", get_client_by_fd(fd)->get_displayname(), fd);
		return ;
	};

	// If the channel does not exist, sends error 403.
	std::string channel = kintsugi[2].substr(1);
	if (kintsugi[2][0] != '#' || get_channel_by_name(channel) == NULL) {
		respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), channel), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(403, " :No such channel\r\n", channel, fd);
		return ;
	};

	// If the inviter is not a member of the channel, sends error 442.
	if (!(get_channel_by_name(channel)->get_client_by_fd(fd)) 
	&& !(get_channel_by_name(channel)->get_admin_by_fd(fd))) {
		respond(ERR_NOTONCHANNEL(get_client_by_fd(fd)->get_displayname(), channel), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(442, " :You're not on that channel\r\n", channel, fd);
		return ;
	};

	Client *client = get_client_by_name(kintsugi[1]);
	// If the invited user does not exist, sends error 401.
	if (!client) {
		respond(ERR_NOSUCHNICK(channel, kintsugi[1]), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(401, kintsugi[1], channel, fd);
		return ;
	};

	// If the invitated user is already a member, sends error 443.
	if (get_channel_by_name(channel)->get_client_by_name(client->get_displayname())) {
		respond(ERR_USERONCHANNEL(get_client_by_fd(fd)->get_displayname(), channel), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(443, " :is already on channel\r\n", channel, fd);
		return ;
	};

	// If the channel is private, but the inviter is not an operator, error 482.
	if (get_channel_by_name(channel)->isprivate() == true 
	&& !get_channel_by_name(channel)->get_admin_by_fd(fd)) {
		respond(ERR_CHANOPRIVSNEEDED(get_client_by_fd(fd)->get_displayname(), channel), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(482, " :You're not channel operator\r\n", channel, fd);
		return ;
	};

	// If the channel is already full, sends error 471.
	if (get_channel_by_name(channel)->get_max_population() 
		&& get_channel_by_name(channel)->get_population() >= get_channel_by_name(channel)->get_max_population()) {
		respond(ERR_CHANNELISFULL(get_client_by_fd(fd)->get_displayname(), channel), fd);
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(471, " :Cannot join channel (+l)\r\n", channel, fd);
		return ;
	};

	//TODO COMPARAR SE RESULTADO ESTÁ SATISFATÓRIO
	// Sends invitation after all the checks, and responds 341.
	client->invite_to_channel(channel);
	respond(RPL_INVITING(get_client_by_fd(fd)->get_displayname(), channel), fd);	
};
