/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 07:33:32 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 11:27:45 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "client.hpp"
#include <iostream> // std::string, std::cerr, std::runtime_error
#include <sstream> // std::stringstream
#include <fstream> // std::ifstream
#include <string> // stricmp
#include <cstring> // strlen, memset
#include <cstdlib> // atoi
#include <csignal> // signal, SIGINT, SIGQUIT
#include <stdexcept> // std::runtime_error
#include <vector> // std::vector
#include <poll.h> // poll, pollfd, POLLIN
//#include <netdb.h> // gethostbyname
#include <unistd.h> // close
//#include <sys/socket.h> // socket, bind, listen, accept, recv
//#include <sys/types.h> // socket, bind, listen, accept, recv
#include <netinet/in.h> // htons, sockaddr_in, INADDR_ANY
#include <arpa/inet.h> // inet_ntoa
#include <fcntl.h> // fcntl, F_SETFL, O_NONBLOCK
#include <ctime> // time_t

#define TRUE (1)
#define FALSE (0)

class Client;
class Channel;

class Server {
	private:
		int port_;
		int socket_;	// File descriptor for the server socket
		static bool signal_;	// Event control signal
		std::string password_;
		std::vector<Client> client_list_;	// Connected clients
		std::vector<Channel> channel_list_;	// Available channels
		/* pollfd is a structure used with the poll system call to determine 
		the status of a file descriptor. I/O multiplexing is a way to handle
		multiple I/O operations with a single thread. */
		std::vector<struct pollfd> fd_list_;	// I/O multiplexing structures
		/* sockaddr_in is a structure containing an internet address.
		It has a short sin_family, containing the AF_INET address family (IPv4), 
		a short sin_port for the port number, and a struct in_addr sin_addr, 
		which contains the IP address. */
		struct sockaddr_in server_address_;	// Server address structure
		struct sockaddr_in client_address_;	// Client address structure
		struct pollfd newclient_;			// New client pollfd structure

	public:
		Server(void);
		Server(Server const &copy);
		Server &operator=(Server const &copy);
		~Server(void);


		int	get_port(void);
		int	get_socket(void);
		std::string	get_password(void);
		Client	*get_client_by_fd(int fd);
		Client	*get_client_by_name(std::string name);
		Channel	*get_channel_by_name(std::string name);

		void	set_port(int port);
		void	set_socket(int fd);
		void	set_login(std::string &login, int fd);
		void	set_displayname(std::string displayname, int fd);
		void	set_password(std::string password);
		void	add_client(Client new_client);
		void	add_channel(Channel new_channel);
		void	add_fd(struct pollfd new_fd);

		void	close_fd(void);
		void	remove_fd_from_list(int fd);
		void	remove_client_from_list(int fd);
		void	remove_channel_from_list(std::string name);
		void	remove_from_all_channels(int fd);

		static void	antenna(int signal);

		// Starts up the server, sets the socket up and waits for connections.
		void	start(int port, std::string password);
		void	socket_setup(void);
		void	welcome_client(void);
		void	welcome_data(int fd);

		// Parses commands in the buffer and executes them.
		void	authenticate(std::string credentials, int fd);
		void	execute_command(std::string &command, int fd);
		std::vector<std::string> split_buffer(std::string buffer);
		std::vector<std::string> split_command(std::string &commands);

		bool	isregistered(int fd);
		bool	isnamevalid(std::string &desired);
		bool	isnametaken(std::string &desired);

		void		respond(std::string response, int fd);
		void		senderror(int code, std::string message, std::string displayname, int fd);
		void		senderror(int code, std::string message, std::string displayname, std::string channel, int fd);
		void		isolate_shard(std::string command, std::string isolate, std::string &string);
		std::string	get_message(std::string command);
		std::string	get_message(std::string &command, std::vector<std::string> &temp, int count);
		

		//* JOIN
		void		join_utils(std::string command, int fd);
		void		join_channel(std::vector<std::pair<std::string, std::string> > &kintsugi, int i, int j, int fd);
		int			pairing(std::vector<std::pair<std::string, std::string> > &kintsugi, std::string command, int fd);
		bool		isinvited(Client *client, std::string channel, int flag);
		void		create_channel(std::vector<std::pair<std::string, std::string> > &kintsugi, int i, int fd);
		int			scour_presence(std::string displayname);
		//* KICK
		void		kick(std::string command, int fd);
		std::string	kick_utils(std::string command, std::vector<std::string> &temp, std::string &user, int fd);
		//* MODE
		void		mode_utils(std::string &command, int fd);
		void		get_arguments(std::string command, std::string &name, std::string &mode, std::string &params);
		std::vector<std::string>	get_params(std::string params);
		std::string	append_mode(std::string mode_list, char binary, char mode);
		std::string	channel_limit(std::vector<std::string> shards, Channel *channel, size_t &position, char binary, int fd, std::string mode_list, std::string &arguments);
		std::string	operator_privilege(std::vector<std::string> shards, Channel *channel, size_t &position, char binary, int fd, std::string mode_list, std::string &arguments);
		std::string	password_mode(std::vector<std::string> shards, Channel *channel, size_t &position, char binary, int fd, std::string mode_list, std::string &arguments);
		std::string	invite_only(Channel *channel, char binary, std::string mode_list);
		std::string	topic_restriction(Channel *channel, char binary, std::string mode_list);
		bool		is_positive(std::string &limit);
		bool		good_password(std::string password);
		//* PART
		void		part(std::string command, int fd);
		bool		part_utils(std::string command, std::vector<std::string> &temp, std::string &reason, int fd);
		//* TOPIC
		void		topic_utils(std::string &command, int &fd);
		std::string	get_topic(std::string &input);
		std::string	get_time(void);
		int			get_position(std::string &command);
		//* INVITE
		void		invite_utils(std::string &command, int &fd);
		//* PRIVMSG
		void		privmsg(std::string command, int fd);
		std::string	privmsg_utils(std::string command, std::vector<std::string> &temp);
		void		validate_receivers(std::vector<std::string> &temp, int fd);
		//* QUIT
		void		quit(std::string command, int fd);
};

