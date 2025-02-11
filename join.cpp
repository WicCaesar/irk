/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/25 05:34:26 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:37:10 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"
#include <sstream>

// Initial handling of the JOIN command.
void	Server::join_utils(std::string command, int fd) {
	std::vector<std::pair<std::string, std::string> > kintsugi;

	if (pairing(kintsugi, command, fd) == false) {
		respond(ERR_NEEDMOREPARAMS(get_client_by_fd(fd)->get_displayname()), get_client_by_fd(fd)->get_fd());
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(461, " :Not enough parameters\r\n", get_client_by_fd(fd)->get_displayname(), get_client_by_fd(fd)->get_fd());
		return ;
	};

	// Mustn't enter more than 10 channels at once.
	//TODO AQUI É PARA TER O NOME DO CANAL QUE ESTÁ TENTANDO ENTRAR. COMO FAZER ISSO? OBSERVAR OUTRO JÁ PRONTO.
	//TODO TESTAR SE ESTÁ DE ACORDO COM A DESCRIÇÃO: Sent to a user when they have joined the maximum number of allowed channels and they try to join another channel.
	if (kintsugi.size() > 10) {
		respond(ERR_TOOMANYCHANNELS(get_client_by_fd(fd)->get_displayname()), get_client_by_fd(fd)->get_fd());
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(405, " :You have joined too many channels\r\n", get_client_by_fd(fd)->get_displayname(), get_client_by_fd(fd)->get_fd());
	};

	for (size_t i = 0; i < kintsugi.size(); i++) {
		bool flag = false;
		for (size_t j = 0; j < this->channel_list_.size(); j++) {
			if (this->channel_list_[j].get_name() == kintsugi[i].first) {
				join_channel(kintsugi, i, j, fd);
				flag = true;
				break ;
			};
		};
		if (flag == false)
			create_channel(kintsugi, i, fd);
	};
};

/* Performs a series of actions to pair channels and respective passwords,
clean up the shards and check if the channel name is valid. */
int	Server::pairing(std::vector<std::pair<std::string, std::string> > &kintsugi, std::string command, int fd) {
	std::string					buffer;
	std::string					channel;
	std::string					password;
	std::vector<std::string>	temp;
	std::istringstream			stream(command);

	// Stores single shards in a vector.
	while (stream >> command)
		temp.push_back(command);

	// There must be at least two shards, JOIN and <channel>.
	// Otherwise, clears the original vector and returns 0.
	if (temp.size() < 2) {
		kintsugi.clear();
		return (0);
	};

	temp.erase(temp.begin()); // Removes JOIN.
	channel = temp[0]; // Stores the channel name.
	temp.erase(temp.begin()); // Removes the channel name.
	if (temp.size() > 0) { // If there's another shard, it must be a password.
		password = temp[0]; // Stores the password.
		temp.clear();
		// TODO TESTAR O DE CIMA (SUGESTÃO DO LULU) E O DE BAIXO (MEU)
		// temp.erase(temp.begin()); // Removes the password.
	};

	// Gets shards from channel, separated by commas. Adds them to the kintsugi.
	for (size_t i = 0; i < channel.size(); i++) {
		if (channel[i] == ',') {
			kintsugi.push_back(std::make_pair(buffer, ""));
			buffer.clear();
		} else
			buffer += channel[i];
	};
	kintsugi.push_back(std::make_pair(buffer, ""));

	// If there's a password, adds it to the kintsugi as well.
	if (password.empty() == false) {
		buffer.clear();
		size_t i = 0;
		for (size_t j = 0; j < password.size(); j++) {
			if (password[j] == ',') {
				kintsugi[i].second = buffer;
				i++;
				buffer.clear();
			} else
				buffer += password[j];
		};
		kintsugi[i].second = buffer;
	};

	// Removes empty shards.
	for (size_t i = 0; i < kintsugi.size(); i++) {
		if (kintsugi[i].first.empty())
			kintsugi.erase(kintsugi.begin() + i--);
	};

	// If the channel name doesn't start with a '#', removes the pair.
	for (size_t i = 0; i < kintsugi.size(); i++) {
		if (*(kintsugi[i].first.begin()) != '#') {
			respond(ERR_NOSUCHCHANNEL(get_client_by_fd(fd)->get_displayname(), kintsugi[i].first), get_client_by_fd(fd)->get_fd());
			//TODO UM OU OUTRO, TESTAR O DE CIMA
			//senderror(403, " :No such channel\r\n", get_client_by_fd(fd)->get_displayname(), kintsugi[i].first, get_client_by_fd(fd)->get_fd());
			kintsugi.erase(kintsugi.begin() + i--);
		} else
			kintsugi[i].first.erase(kintsugi[i].first.begin());
	};

	return (1);
};

