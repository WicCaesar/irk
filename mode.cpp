/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 19:48:31 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 11:37:37 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "channel.hpp"
#include <string>

void	Server::mode_utils(std::string &command, int fd) {
	Client		*client = get_client_by_fd(fd);
	Channel		*channel;
	std::string	name;
	std::string	params;
	std::string	new_mode;
	std::string	arguments = ":";
	std::stringstream	mode_list;
	char		binary = '\0';
	
	arguments.clear();
	mode_list.clear();
	// Searches the command for the first character that is not MODE \t\v.
	size_t		position = command.find_first_not_of("MODE \t\v");
	if (position != std::string::npos)
		command = command.substr(position);
	else { // If it's not found, sends error 461.
		respond(ERR_NEEDMOREPARAMS(client->get_displayname()), fd);
		return ;
	};
	// Looks at the arguments and splits them into individual parameters.
	get_arguments(command, name, new_mode, params);
	std::vector<std::string>	shards = get_params(params);
	// If the channel doesn't exist, or its name's invalid, sends error 403.
	if (name[0] != '#' || !(channel = get_channel_by_name(name.substr(1)))) {
		respond(ERR_NOSUCHCHANNEL(client->get_displayname(), name), fd);
		return ; // If the client is not a member, sends error 442.
	} else if (!channel->get_client_by_fd(fd) && !channel->get_admin_by_fd(fd)) {
		respond(ERR_NOTONCHANNEL(client->get_displayname(), name), fd);
		return ; // If no mode is set, displays the current modes.
	} else if (new_mode.empty()) {
		respond(RPL_CHANNELMODEIS(client->get_displayname(), name, channel->get_modes()) + 
				RPL_CREATIONTIME(client->get_displayname(), channel->get_name(), channel->get_creation_auto()), fd);
		return ; // If the client trying to set modes is not an operator, sends error 482.
	} else if (!channel->get_admin_by_fd(fd)) {
		respond(ERR_CHANOPRIVSNEEDED(client->get_displayname(), name), fd);
		return ;
	} else if (channel) {
		size_t	position = 0;
		// Iterates through the parameters and sets the modes accordingly.
		for (size_t i = 0; i < new_mode.size(); i++) {
			if (new_mode[i] == '+' || new_mode[i] == '-')
				binary = new_mode[i]; // Updates the binary operator.
			else if (new_mode[i] == 'l')
				mode_list << channel_limit(shards, channel, position, binary, fd, mode_list.str(), arguments);
			else if (new_mode[i] == 'o')
				mode_list << operator_privilege(shards, channel, position, binary, fd, mode_list.str(), arguments);
			else if (new_mode[i] == 'k')
				mode_list << password_mode(shards, channel, position, binary, fd, mode_list.str(), arguments);
			else if (new_mode[i] == 'i')
				mode_list << invite_only(channel, binary, mode_list.str());
			else if (new_mode[i] == 't')
				mode_list << topic_restriction(channel, binary, mode_list.str());
			else
				respond(ERR_UNKNOWNMODE(client->get_displayname(), channel->get_name(), new_mode[i]), fd);
		};
	};
	std::string	final_mode = mode_list.str();
	if (final_mode.empty())
		return ;
	channel->relay(RPL_CHANGEMODE(client->get_hostname(), channel->get_name(), final_mode, arguments));
};

// Points at the first character that is not name, mode or whitespace.
void	Server::get_arguments(std::string command, std::string &name, std::string &mode, std::string &params) {
	std::istringstream stream(command);
	stream >> name;
	stream >> mode;
	size_t	position = command.find_first_not_of(name + mode + " \t\v");
	if (position != std::string::npos)
		params = command.substr(position);
};

// Returns a list of all the parameters split by commas.
std::vector<std::string>	Server::get_params(std::string params) {
	if (!params.empty() && params[0] == ':')
		params.erase(params.begin());
	std::string					param;
	std::vector<std::string>	shards;
	std::istringstream			stream(params);
	while (std::getline(stream, param, ',')) {
		shards.push_back(param);
		param.clear();
	};
	return (shards);
};

/* Sets the maximum population of a channel. If there are more users than 
the maximum amount, nobody is kicked from the channel, but nobody can join. 
Usage: MODE #Channel +l 42 */
std::string	Server::channel_limit(std::vector<std::string> shards, Channel *channel, size_t &position, char binary, int fd, std::string mode_list, std::string &arguments) {
	std::string	param;
	std::string	limit;
	param.clear();
	limit.clear();

	if (binary == '+') {
		if (shards.size() > position) {
			limit = shards[position++];
			// If the limit is invalid, sends error.
			if (is_positive(limit) == false)
				respond(ERR_INVALIDMODEPARAM(channel->get_name(), "(l)"), fd);
			else {
				channel->set_mode_at_index(0, true);
				channel->set_max_population(std::atoi(limit.c_str()));
				// Adds "limit" to the list of arguments.
				if (!arguments.empty())
					arguments += " ";
				arguments += limit;
				// Appends the 'l' mode to the list of modes.
				param = append_mode(mode_list, binary, 'l');
			};
		} else
			respond(ERR_NEEDMODEPARAM(channel->get_name(), "(l)"), fd);
	} else if (binary == '-' && channel->get_mode_at_index(0)) {
		// If the operator is minus, removes the channel limit mode.
		channel->set_mode_at_index(0, false);
		channel->set_max_population(0);
		param = append_mode(mode_list, binary, 'l');
	};
	return (param);
};

