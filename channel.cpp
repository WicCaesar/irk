/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/24 15:14:33 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 11:42:53 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "client.hpp"
#include "channel.hpp"

Channel::Channel(void) {
	this->key_ = 0;
	this->name_ = "";
	this->islocked_ = 0;
	this->max_population_ = 0;
	this->topic_ = 0;
	this->topic_name_ = "";
	this->topic_restriction_ = false;
	this->creation_given_ = "";
	char mode_list_[5] = {'l', 'o', 'k', 'i', 't'};
	// l is for to limit the amount of users in channels.
	// o is for operator privileges in channels.
	// k is for password-protected channels.
	// i is for invite-only channels.
	// t is for topic-restricted channels.
	for (int i = 0; i < 5; i++) {
		this->mode_list_.push_back(std::make_pair(mode_list_[i], false));
	};
};

Channel::Channel(Channel const &copy) {
	*this = copy;
};

Channel &Channel::operator=(Channel const &copy) {
	if (this != &copy) {
		this->key_ = copy.key_;
		this->name_ = copy.name_;
		this->password_ = copy.password_;
		this->islocked_ = copy.islocked_;
		this->max_population_ = copy.max_population_;
		this->topic_ = copy.topic_;
		this->topic_name_ = copy.topic_name_;
		this->topic_restriction_ = copy.topic_restriction_;
		this->creation_auto_ = copy.creation_auto_;
		this->creation_given_ = copy.creation_given_;
		this->client_list_ = copy.client_list_;
		this->admin_list_ = copy.admin_list_;
		this->mode_list_ = copy.mode_list_;
	};
	return (*this);
};

Channel::~Channel(void) {};

int	Channel::get_key(void) {
	return (this->key_);
};

std::string	Channel::get_name(void) {
	return (this->name_);
};

std::string	Channel::get_password(void) {
	return (this->password_);
};

// Returns the channel's entry status. TRUE if private.
bool	Channel::isprivate(void) {
	return (this->islocked_);
};

int	Channel::get_max_population(void) {
	return (this->max_population_);
};

int	Channel::get_topic(void) {
	return (this->topic_);
};

std::string	Channel::get_topic_name(void) {
	return (this->topic_name_);
};

bool	Channel::istopic_restricted(void) const {
	return (this->topic_restriction_);
};

std::string	Channel::get_creation_auto(void) {
	return (this->creation_auto_);
};

std::string	Channel::get_creation_given(void) {
	return (this->creation_given_);
};

int	Channel::get_population(void) {
	return (this->client_list_.size() + this->admin_list_.size());
};

/* Returns a string with the names of operators and users in the channel,
separated by spaces. */
//TODO I guess the IRC convention is to separate with a comma, not a space.
std::string	Channel::get_client_list(void) {
	std::string	list;

	if (this->admin_list_.size()) {
		for (size_t i = 0; i < this->admin_list_.size(); i++) {
			list += "@" + this->admin_list_[i].get_displayname();
			list += " ";
		};
	};
	if (this->client_list_.size()) {
		for (size_t i = 0; i < this->client_list_.size(); i++) {
			list += this->client_list_[i].get_displayname();
			list += " ";
		};
	};
	return (list);
};

Client	*Channel::get_admin_by_fd(int fd) {
	for (std::vector<Client>::iterator i = this->admin_list_.begin(); i != this->admin_list_.end(); i++) {
		if (i->get_fd() == fd)
			return (&(*i));
	};
	return (NULL);
};

Client	*Channel::get_client_by_fd(int fd) {
	for (std::vector<Client>::iterator i = this->client_list_.begin(); i != this->client_list_.end(); i++) {
		if (i->get_fd() == fd)
			return (&(*i));
	};
	return (NULL);
};

Client	*Channel::get_client_by_name(std::string displayname) {
	for (std::vector<Client>::iterator i = this->admin_list_.begin(); i != this->admin_list_.end(); i++) {
		if (i->get_displayname() == displayname)
			return (&(*i));
	};
	for (std::vector<Client>::iterator i = this->client_list_.begin(); i != this->client_list_.end(); i++) {
		if (i->get_displayname() == displayname)
			return (&(*i));
	};
	return (NULL);
};

// Checks if a client is in the channel.
bool	Channel::ishere(std::string displayname) {
	for (size_t i = 0; i < this->admin_list_.size(); i++) {
		if (this->admin_list_[i].get_displayname() == displayname)
			return (true);
	};
	for (size_t i = 0; i < this->client_list_.size(); i++) {
		if (this->client_list_[i].get_displayname() == displayname)
			return (true);
	};
	return (false);
};

// Returns the channel's modes (e.g.: +snt, +k).
std::string	Channel::get_modes(void) {
	std::string	list;

	for (size_t i = 0; i < this->mode_list_.size(); i++) {
		if (this->mode_list_[i].first != 'o' && this->mode_list_[i].second)
			list.push_back(this->mode_list_[i].first);
	};
	// If there are modes set, adds a '+' to the beginning (per IRC convention).
	if (!list.empty())
		list.insert(list.begin(), '+');
	return (list);
};

// Returns which mode is at the given index in the mode list.
bool	Channel::get_mode_at_index(size_t index) {
	return (this->mode_list_[index].second);
};