/* The codes below are the responses and errors the server can send to the client.
They are based on the RFC 1459 and RFC 2812 (standard IRC protocols).
https://www.rfc-editor.org/rfc/inline-errata/rfc1459.html
https://www.rfc-editor.org/rfc/inline-errata/rfc2812.html#:~:text=5.1%20Command%20responses
https://www.alien.net.au/irc/irc2numerics.html
*/
//TODO TESTAR TODOS
#define	CRLF "\r\n"
// 001, the first message sent after client registration.
#define	RPL_WELCOME(displayname) (":001 " + displayname + " :Boas-vindas à Rede de Comunicação pela Internet." + CRLF)
// 221, information about a user's own modes. Some daemons have extended the mode command and certain modes take parameters (like channel modes).
#define	RPL_UMODEIS(hostname, channelname, mode, user) (":221 " + hostname + " MODE " + channelname + " " + mode + " " + user + CRLF)
// 324, response when the channel modes are requested.
#define	RPL_CHANNELMODEIS(displayname, channelname, modes) (":324 " + displayname + " " + channelname + " " + modes + CRLF)
	// Response when the channel modes are altered.
	#define	RPL_CHANGEMODE(hostname, channelname, mode, arguments) (":" + hostname + " MODE #" + channelname + " " + mode + " " + arguments + CRLF)
// 329, response with time of creation of the channel.
#define	RPL_CREATIONTIME(displayname, channelname, creationtime) (":329 " + displayname + " #" + channelname + " " + creationtime + CRLF)
// 331, response to TOPIC when no topic is set.
#define	RPL_NOTOPIC(displayname, channelname) (":331 " + displayname + " #" + channelname + " :No topic is set" + CRLF)
// 332, response to TOPIC with the set topic.
#define	RPL_TOPIC(displayname, channelname, topic) (":332 " + displayname + " #" + channelname + " :" + topic + "\r\n")
// 333, response to TOPIC with the set topic and the user who set it.
#define	RPL_TOPICWHOTIME(displayname, channelname, topic) (":333 " + displayname + " #" + channelname + " " + displayname + " " + get_channel_by_name(channelname)->get_creation_auto() + CRLF)
// 341, returned by the server to indicate that the attempted INVITE message was successful and is being passed onto the end client.
#define	RPL_INVITING(displayname, invitee, channelname) (":341 " + displayname + ":Inviting " + invitee + " to #" + channelname + CRLF)
// 353, response to NAMES.
#define	RPL_NAMREPLY(displayname, channelname, clientslist) (":353 " + displayname + " #" + channelname + " :" + clientslist + CRLF)
// 366, termination of an RPL_NAMREPLY list.
#define	RPL_ENDOFNAMES(displayname, channelname) (":366 " + displayname + " #" + channelname + " :End of NAMES list" + CRLF)
	// Response when a client changes their display name.
	#define	RPL_NAMECHANGE(old_displayname, displayname) (":" + old_displayname + " NICK " + displayname + CRLF)
	// Response when a client joins a channel.
	#define	RPL_JOIN(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN #" + channelname + CRLF)

