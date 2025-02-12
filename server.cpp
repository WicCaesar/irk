/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 07:33:23 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:36:14 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "client.hpp"
#include "channel.hpp"

Server::Server(void) {
	this->socket_ = -1;
};

Server::Server(Server const &copy) {
	*this = copy;
};

Server &Server::operator=(Server const &copy) {
	if (this != &copy) {
		this->port_ = copy.port_;
		this->socket_ = copy.socket_;
		this->password_ = copy.password_;
		this->client_list_ = copy.client_list_;
		this->channel_list_ = copy.channel_list_;
		this->fd_list_ = copy.fd_list_;
	};
	return *this;
};

Server::~Server(void) {};

int	Server::get_port(void) {
	return (this->port_);
};

int	Server::get_socket(void) {
	return (this->socket_);
};

std::string	Server::get_password(void) {
	return (this->password_);
};

// Returns the client with the respective file descriptor.
Client	*Server::get_client_by_fd(int fd) {
	for (size_t i = 0; i < this->client_list_.size(); i++) {
		if (this->client_list_[i].get_fd() == fd)
			return (&this->client_list_[i]);
	};
	return (NULL);
};

// Returns the client with the respective display name.
Client	*Server::get_client_by_name(std::string displayname) {
	for (size_t i = 0; i < this->client_list_.size(); i++) {
		if (this->client_list_[i].get_displayname() == displayname)
			return (&this->client_list_[i]);
	};
	return (NULL);
};

Channel	*Server::get_channel_by_name(std::string name) {
	for (size_t i = 0; i < this->channel_list_.size(); i++) {
		if (this->channel_list_[i].get_name() == name)
			return (&this->channel_list_[i]);
	};
	return (NULL);
};

void	Server::set_port(int port) {
	this->port_ = port;
};

void	Server::set_socket(int fd) {
	this->socket_ = fd;
};

void	Server::set_password(std::string password) {
	this->password_ = password;
};

void	Server::add_client(Client new_client) {
	this->client_list_.push_back(new_client);
};

void	Server::add_channel(Channel new_channel) {
	this->channel_list_.push_back(new_channel);
};

void	Server::add_fd(struct pollfd new_fd) {
	this->fd_list_.push_back(new_fd);
};

// Static variable to check if a signal was received.
bool	Server::signal_ = false;

void Server::antenna(int signal) {
	(void)signal;
	std::cout << "\nSinal recebido!" << std::endl;
	Server::signal_ = true;
};

void	Server::start(int port, std::string password) {
	this->port_ = port;
	this->password_ = password;
	this->socket_setup();

	std::cout << "Aguardando conexão…" << std::endl;
	while (Server::signal_ == false) {
		//TODO STUDY POLL, HOW IT'S DIFFERENT FROM EPOLL, HOW IT LINKS TO THE SOCKET, 0_NONBLOCKING.
		// If the poll call fails and no signal is received, throws an exception.
		if ((poll(&fd_list_[0], fd_list_.size(), -1) == -1)
			&& Server::signal_ == false)
			throw(std::runtime_error("Falha no poll.")); // To force this error, hit Ctrl+Z.
		for (size_t i = 0; i < fd_list_.size(); i++) {
			// What to do if there's incoming data to read on the socket?
			if (fd_list_[i].revents && POLLIN) {
				if (fd_list_[i].fd == socket_) // If it's the server socket,
					this->welcome_client(); // accept the new client.
				else
					this->welcome_data(fd_list_[i].fd); // Or receive new data from the client.
			/* If the fd in the list is the same as the socket, it means that 
			an event has occurred on the server socket fd. In a server 
			application like mine, this type of event usually indicates that 
			a new client is trying to connect to the server, so welcome them.
			If an event occurs in a different file descriptor than the server's, 
			it's a client that has sent data that needs to be processed.*/
			};
		};

	};
};

// Kicks all the clients and closes the server socket.
void	Server::close_fd(void) {
	for (size_t i = 0; i < client_list_.size(); i++) {
		std::cout << "Cliente " << client_list_[i].get_fd() << " desconectado." << std::endl;
		close(client_list_[i].get_fd());
	}
	if (socket_ != -1) {
		std::cout << "Servidor " << socket_ << " desconectado." << std::endl;
		close(socket_);
	}
};