void	Server::join_channel(std::vector<std::pair<std::string, std::string> > &kintsugi, int i, int j, int fd) {
	// If the client is already a member of the channel, does nothing.
	if (this->channel_list_[j].get_client_by_name(get_client_by_fd(fd)->get_displayname()))
		return ;

	// If the client is a member of 10 channels (limit), sends error 405.
	if (scour_presence(get_client_by_fd(fd)->get_displayname()) >= 10) {
		respond(ERR_TOOMANYCHANNELS(get_client_by_fd(fd)->get_displayname()), get_client_by_fd(fd)->get_fd());
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(405, " :You have joined too many channels\r\n", get_client_by_fd(fd)->get_displayname(), get_client_by_fd(fd)->get_fd());
		return ;
	};

	// If the channel has a password and it doesn't match, sends error 475.
	if (this->channel_list_[j].get_password().empty() == false 
		&& this->channel_list_[j].get_password() != kintsugi[i].second) {
		if (isinvited(get_client_by_fd(fd), kintsugi[i].first, 0) == false) {
			respond(ERR_BADCHANNELKEY(get_client_by_fd(fd)->get_displayname(), kintsugi[i].first), get_client_by_fd(fd)->get_fd());
			//TODO UM OU OUTRO, TESTAR O DE CIMA
			//senderror(475, " :Cannot join channel (+k)\r\n", get_client_by_fd(fd)->get_displayname(), "#" + kintsugi[i].first, get_client_by_fd(fd)->get_fd());
			return ;
		};
	};

	// If the channel is invite-only, but the client is not invited, sends error 473.
	if (this->channel_list_[j].isprivate() == true) {
		if (isinvited(get_client_by_fd(fd), kintsugi[i].first, 1) == false) {
			respond(ERR_INVITEONLYCHAN(get_client_by_fd(fd)->get_displayname(), kintsugi[i].first), get_client_by_fd(fd)->get_fd());
			//TODO UM OU OUTRO, TESTAR O DE CIMA
			//senderror(473, " :Cannot join channel (+i)\r\n", get_client_by_fd(fd)->get_displayname(), "#" + kintsugi[i].first, get_client_by_fd(fd)->get_fd());
			return ;
		};
	};

	// If the channel has reached its maximum population, sends error 471.
	if (this->channel_list_[j].get_max_population() 
		&& this->channel_list_[j].get_population() >= this->channel_list_[j].get_max_population()) {
		respond(ERR_CHANNELISFULL(get_client_by_fd(fd)->get_displayname(), kintsugi[i].first), get_client_by_fd(fd)->get_fd());
		// TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(471, " :Cannot join channel (+l)\r\n", get_client_by_fd(fd)->get_displayname(), "#" + kintsugi[i].first, get_client_by_fd(fd)->get_fd());
		return ;
	};

	//* If no problems were found, adds the client to the channel.
	Client	*client = this->get_client_by_fd(fd);
	this->channel_list_[j].add_client(*client);

	if (this->channel_list_[j].get_topic_name().empty() == false)
		respond(RPL_JOIN(get_client_by_fd(fd)->get_hostname(), get_client_by_fd(fd)->get_ip(), kintsugi[i].first) 
	+ RPL_TOPIC(get_client_by_fd(fd)->get_displayname(), channel_list_[j].get_name(), channel_list_[j].get_topic_name()) 
	+ RPL_NAMREPLY(get_client_by_fd(fd)->get_displayname(), channel_list_[j].get_name(), channel_list_[j].get_client_list()) 
	+ RPL_ENDOFNAMES(get_client_by_fd(fd)->get_displayname(), channel_list_[j].get_name()), fd);
	else
		respond(RPL_JOIN(get_client_by_fd(fd)->get_hostname(), get_client_by_fd(fd)->get_ip(), kintsugi[i].first) 
	+ RPL_NAMREPLY(get_client_by_fd(fd)->get_displayname(), channel_list_[j].get_name(), channel_list_[j].get_client_list()) 
	+ RPL_ENDOFNAMES(get_client_by_fd(fd)->get_displayname(), channel_list_[j].get_name()), fd);

	channel_list_[j].relay(RPL_JOIN(get_client_by_fd(fd)->get_hostname(), get_client_by_fd(fd)->get_ip(), kintsugi[i].first), fd);
};

// Scours every channel and tells in how many a client is present.
int	Server::scour_presence(std::string displayname) {
	int	count = 0;
	for (size_t i = 0; i < this->channel_list_.size(); i++) {
		if (this->channel_list_[i].get_client_by_name(displayname))
			count++;
	};
	return (count);
};

bool	Server::isinvited(Client *client, std::string channel, int flag) {
	if (client->get_invitation_status(channel)) {
		if (flag == 1)
			client->undo_invitation(channel);
		return (true);
	};
	return (false);
};

void	Server::create_channel(std::vector<std::pair<std::string, std::string> > &kintsugi, int i, int fd) {
	// If the client is a member of 10 channels (limit), sends error 405.
	if (scour_presence(get_client_by_fd(fd)->get_displayname()) >= 10) {
		respond(ERR_TOOMANYCHANNELS(get_client_by_fd(fd)->get_displayname()), get_client_by_fd(fd)->get_fd());
		//TODO UM OU OUTRO, TESTAR O DE CIMA
		//senderror(405, " :You have joined too many channels\r\n", get_client_by_fd(fd)->get_displayname(), get_client_by_fd(fd)->get_fd());
		return ;
	};

	/* If the user tries to join a channel that does not exist, 
	creates it and adds the client as operator. */
	Channel	new_channel;
	new_channel.set_name(kintsugi[i].first);
	new_channel.add_admin(*get_client_by_fd(fd));
	new_channel.set_creation_auto();
	this->channel_list_.push_back(new_channel);

	respond(RPL_JOIN(get_client_by_fd(fd)->get_hostname(), get_client_by_fd(fd)->get_ip(), kintsugi[i].first) 
	+ RPL_NAMREPLY(get_client_by_fd(fd)->get_displayname(), channel_list_[i].get_name(), channel_list_[i].get_client_list()) 
	+ RPL_ENDOFNAMES(get_client_by_fd(fd)->get_displayname(), channel_list_[i].get_name()), fd);
};