// 401, used to indicate the nickname parameter supplied to a command is currently unused.
#define	ERR_NOSUCHNICK(channelname, name) (":401 #" + channelname + " " + name + " :No such nick/channel" + CRLF)
// 403, used to indicate the given channel name is invalid, or does not exist.
#define	ERR_NOSUCHCHANNEL(displayname, channelname) (":403 " + displayname + " " + channelname + " :No such channel" + CRLF)
// 407, the given target(s) for a command are ambiguous in that they relate to too many targets. Returned to a client which is attempting to send a PRIVMSG/NOTICE using the user@host destination format and for a user@host which has several occurrences.
#define	ERR_TOOMANYTARGETS(displayname) (":407 " + displayname + " :Duplicate recipients. No message delivered" + CRLF)
// 404, sent to a user who is either (a) not on a channel which is mode +n or (b) not a chanop (or mode +v) on a channel which has mode +m set and is trying to send a PRIVMSG message to that channel.
#define	ERR_CANNOTSENDTOCHAN(displayname, channelname) (":404 " + displayname + " #" + channelname + " :Cannot send to channel" + CRLF)
// 405, sent to a user when they have joined the maximum number of allowed channels and they tried to join another channel.
#define	ERR_TOOMANYCHANNELS(channelname) (":405 " + channelname + " :You have joined too many channels" + CRLF)
// 411, returned when no recipient is given with a command.
#define	ERR_NORECIPIENT(displayname, command) (":411 " + displayname + " :No recipient given (" + command + ")" + CRLF)
// 412, returned when NOTICE/PRIVMSG is used with no message given.
#define	ERR_NOTEXTTOSEND(displayname) (":412 " + displayname + " :No text to send" + CRLF)
// 421, returned when the given command is unknown to the server (or hidden because of lack of access rights).
#define	ERR_UNKNOWNCOMMAND(displayname, command) (":421 " + displayname + " " + command + " :Unknown command" + CRLF)
// 431, returned when a nickname parameter expected for a command isn't found.
#define	ERR_NONICKNAMEGIVEN(displayname) (":431 " + displayname + " :No nickname given" + CRLF)
// 432, returned after receiving a NICK message which contains a nickname which is considered invalid, such as it's reserved ('anonymous') or contains characters considered invalid for nicknames.
#define	ERR_ERRONEUSNICKNAME(displayname) (":432 " + displayname + " :Erroneous nickname" + CRLF)
// 433, returned by the NICK command when the given nickname is already in use.
#define	ERR_NICKNAMEINUSE(displayname) (":433 " + displayname + " :Nickname is already in use" + CRLF)
// 441, returned by the server to indicate that the target user of the command is not on the given channel.
#define	ERR_USERNOTINCHANNEL(displayname, channelname) (":441 " + displayname + "#" + channelname + " :They aren't on that channel" + CRLF)
// 442, returned by the server whenever a client tries to perform a channel effecting command for which the client is not a member.
#define	ERR_NOTONCHANNEL(displayname, channelname) (":442 " + displayname + " #" + channelname + " :You're not on that channel" + CRLF)
// 443, returned when a client tries to invite a user to a channel they're already on.
#define	ERR_USERONCHANNEL(displayname, channelname) (":443 " + displayname + " #" + channelname + " :is already on channel" + CRLF)
// 451, returned by the server to indicate that the client must be registered before the server will allow it to be parsed in detail.
#define	ERR_NOTREGISTERED(displayname) (":451 " + displayname + " :You have not registered" + CRLF)
// 461, returned by the server by any command which requires more parameters than the number of parameters given.
#define	ERR_NEEDMOREPARAMS(displayname) (":461 " + displayname + " :Not enough parameters" + CRLF)
// 462, returned by the server to any link which attempts to register again.
#define	ERR_ALREADYREGISTRED(displayname) (":462 " + displayname + " :Unauthorized command (already registered)" + CRLF)
// 464, returned by the PASS command to indicate the given password was required and was either not given or was incorrect.
#define	ERR_PASSWDMISMATCH(displayname) (":464 " + displayname + " :Password incorrect" + CRLF)
// 467, returned when the channel key for a channel has already been set.
#define	ERR_KEYSET(channelname) (":467 #" + channelname + " :Channel key already set" + CRLF)
// 471, returned when attempting to join a channel which is set +l and is already full.
#define	ERR_CHANNELISFULL(displayname, channelname) (":471 " + displayname + " #" + channelname + " :Cannot join channel (+l)" + CRLF)
// 472, returned when a given mode is unknown.
#define	ERR_UNKNOWNMODE(displayname, channelname, mode) (":472 " + displayname + " #" + channelname + " " + mode + " :is unknown mode char to me for this channel" + CRLF)
// 473, returned when attempting to join a channel which is invite only without an invitation.
#define	ERR_INVITEONLYCHAN(displayname, channelname) (":473 " + displayname + " #" + channelname + " :Cannot join channel (+i)" + CRLF)
// 475, returned when attempting to join a key-locked channel either without a key or with the wrong key.
#define	ERR_BADCHANNELKEY(displayname, channelname) (":475 " + displayname + " #" + channelname + " :Cannot join channel (+k)" + CRLF)
// 482, returned by any command requiring operator privileges to indicate the operation was unsuccessful.
#define	ERR_CHANOPRIVSNEEDED(displayname, channelname) (":482 #" + channelname + " :You're not channel operator" + CRLF)
	// 696?
	#define	ERR_NEEDMODEPARAM(channelname, mode) ("#" + channelname + " * :You must specify a parameter for the key mode " + mode + CRLF)
	// 696? 
	#define	ERR_INVALIDMODEPARAM(channelname, mode) ("#" + channelname + " :Invalid mode parameter " + mode + CRLF)

#endif
