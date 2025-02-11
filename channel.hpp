/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/24 15:14:20 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 11:42:17 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "server.hpp"
#include "client.hpp"

class	Client;

class	Channel {
	private:
		int			key_;
		std::string	name_;
		std::string	password_;
		bool		islocked_; // If true, channel's only accessible through invitation or password.
		int			max_population_;
		int			topic_;	// The channel's topic ID (eg.: 1 for General Chat).
		std::string	topic_name_;
		bool		topic_restriction_;
		std::string	creation_auto_;
		std::string	creation_given_;
		std::vector<Client>	admin_list_;
		std::vector<Client>	client_list_;
		std::vector<std::pair<char, bool> > mode_list_;

	public:
		Channel(void);
		Channel(Channel const &copy);
		Channel &operator=(Channel const &copy);
		~Channel(void);

		int			get_key(void);
		std::string	get_name(void);
		std::string	get_password(void);
		bool		isprivate(void);	// Returns the channel's entry status. TRUE if private.
		int			get_max_population(void);
		int			get_topic(void);
		std::string	get_topic_name(void);
		bool		istopic_restricted(void) const; // Returns the channel's topic restriction status.
		std::string	get_creation_auto(void);
		std::string	get_creation_given(void);
		int			get_population(void);
		std::string	get_client_list(void);
		Client		*get_admin_by_fd(int fd);
		Client		*get_client_by_fd(int fd);
		Client		*get_client_by_name(std::string displayname);
		bool		ishere(std::string displayname); // Checks if a client is in the channel.
		std::string	get_modes(void); // Returns the channel's modes (for instance, +snt, +k, etc).
		bool		get_mode_at_index(size_t index); // Returns the mode at a specific index.

		void	set_key(int key);
		void	set_name(std::string name);
		void	set_password(std::string password);
		void	lock(bool islocked_);
		void	set_max_population(int limit);
		void	set_topic(int topic);
		void	set_topic_name(std::string topic_name);
		void	set_topic_restriction(bool status);
		void	set_creation_auto(void);
		void	set_creation_given(std::string time);
		void	set_mode_at_index(size_t index, bool mode);

		void	add_admin(Client new_admin);
		void	add_client(Client new_client);
		void	remove_admin(int fd);
		void	remove_client(int fd);
		bool	promote(std::string &displayname); // Promotes a client to admin.
		bool	demote(std::string &displayname); // Demotes an admin to client.

		void	relay(std::string message);
		void	relay(std::string message, int fd);
};

#endif