void	Channel::set_key(int key) {
	this->key_ = key;
};

void	Channel::set_name(std::string name) {
	this->name_ = name;
};

void	Channel::set_password(std::string password) {
	this->password_ = password;
};

void	Channel::lock(bool islocked) {
	this->islocked_ = islocked;
};

void	Channel::set_max_population(int limit) {
	this->max_population_ = limit;
};

void	Channel::set_topic(int topic) {
	this->topic_ = topic;
};

void	Channel::set_topic_name(std::string topic_name) {
	this->topic_name_ = topic_name;
};

void	Channel::set_topic_restriction(bool status) {
	this->topic_restriction_ = status;
};

// Sets the channel's creation based in seconds since 1970. E.g.: "1643079273".
void	Channel::set_creation_auto(void) {
	std::ostringstream stream;
	std::time_t now = std::time(0); // Current time in seconds, since 1970-01-01 00:00:00 UTC.

	stream << now; // Converts the time to a string.
	this->creation_auto_ = stream.str();
};

/* Sets the creation time of the channel based on the given time.
E.g.: "2022-01-24 15:14:33". */
void	Channel::set_creation_given(std::string time) {
	this->creation_given_ = time;
};

/* Takes the mode list at the position of the first argument
and changes for the mode given in the second argument. */
void	Channel::set_mode_at_index(size_t index, bool mode) {
	mode_list_[index].second = mode;
};

void	Channel::add_admin(Client new_admin) {
	this->admin_list_.push_back(new_admin);
};

void	Channel::add_client(Client new_client) {
	this->client_list_.push_back(new_client);
};

void	Channel::remove_admin(int fd) {
	for (std::vector<Client>::iterator i = this->admin_list_.begin(); i != this->admin_list_.end(); i++) {
		if (i->get_fd() == fd) {
			this->admin_list_.erase(i);
			return ; //! RETURN OR BREAK?
		};
	};
};

void	Channel::remove_client(int fd) {
	for (std::vector<Client>::iterator i = this->client_list_.begin(); i != this->client_list_.end(); i++) {
		if (i->get_fd() == fd) {
			this->client_list_.erase(i);
			return ; //! RETURN OR BREAK?
		};
	};
};

// Promotes a client to admin.
//TODO Try with -1 as the initial value and ++i in the for loop.
bool	Channel::promote(std::string &displayname) {
	size_t i = 0;

	// First, checks if the client is in the list and retrieves its position.
	for (; i < this->client_list_.size(); i++) {
		if (this->client_list_[i].get_displayname() == displayname)
			break;
	};
	// If so, adds them to the admin list and removes from the client list.
	if (i < this->client_list_.size()) { // 
		this->admin_list_.push_back(this->client_list_[i]);
		this->client_list_.erase(i + this->client_list_.begin());
		return (true);
	};
	return (false);	
};

// Demotes an admin to client.
bool	Channel::demote(std::string &displayname) {
	size_t i = 0;

	for (; i < this->admin_list_.size(); i++) {
		if (this->admin_list_[i].get_displayname() == displayname)
			break;
	};
	if (i < this->admin_list_.size()) { // 
		this->client_list_.push_back(this->admin_list_[i]);
		this->admin_list_.erase(i + this->admin_list_.begin());
		return (true);
	};
	return (false);	
};

// Sends a message to everyone in the channel.
void	Channel::relay(std::string message) {
	// Sends message to all channel operators, one by one.
	for (size_t i = 0; i < admin_list_.size(); i++) {
		//TODO VER SE ESSE É O PROCEDIMENTO PADRÃO (MOSTRAR MENSAGEM DE ERRO). SENÃO, TROCAR A LÓGICA (TIRAR O IF -1 E DEIXAR SÓ O SEND COM ARGUMENTOS).
		if (send(admin_list_[i].get_fd(), message.c_str(), message.size(), 0) == -1)
			std::cerr << "A mensagem não foi enviada para " 
					  << admin_list_[i].get_displayname() << "." << std::endl;
	};
	// Then does the same to every client.
	for (size_t i = 0; i < client_list_.size(); i++) {
		if (send(client_list_[i].get_fd(), message.c_str(), message.size(), 0) == -1)
			std::cerr << "A mensagem não foi enviada para " 
					  << client_list_[i].get_displayname() << "." << std::endl;
	};
};

/* Sends a message to everyone in the channel, 
but the one with the fd given as second argument. */
void	Channel::relay(std::string message, int excluded) {
	for (size_t i = 0; i < admin_list_.size(); i++) {
		if (admin_list_[i].get_fd() != excluded) {
			if (send(admin_list_[i].get_fd(), message.c_str(), message.size(), 0) == -1)
				std::cerr << "A mensagem não foi enviada para " 
						  << admin_list_[i].get_displayname() << "." << std::endl;
		};
	};
	for (size_t i = 0; i < client_list_.size(); i++) {
		if (client_list_[i].get_fd() != excluded) {
			if (send(client_list_[i].get_fd(), message.c_str(), message.size(), 0) == -1)
				std::cerr << "A mensagem não foi enviada para " 
						  << client_list_[i].get_displayname() << "." << std::endl;
		};
	};
};
