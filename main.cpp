/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnascime <cnascime@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 10:48:59 by cnascime          #+#    #+#             */
/*   Updated: 2025/02/11 13:13:11 by cnascime         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "client.hpp"
#include "channel.hpp"

bool	isport_valid(std::string port);

int	main(int argc, char** argv)
{
	if (argc != 3 || argv[2] == NULL || std::strlen(argv[2]) > 20
		|| isport_valid(argv[1]) == false)
		return (1);

	Server myserver;
	try
	{
		// Raises antennae.
		signal(SIGINT, Server::antenna); // Ctrl+C.
		signal(SIGQUIT, Server::antenna); // Ctrl+\.
		myserver.start(std::atoi(argv[1]), argv[2]);
	}
	catch (const std::exception &e)
	{
		myserver.close_fd();
		std::cerr << e.what() << std::endl;
	};
	std::cout << "Servidor encerrado." << std::endl; //! MANTER MENSAGEM OU NÃO? DEIXAR POR ORA PARA TESTES
};

// Checks if the port is a number between 1024 and 65535.
bool	isport_valid(std::string port)
{
	bool is_numeric = (port.find_first_not_of("0123456789") == std::string::npos);

	int number = std::atoi(port.c_str());

	bool within_range = (number >= 1024 && number <= 65535);

	if (is_numeric && within_range)
		return (true);
	return (false);
};
