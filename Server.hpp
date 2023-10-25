#pragma once

#include "utils.hpp"

class Client;

class Server
{
private:
	uint16_t const				_port;
	std::string const			_psw;
	bool const					_isPassword;
	std::map<int, Client>		_clients;
public:
	Server(const std::string &port, const std::string &psw);
	~Server();


	void	run();
	void	msgAnalyzer(Client &client, char *msg);
	void	registration(Client &client, std::string &msg);
};

