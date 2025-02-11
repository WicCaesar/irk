/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 12:38:40 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 11:44:38 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "client.hpp"
#include "channel.hpp"

Client::Client(void) {
	this->fd_ = -1;
	this->login_ = "";
	this->displayname_ = "";
	this->ip_ = "";
	this->online_status_ = false;
	this->privileged_status_ = false;
	this->registration_status_ = false;
	this->buffer_ = "";
};

Client::Client(std::string displayname, std::string login, int fd) : fd_(fd), login_(login), displayname_(displayname) {};

Client::Client(Client const &copy) {
	*this = copy;
};

Client &Client::operator=(Client const &copy) {
	if (this != &copy) {
		this->fd_ = copy.fd_;
		this->login_ = copy.login_;
		this->displayname_ = copy.displayname_;
		this->ip_ = copy.ip_;
		this->online_status_ = copy.online_status_;
		this->privileged_status_ = copy.privileged_status_;
		this->registration_status_ = copy.registration_status_;
		this->buffer_ = copy.buffer_;
		this->invitations_ = copy.invitations_;
	};
	return *this;
};

Client::~Client(void) {};

int		Client::get_fd(void) {
	return (this->fd_);
};

std::string	Client::get_login(void) {
	return (this->login_);
};

std::string	Client::get_displayname(void) {
	return (this->displayname_);
};

std::string	Client::get_ip(void) {
	return (this->ip_);
};

bool	Client::isonline(void) {
	return (this->online_status_);
};

bool	Client::isprivileged(void) {
	return (this->privileged_status_);
};

bool	Client::isregistered(void) {
	return (this->registration_status_);
};

std::string	Client::get_buffer(void) {
	return (this->buffer_);
};

bool	Client::get_invitation_status(std::string &channel_name) {
	for (size_t i = 0; i < this->invitations_.size(); i++) {
		if (this->invitations_[i] == channel_name) {
			return (true);
		};
	};
	return (false);
};

// Returns the client's hostname (displayname!login).
std::string	Client::get_hostname(void) {
	std::string	hostname = this->get_displayname() + "!" + this->get_login();
	return (hostname);
};

void	Client::set_fd(int fd) {
	this->fd_ = fd;
};

void	Client::set_login(std::string &login) {
	this->login_ = login;
};

void	Client::set_displayname(std::string &displayname) {
	this->displayname_ = displayname;
};

void	Client::set_ip(std::string ip) {
	this->ip_ = ip;
};

void	Client::set_online_status(bool online) {
	this->online_status_ = online;
};

void	Client::set_privileged_status(bool privileged) {
	this->privileged_status_ = privileged;
};

void	Client::set_registration_status(bool registered) {
	this->registration_status_ = registered;
};

void	Client::set_buffer(std::string received_data) {
	this->buffer_ += received_data;
};

void	Client::clear_buffer(void) {
	this->buffer_.clear();
};

void	Client::invite_to_channel(std::string &channel_name) {
	this->invitations_.push_back(channel_name);
};

void	Client::undo_invitation(std::string &channel_name) {
	for (size_t i = 0; i < this->invitations_.size(); i++) {
		if (this->invitations_[i] == channel_name) {
			this->invitations_.erase(this->invitations_.begin() + i);
			return ;
		};
	};
};