// Adjusts the IRC server socket.
void	Server::socket_setup(void) {
	int enable = TRUE; // I'll use it to enable options for setsockopt (SO_REUSEADDR, O_NONBLOCK).
	//! Never change this to boolean, as apparently setsockopt can't handle it.
	this->server_address_.sin_family = AF_INET; // Sets the address to IPv4 (AF_INET6 for IPv6).
	this->server_address_.sin_port = htons(port_);
	/* In the sockaddr_in struct, the sin_port stores a 16-bit integer, big endian.
	So I use htons to convert the port into this network byte order (6667 to 0x1A0B). */
	this->server_address_.sin_addr.s_addr = INADDR_ANY; // Allows connections to any address (including local).
	/* in_addr typically has a single member, representing the IPv4 address.
	INADDR_ANY represents “any” IP address, meaning the socket will be bound to
	all available network interfaces on the host. */
	
	/* Creates the server socket (which is really a file descriptor with value 3). 
	The previous are stdin (0), stdout (1) and stderr (2). */
	socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_ == -1)
		throw(std::runtime_error("Falha ao criar o soquete."));
	// SOL_SOCKET is the socket level (manipulates options at the socket level, independent of protocol).
	// SO_REUSEADDR allows reusing the socket immediately after it is closed.
	if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
		throw(std::runtime_error("Falha no SO_REUSEADDR do soquete."));
	// F_SETFL sets the file status flags to the value of its third argument.
	// O_NONBLOCK means means it doesn't wait for data to be read.
	//! It's the only form allowed by the subject; any other flag's forbidden.
	if (fcntl(socket_, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("Falha no O_NONBLOCK do soquete."));
	// Binds the socket to the address and port.
	if (bind(socket_, (struct sockaddr *)&this->server_address_, sizeof(this->server_address_)) == -1)
		throw(std::runtime_error("Falha ao vincular o soquete."));
	// Listens for incoming connections, turning into a passive socket (create, bind, listen, accept).
	/* SOMAXCONN is the maximum of pending connections that can be queued.
	Setting it to -1 allows the system to choose its maximum capacity. */
	if (listen(socket_, SOMAXCONN) == -1)
		throw(std::runtime_error("Falha na escuta."));

	this->newclient_.fd = socket_;
	/* Requested events are conditions to be met for the poll call to return.
	It's set to POLLIN, which is incoming data. Could be read, write, error… */
	this->newclient_.events = POLLIN;
	this->newclient_.revents = 0; // Events returned for this client. 0 is no events.
	fd_list_.push_back(this->newclient_); // Adds this new client to the fd list.
};