/* Promotes a member to operator, or demotes an operator to member, 
depending on the binary operator before the letter 'o'.
Usage: MODE #Channel +o new_operator || MODE #Channel -o bad_operator */
std::string	Server::operator_privilege(std::vector<std::string> shards, Channel *channel, size_t &position, char binary, int fd, std::string mode_list, std::string &arguments) {
	std::string	param;
	std::string	user;
	param.clear();
	user.clear();

	// Gets the user.
	if (shards.size() > position)
		user = shards[position++];
	else {
		respond(ERR_NEEDMODEPARAM(channel->get_name(), "(o)"), fd);
		return (param);
	};

	// If the user is not a member, sends error 441.
	if (!channel->get_client_by_name(user)) {
		respond(ERR_USERNOTINCHANNEL(user, channel->get_name()), fd);
		return (param);
	};

	if (binary == '+') {
		channel->set_mode_at_index(1, true);
		if (channel->promote(user)) {
			param = append_mode(mode_list, binary, 'o');
			if (!arguments.empty())
				arguments += " ";
			arguments += user;
		};
	} else if (binary == '-') {
		channel->set_mode_at_index(1, false);
		if (channel->demote(user)) {
			param = append_mode(mode_list, binary, 'o');
			if (!arguments.empty())
				arguments += " ";
			arguments += user;
		};
	};
	return (param);
};

/* New members of the channel can only join if they add the right password 
as second argument of the JOIN command. 
Usage: MODE #Channel +k password */
std::string	Server::password_mode(std::vector<std::string> shards, Channel *channel, size_t &position, char binary, int fd, std::string mode_list, std::string &arguments) {
	std::string	param;
	std::string	password;
	param.clear();
	password.clear();

	// Gets the password.
	if (shards.size() > position)
		password = shards[position++];
	else {
		respond(ERR_NEEDMODEPARAM(channel->get_name(), "(k)"), fd);
		return (param);
	};

	std::cout << "APAGAR (TESTE) Password 1: " << password << std::endl;

	// If the password is not up to the standards, sends error.
	if (good_password(password) == false) {
		respond(ERR_INVALIDMODEPARAM(channel->get_name(), "(k)"), fd);
		return (param);
	};

	std::cout << "APAGAR (TESTE) Password 2: " << password << std::endl;
	
	if (binary == '+') {
		channel->set_mode_at_index(2, true);
		channel->set_password(password);
		std::cout << "APAGAR (TESTE) Password 3: " << password << std::endl;
		if (!arguments.empty())
			arguments += " ";
		arguments += password;
		std::cout << "APAGAR (TESTE) Password 4: " << password << std::endl;
		param = append_mode(mode_list, binary, 'k');
	} else if (binary == '-' && channel->get_mode_at_index(2)) {
		if (password == channel->get_password()) {
			channel->set_mode_at_index(2, false);
			channel->set_password("");
			param = append_mode(mode_list, binary, 'k');
		} else
			respond(ERR_KEYSET(channel->get_name()), fd);
		};
	return (param);
};

/* This channel is now only accessible if a member uses this command with 
another client's display name. Only then they can successfully join.
Usage: MODE #Channel +i -> INVITE name #Channel */
std::string	Server::invite_only(Channel *channel, char binary, std::string mode_list) {
	std::string	param;
	param.clear();
	if (binary == '+' && !channel->get_mode_at_index(3)) {
		channel->set_mode_at_index(3, true);
		channel->lock(true);
		param = append_mode(mode_list, binary, 'i');
	} else if (binary == '-' && channel->get_mode_at_index(3)) {
		channel->set_mode_at_index(3, false);
		channel->lock(false);
		param = append_mode(mode_list, binary, 'i');
	};
	return (param);
};

std::string	Server::topic_restriction(Channel *channel, char binary, std::string mode_list) {
	std::string	param;
	param.clear();

	if (binary == '+' && channel->get_mode_at_index(4) == false) {
		channel->set_mode_at_index(4, true);
		channel->set_topic_restriction(true);
		param = append_mode(mode_list, binary, 't');
	} else if (binary == '-' && channel->get_mode_at_index(4)) {
		channel->set_mode_at_index(4, false);
		channel->set_topic_restriction(false);
		param = append_mode(mode_list, binary, 't');
	};
	return (param);
};

std::string	Server::append_mode(std::string mode_list, char binary, char mode) {
	std::stringstream	stream;
	stream.clear();

	// Gets the last operator.
	char	last = '\0';
	for (size_t i = 0; i < mode_list.size(); i++) {
		if (mode_list[i] == '+' || mode_list[i] == '-')
			last = mode_list[i];
	};

	//If the last operator differs from the current one, adds operator and mode.
	if (last != binary)
		stream << binary << mode;
	else
		stream << mode;
	return (stream.str());
};

bool	Server::is_positive(std::string &limit) {
    // Checks if the passed limit contains only digits.
    bool is_numeric = (limit.find_first_not_of("0123456789") == std::string::npos);
    // Converts it to an integer.
    int limit_value = std::atoi(limit.c_str());
    // Checks if it's greater than 0.
    bool is_positive = (limit_value > 0);

    if (is_numeric && is_positive)
		return (true);
    return (false);
};

bool	Server::good_password(std::string password) {
	if (password.empty())
		return (false);
	for (size_t i = 0; i < password.size(); i++) {
		if (std::isalnum(password[i] == false) && password[i] != '_')
			return (false);
	};
	return (true);
};
