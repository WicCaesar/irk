/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 11:52:38 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 11:26:10 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <vector>

class	Server;
class	Channel;

class	Client {
	private:
		int			fd_;
		std::string	login_;
		std::string	displayname_;
		std::string	ip_;
		bool		online_status_;
		// Indicates whether the client is an operator (admin).
		bool		privileged_status_;
		bool		registration_status_;
		std::string	buffer_;
		// Names of channels for which this client has been invited.
		std::vector<std::string>	invitations_;

	public:
		Client(void);
		Client(std::string displayname, std::string login, int fd);
		Client(Client const &copy);
		Client &operator=(Client const &copy);
		~Client(void);

		int			get_fd(void);
		std::string	get_login(void);
		std::string	get_displayname(void);
		std::string	get_ip(void);
		bool		isonline(void);
		bool		isprivileged(void);
		bool		isregistered(void);
		std::string	get_buffer(void);
		bool		get_invitation_status(std::string &channel_name);
		std::string	get_hostname(void);

		void	set_fd(int fd);
		void	set_login(std::string &login);
		void	set_displayname(std::string &displayname);
		void	set_ip(std::string ip);
		void	set_online_status(bool online);
		void	set_privileged_status(bool privileged);
		void	set_registration_status(bool registered);
		void	set_buffer(std::string received_data);

		void	invite_to_channel(std::string &channel_name);
		void	undo_invitation(std::string &channel_name);	
		void	clear_buffer(void);
};

#endif