// Accepts a new client.
void	Server::welcome_client(void) {
	Client client;

	memset(&this->client_address_, 0, sizeof(this->client_address_)); // Clears the client address structure.
	socklen_t length = sizeof(this->client_address_);
	int incoming_fd = accept(socket_, (sockaddr *)&(this->client_address_), &length); // Accepts a new connection.
	if (incoming_fd == -1) { // If accept fails, prints an error message.
		std::cerr << "Falha ao aceitar a conexão." << std::endl;
		return ;
	}; // If O_NONBLOCK fails, prints an error message.
	if (fcntl(incoming_fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Falha ao configurar o O_NONBLOCK." << std::endl;
		return ;
	};

	this->newclient_.fd = incoming_fd;
	this->newclient_.events = POLLIN;
	this->newclient_.revents = 0;
	client.set_fd(incoming_fd);
	client.set_ip(inet_ntoa((this->client_address_.sin_addr))); // Converts the client's IP address to string.
	client_list_.push_back(client);
	fd_list_.push_back(this->newclient_);
	std::cout << "Cliente " << incoming_fd << " conectado." << std::endl;
};

// Receives new data from a client that is already registered.
void	Server::welcome_data(int fd) {
	char buffer[1024];

	memset(buffer, 0, sizeof(buffer)); // Clears the buffer to ensure any previous data is cleared.
	Client *client = get_client_by_fd(fd); // Gets the respective client of this fd.
	// Receives data from the client and stores it in the buffer.
	ssize_t bytes = recv(fd, buffer, sizeof(buffer) -1, 0); // -1 space for the NULL terminator.
	if (bytes <= 0) { // If there's no data to read or an error occurred, 
		std::cout << "Cliente " << fd << " desconectado." << std::endl; // informs the user,
		remove_from_all_channels(fd);	// removes the client from all channels,
		remove_client_from_list(fd); 	// from the client list,
		remove_fd_from_list(fd); 		// from the fd list,
		close(fd); 						// and closes the client's socket.
	} else { // If no problems occurred, prints the received data.
		client->set_buffer(buffer);
		// If the client's buffer doesn't contain a newline character, returns.
		if (client->get_buffer().find_first_of("\r\n") == std::string::npos)
			return ;

		// Splits the buffer into commands, storing them in the command vector.
		std::vector<std::string> command = split_buffer(client->get_buffer());
		for (size_t i = 0; i < command.size(); i++)
			this->execute_command(command[i], fd); // Parses and executes the command.
		if (get_client_by_fd(fd))
			get_client_by_fd(fd)->clear_buffer(); // Clears the client's buffer.
	};
};

// Searches through the fd list and removes whatever has the given descriptor.
void	Server::remove_fd_from_list(int fd) {
	for (size_t i = 0; i < this->fd_list_.size(); i++) {
		if (this->fd_list_[i].fd == fd) {
			this->fd_list_.erase(this->fd_list_.begin() + i);
			return ;
		};
	};
};

// Searches through the client list and removes the client with the given fd.
void	Server::remove_client_from_list(int fd) {
	for (size_t i = 0; i < this->client_list_.size(); i++) {
		if (this->client_list_[i].get_fd() == fd) {
			this->client_list_.erase(this->client_list_.begin() + i);
			return ;
		};
	};
};

// Searches through the channel list and removes a channel with that given name.
void	Server::remove_channel_from_list(std::string name) {
	for (size_t i = 0; i < this->channel_list_.size(); i++) {
		if (this->channel_list_[i].get_name() == name) {
			this->channel_list_.erase(this->channel_list_.begin() + i);
			return ;
		};
	};
};

void	Server::remove_from_all_channels(int fd) {
	for (size_t i = 0; i < this->channel_list_.size(); i++) {
		bool someone_removed = false;
		// Whether it's just a member
		if (channel_list_[i].get_client_by_fd(fd)) {
			channel_list_[i].remove_client(fd);
			someone_removed = true;
		// or an operator, removes them from the channel.
		} else if (channel_list_[i].get_admin_by_fd(fd)) {
			channel_list_[i].remove_admin(fd);
			someone_removed = true;
		};
		// If the channel has no more members, close the channel entirely.
		if (channel_list_[i].get_population() == 0) {
			channel_list_.erase(channel_list_.begin() + i);
			i--;
			continue;
		};
		// If any member's been removed from the channel, informs other members.
		if (someone_removed == true) {
			std::string response = ":" + get_client_by_fd(fd)->get_displayname() + "!~" \
				+ get_client_by_fd(fd)->get_login() + "@localhost QUIT Quit\r\n";
			// <prefix> <nickname> <separator> <username> <hostname> <action> <message>
			//     :     WicCaesar      !~      cnascime  @localhost   QUIT     Quit
		};
	};
};

// Splits the buffer into separate lines after certain dividers.
std::vector<std::string> Server::split_buffer(std::string buffer) {
	std::vector<std::string> kintsugi; // Stores the lines.
	std::istringstream stream(buffer);
	std::string shard;

	while (std::getline(stream, shard)) { // While there are lines to read,
		size_t position = shard.find_first_of("\r\n"); // looks for a new line,
		if (position != std::string::npos) 
			shard = shard.substr(0, position); // removes all after it,
		kintsugi.push_back(shard); // and adds the line to the vector.
	};
	return (kintsugi);
};

// Splits series of commands into separate tokens.
std::vector<std::string>	Server::split_command(std::string &commands) {
	std::vector<std::string> queue;
	std::istringstream stream(commands);
	std::string isolated_command;

	while (stream >> isolated_command) {
		queue.push_back(isolated_command);
		isolated_command.clear();
	};
	return (queue);
};

// Executes the command received from the client.
void	Server::execute_command(std::string &command, int fd) {
	if (command.empty())
		return ;
	std::vector<std::string> queue = split_command(command);
	// Looks for the first non-whitespace character.
	size_t found = command.find_first_not_of(" \t\v");
	if (found != std::string::npos)
		command = command.substr(found); // Removes everything before non-whitespaces.
	// If there's something on the queue, compares to these possibilities.
	if (queue.size()) {
		if (queue[0] == "BONG" || queue[0] == "/bong")
			return ; // bong is a ping equivalent.
		if (queue[0] == "USER" || queue[0] == "/bong")
			this->set_login(command, fd); // Changes the login ID (for access).
			//TODO TRY WITHOUT "THIS->".
		else if (queue[0] == "PASS" || queue[0] == "/pass" \
				|| queue[0] == "AUTHENTICATE" || queue[0] == "/authenticate")
			authenticate(command, fd); // Authenticates the client.
		else if (queue[0] == "NICK" || queue[0] == "/nick")
			set_displayname(command, fd); // Changes the display name.
		else if (queue[0] == "QUIT" || queue[0] == "/quit")
			quit(command, fd);
		else if (isregistered(fd) == TRUE) {
			if (queue[0] == "KICK" || queue[0] == "/kick")
				kick(command, fd); // Removes a member from a channel.
			else if (queue[0] == "JOIN" || queue[0] == "/join")
				join_utils(command, fd);
			else if (queue[0] == "PART" || queue[0] == "/part")
				part(command, fd); // Leaves a channel without quitting the server.
			else if (queue[0] == "PRIVMSG" || queue[0] == "/privmsg" \
					|| queue[0] == "MSG" || queue[0] == "/msg" \
					|| queue[0] == "SAY" || queue[0] == "/say" \
					|| queue[0] == "SAYTO" || queue[0] == "/sayto")
				privmsg(command, fd);
			else if (queue[0] == "MODE" || queue[0] == "/mode")
				mode_utils(command, fd); // Changes options in a channel.
			else if (queue[0] == "TOPIC" || queue[0] == "/topic")
				topic_utils(command, fd);
			else if (queue[0] == "INVITE" || queue[0] == "/invite")
				invite_utils(command, fd);
			else
				respond(ERR_UNKNOWNCOMMAND(get_client_by_fd(fd)->get_displayname(), queue[0]), fd);
		} else if (isregistered(fd) == FALSE)
			respond(ERR_NOTREGISTERED(std::string("*")), fd);
	};
};

// Checks a series of indicators to see whether a client is registered.
bool	Server::isregistered(int fd) {
	if (get_client_by_fd(fd) == FALSE || get_client_by_fd(fd)->get_login().empty() 
	|| get_client_by_fd(fd)->get_login().empty() || get_client_by_fd(fd)->get_displayname() == "*"
	|| get_client_by_fd(fd)->isonline() == false)
		return (false);
	return (true);
};

//* Client authentication

void	Server::authenticate(std::string credentials, int fd) {
	Client	*client	= get_client_by_fd(fd);
	credentials = credentials.substr(4); // Removes the first 4 characters (PASS).
	size_t beginning = credentials.find_first_not_of(" \t\v"); // Sets beginning point at the first non-whitespace character.
	if (beginning < credentials.size()) {
		credentials = credentials.substr(beginning); // Removes whitespace characters.
		if (credentials[0] == ':') // Removes ':'.
			credentials.erase(credentials.begin());
	};
	// Throws error if it's empty or if there's only whitespace characters.
	if (credentials.empty() || beginning == std::string::npos) {
		respond(ERR_NEEDMOREPARAMS(std::string("*")), fd);
		return ;
	} else if (client->isregistered() == FALSE) {
		std::string password = credentials;
		if (password == this->password_) //! SHOULD THIS BE THE CLIENT'S PERSONAL PASSWORD OR THE SERVER'S?
			client->set_registration_status(true);
		else
			respond(ERR_PASSWDMISMATCH(std::string("*")), fd);
	} else
		respond(ERR_ALREADYREGISTRED(client->get_displayname()), fd);
};

void	Server::set_login(std::string &login, int fd) {
	Client	*client = get_client_by_fd(fd);
	std::vector<std::string>	shards = split_command(login);

	// The string is passed as "USER cnascime 0 * César Augusto".
	if (client && shards.size() < 5) {
		respond(ERR_NEEDMOREPARAMS(client->get_displayname()), fd);
		return ;
	};
	if (!client || client->isregistered() == false) {
		respond(ERR_NOTREGISTERED(std::string("*")), fd);
		// return ; //! RETURN OR NOT? I'M GUESSING NOT.
	} else if (client && !client->get_login().empty()) {
		respond(ERR_ALREADYREGISTRED(client->get_displayname()), fd);
		return ;
	} else
		client->set_login(shards[1]);

	/* When registration is complete, welcomes the new client. */
	if (client && client->isregistered() && !client->get_login().empty() 
	&& !client->get_displayname().empty() && client->get_displayname() != "*" 
	&& !client->isonline())	{
		client->set_online_status(true);
		respond(RPL_WELCOME(client->get_displayname()), fd);
	};
};

void	Server::set_displayname(std::string new_displayname, int fd) {
	new_displayname = new_displayname.substr(4); // Removes the first 4 characters (NICK).
	// Sets beginning point at the first non-whitespace character.
	size_t beginning = new_displayname.find_first_not_of(" \t\v");
	if (beginning < new_displayname.size()) {
		new_displayname = new_displayname.substr(beginning); // Removes whitespace characters.
		if (new_displayname[0] == ':') // Removes ':'.
			new_displayname.erase(new_displayname.begin());
	};
	Client	*client = get_client_by_fd(fd);
	// Throws error if it's empty or if there's only whitespace characters.
	if (new_displayname.empty() || beginning == std::string::npos) {
		respond(ERR_NEEDMOREPARAMS(std::string("*")), fd);
		return ;
	};
	// If desired display name is already taken and doesn't belong to this client,
	// changes to a placeholder and sends an error message.
	if (isnametaken(new_displayname) && client->get_displayname() != new_displayname) {
		std::string placeholder = "*";
		if (client->get_displayname().empty())
			client->set_displayname(placeholder);
		respond(ERR_NICKNAMEINUSE(std::string(new_displayname)), fd);
		return ;
	};
	// If desired display name contains invalid characters, sends error message.
	// Valid characters are alphanumeric and underscore.
	if (isnamevalid(new_displayname) == false) {
		respond(ERR_ERRONEUSNICKNAME(new_displayname), fd);
		return ;
	} else {
		// If client exists and is registered, changes its display name.
		if (client && client->isregistered()) {
			std::string old_name = client->get_displayname();
			client->set_displayname(new_displayname);
			// If old name is not empty and different from new display name,
			if (old_name.empty() == FALSE && old_name != new_displayname) {
				// and if old name is a placeholder and client has a login ID,
				if (old_name == "*" && !client->get_login().empty()) {
					// connects the client and sends a reply with the new name.
					client->set_online_status(true);
					respond(RPL_WELCOME(client->get_displayname()), fd);
					//TODO TEST IF IT WOULD SEEM REDUNDANT ADDING RPL_NAMECHANGE INSIDE THIS CONDITION (WITH THE NEW NAME).
				};
				respond(RPL_NAMECHANGE(old_name, client->get_displayname()), fd);
				// return ;
			};
		} else if (client && client->isregistered() == false)
			respond(ERR_NOTREGISTERED(new_displayname), fd);
	};
	// If client exists, is registered with valid login ID and display name,
	// but online status is set to false, changes it to true and sends response.
	if (client && client->isregistered() && !client->get_login().empty() 
	&& !client->get_displayname().empty() && client->get_displayname() != "*" 
	&& !client->isonline())	{
		client->set_online_status(true);
		respond(RPL_WELCOME(client->get_displayname()), fd);
	};
};
//? I had to create the placeholder variable because, otherwise, set_displayname
//? wouldn't work due to the argument type (char '*'); it accepts only string.

/* Checks if the desired display name is too big or has invalid characters 
(anything but alphanumeric and underscore). TRUE if valid. */
bool	Server::isnamevalid(std::string &desired) {
	if (!desired.empty() && desired.size() <= 9 && desired.find_first_not_of \
("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == std::string::npos)
		return (true);
	return (false);
};

// Iterates through the client list and compares display names with the desired.
bool	Server::isnametaken(std::string &desired) {
	for (size_t i = 0; i < this->client_list_.size(); i++) {
		if (this->client_list_[i].get_displayname() == desired)
			return (true);
	};
	return (false);
};

void	Server::respond(std::string response, int fd) {
	if (send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Falha ao enviar informação." << std::endl;
};

void	Server::senderror(int code, std::string message, std::string displayname, int fd) {
	std::stringstream	stream;
	stream << ":localhost " << code << " " << displayname << message;
	std::string senderror = stream.str();
	if (send(fd, senderror.c_str(), senderror.size(), 0) == -1)
		std::cerr << "Falha ao enviar informação." << std::endl;
};

void	Server::senderror(int code, std::string message, std::string displayname, std::string channel, int fd) {
	std::stringstream	stream;
	stream << ":localhost " << code << " " << displayname << " " << channel << message;
	std::string senderror = stream.str();
	if (send(fd, senderror.c_str(), senderror.size(), 0) == -1)
		std::cerr << "Falha ao enviar informação." << std::endl;
};

// Separates the client message from the rest of the command.
// count is 3 for KICK (reason).
// count is 2 for PART (reason) and PRIVMSG (private message).
std::string	Server::get_message(std::string &command, std::vector<std::string> &temp, int count) {
	std::string			shard;
	std::string			message;
	std::stringstream 	stream(command);

	std::cout << "Entrou na função GET_MESSAGE DE VÁRIOS ARGUMENTOS" << std::endl;
	// Gets the first count shards from the command.
	int countdown = count;
	while (stream >> shard && countdown--)
		temp.push_back(shard);
	// If the command is not complete, returns an empty string.
	if ((int)temp.size() != count)
		return (std::string(""));
	isolate_shard(command, temp[(count - 1)], message);
	std::cout << "APAGAR (TESTE) MESSAGE: " << message << std::endl;
	return (message);
};

// Polymorphs the get_message function to work with a single string.
std::string	Server::get_message(std::string command) {
	std::string			shard;
	std::string			message;
	std::stringstream 	stream(command);

	std::cout << "Entrou na função GET_MESSAGE DE UM COMANDO SÓ" << std::endl;
	stream >> shard;
	isolate_shard(command, shard, message);
	std::cout << "Passou pela função ISOLATE_SHARD" << std::endl;
	if (message.empty())
		return (std::string("QUIT"));
	// If the reason doesn't start with a colon, adds it to the beginning.
	if (message[0] != ':') {
		for (size_t i = 0; i < message.size(); i++) {
			if (message[i] == ' ') {
				message.erase(message.begin() + i, message.end());
				break;
			};
		};
		message.insert(message.begin(), ':');
	};
	return (message);
};

void	Server::isolate_shard(std::string command, std::string isolate, std::string &string) {
	size_t	i = 0;
	std::cout << "Entrou na função ISOLATE_SHARD" << std::endl;
	std::cout << "APAGAR (TESTE) COMMAND: " << command << std::endl;
	for (; i < command.size(); i++) {
		if (command[i] != ' ') {
			std::string	temp;
			// Adds element to the string until a space is found.
			for (; i < command.size() && command[i] != ' '; i++)
				temp += command[i];
			// If the element is the one that needs to be isolated, stops.
			if (temp == isolate)
				break;
			else
				temp.clear();
		};
	};

	// If the element is found, adds the rest of the command to the string.
	if (i < command.size())
		string = command.substr(i);
	i = 0;
	// Removes space from the beginning of the string.
	for (; i < string.size() && string[i] == ' '; i++)
		;
	string = string.substr(i);
};
