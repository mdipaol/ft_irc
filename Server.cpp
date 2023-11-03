#include "Server.hpp"
#include "Server.hpp"

Server::Server(const std::string &port, const std::string &psw) : _port(portConverter(port)), _psw(psw), _isPassword(psw.compare("") != 0)
{
	_commands["JOIN"] = Command::join;
	_commands["PRIVMSG"] = Command::privmsg;
	_commands["PING"] = Command::ping;
	_commands["KICK"] = Command::kick;
	_commands["INVITE"] = Command::invite;
	_commands["TOPIC"] = Command::topic;
	_commands["MODE"] = Command::mode;
	_commands["NICK"] = Command::nick;
	_commands["PASS"] = Command::pass;
}

Server::~Server()
{
}

const std::string	&Server::getPassword() const { return _psw; }


Client *Server::getClient(const std::string &clName)
{
	Client	*c = NULL;
	std::map<std::string, Client*>::const_iterator	it = _clients.find(clName);

	if (it != _clients.end())
		c = it->second;
	return(c);
}

void Server::updateNick(Client &client, const std::string &newName)
{
	_clients.erase(client.getNickname());
	_clients[newName] = &client;
	client.setNikcname(newName);
}

Client	*Server::getClientByFd(int fd) const
{
	std::map<std::string, Client*>::const_iterator	it = _clients.begin();
	std::map<std::string, Client*>::const_iterator	end = _clients.end();

	while (it != end)
	{
		if (it->second && it->second->getFd() == fd)
			return(it->second);
		++it;
	}

	std::list<Client*>::const_iterator	lit = _clientsNotRegistered.begin();
	std::list<Client*>::const_iterator	lend = _clientsNotRegistered.end();

	while (lit != lend)
	{
		if (*lit && (*lit)->getFd() == fd)
			return (*lit);
		++lit;
	}
	return (NULL);
}

Channel *Server::getChannel(const std::string &chName)
{
	std::map<std::string, Channel*>::iterator	it;
	if ((it = _channels.find(chName)) != _channels.end())
		return(_channels[chName]);
	return NULL;
}

void Server::deleteClientByFd(int fd)
{
	std::map<std::string, Client*>::iterator	it = _clients.begin();
	std::map<std::string, Client*>::iterator	end = _clients.end();

	while (it != end)
	{
		if (it->second)
		{
			if (it->second->getFd() == fd)
			{
				// Remove client from all channels
				// Delete client memory
				// Erase iterator from map
			}
		}
		else
			_clients.erase(it);
		++it;
	}
}

void	Server::run()
{
	signal(SIGINT, sigHandler);
	int serverSocket, newSocket;
	struct sockaddr_in serverAddr, newAddr;
	socklen_t addrSize;
	char buffer[512];

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		perror("socket");
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		perror("setsockopt");
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
		throw (std::runtime_error("fcntl-server"));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(this->_port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)))
		perror("bind");
	if (listen(serverSocket, MAX_QUEUE_CONN) == -1)
		perror("listen");

	std::cout << "Server is listening on port " << this->_port << "..." << std::endl;

	int epoll_fd = epoll_create1(0); // Create an epoll instance

	struct epoll_event event;
	event.events = EPOLLIN; // Monitor read events
	event.data.fd = serverSocket;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event); // Add the server socket to the epoll

	struct epoll_event arrEvent[MAX_CLIENT]; // Create an event array to store events

	while (running)
	{
		int ready_fds = epoll_wait(epoll_fd, arrEvent, MAX_CLIENT, -1); // Wait for events

		for (int i = 0; i < ready_fds; ++i)
		{
			if (arrEvent[i].data.fd == serverSocket)
			{
				newSocket = accept(serverSocket, (struct sockaddr*)&newAddr, &addrSize);
				if (newSocket == -1)
					perror("accept");
				if (fcntl(newSocket, F_SETFL, O_NONBLOCK) == -1)
					throw (std::runtime_error("fcntl-client"));
				event.data.fd = newSocket;
				event.events = EPOLLIN; // Monitor read events for the new socket
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newSocket, &event);
				std::cout << "Connection established with a client." << std::endl;
				
				// _clients.insert(std::make_pair(??, new Client(newSocket)));
					

				// Enter new fd in the list of not registered client

				_clientsNotRegistered.push_back(new Client(newSocket));
			}
			else
			{
				int clientSocket = arrEvent[i].data.fd;
				memset(buffer, 0, sizeof(buffer));
				int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
				std::cout << "Client: " << clientSocket << std::endl;
				// std::cout << buffer << std::endl;
				if (bytesReceived <= 0)
				{
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clientSocket, NULL);
					close(clientSocket);
					// _clients.erase(_clients.find(clientSocket));
					
					// Check if clientSocket is registered or not

					std::cout << "Client disconnected." << std::endl;
				}
				else
				{
					Client * c = NULL;
					c = getClientByFd(clientSocket);
					std::cout << "Address of client " << clientSocket << " : " << c << std::endl;
					if (c)
						msgAnalyzer(*c, buffer);
					//std::cout << "Client :" << buffer << std::endl;
				}
			}
		}
	}

	std::cout << "Sono uscito dal run" << std::endl;

	// DESTRUCTOR CALL

	// for(std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	// {
	// 	std::cout << "Closing fd " << it->first << std::endl;
	// 	send(it->first, "QUIT :Server disconnected!\r\n", 29, 0);
	// 	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
	// 	close(it->first);
	// }
	// close(epoll_fd);
	// close(serverSocket);
}

void	Server::msgAnalyzer(Client &client, const char *message)
{
	std::string msg = message;
	size_t		pos;

	msg = client.getBuffer() + msg;
	client.setBuffer("");

	while ((pos = msg.find('\n')) != std::string::npos)
	{
		std::string			line;
		std::istringstream	iss(msg);

		std::getline(iss, line);
		msg.erase(0, pos + 1);

		// if (line.size() && line[line.size() - 1] == '\r')
		// {
		// 	printf("puliziaTime\n");
		// 	line.erase(line.size() - 1, 1);
		// }

		if (client.getIsRegistered())
			cmdAnalyzer(client, line);
		else
			registration(client, line);

	}
	client.setBuffer(msg);
}

void	Server::welcomeMessage(Client &client)
{
	std::string serverName = ":SUCA";
	const int flags = MSG_DONTWAIT | MSG_NOSIGNAL;
	std::string RPL_WELCOME = serverName + " 001 " + client.getNickname() + " :Welcome to the 42 Internet Relay Network " + client.getNickname() + "\r\n";
	std::string RPL_YOURHOST = serverName + " 002 " + client.getNickname() + " :Hosted by Ale, Dami, Manu\r\n";
	std::string RPL_CREATED = serverName + " 003 " + client.getNickname() + " :This server was created in Nidavellir\r\n";

	send(client.getFd(), RPL_WELCOME.c_str(), RPL_WELCOME.length(), flags);
	send(client.getFd(), RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), flags);
	send(client.getFd(), RPL_CREATED.c_str(), RPL_CREATED.length(), flags);
}

void	Server::registration(Client &client, const std::string &msg)
{
	// std::istringstream iss(msg);
	// std::string token;
	// std::string info;
	std::vector<std::string>	params;
	std::string					cmd;

	params = ft_split(msg, ' ');
	if (params.size() < 2)
	{
		std::string error = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), error.c_str(), error.size(), 0);
		return;
	}

	cmd = params[0];
	params.erase(params.begin());

	if (!_isPassword)
		client.setPassTaken(true);

	if (!client.getPassTaken() && !cmd.compare("PASS"))
		Command::pass(*this, client, params);
	else if (!cmd.compare("NICK"))
		Command::nick(*this, client, params);
	else if (!cmd.compare("USER"))
		client.setUser(params[0]);
	// else if (!cmd.compare("TEST"))
	// 	sleep(15);
	if (!client.getNickname().empty() && !client.getUser().empty() && client.getPassTaken())
	{
		std::cout << client.getNickname() << " registered!" << std::endl;
		//send(client.getFd(), "Registration finished!\r\n", 25, 0);
		//send(client.getFd(), "Welcome to My IRC Server! Enjoy your stay.\r\n", 45, 0);
		std::cout << "ciao" << std::endl;
		client.setIsRegistered(true);
		welcomeMessage(client);

		_clientsNotRegistered.remove(&client);
		_clients[client.getNickname()] = &client;
	}
	std::cout << msg << std::endl;
}

static void	fillParam(std::vector<std::string> &vParam, std::istringstream &iss)
{
	std::string	param, last;


	while (std::getline(iss, param, ' '))
	{
		if (param.empty())
			continue;
		else if (param[0] == ':')
		{
			std::getline(iss, last, (char)EOF);
			param.erase(0, 1);
			if (last.size() + param.size())
			{
				std::cout << "ciao" << std::endl;
				if (!last.empty())
					vParam.push_back(param + " " + last);
				else
					vParam.push_back(param);
			}
		}
		else
			vParam.push_back(param);
	}
}

void	Server::cmdAnalyzer(Client &client, const std::string &msg)
{
	// pulire stringa /r/n
	std::vector<std::string>						vParam;
	std::string										cmd;
	std::istringstream								iss(msg);
	std::map<std::string, commandFunct>::iterator	it;

	std::cout << "\033[32m" << msg << "\033[0m" << std::endl;
	iss >> cmd;
	if ((it = _commands.find(cmd)) != _commands.end())
	{
		fillParam(vParam, iss);
		_commands[cmd](*this, client, vParam);
	}
	else
	{
		std::cout << "Unrecognized command" << std::endl;
	}
}

void	Server::sendJoin(const std::string &name, Client &client)
{
	std::string RPL_JOIN = ":" + client.getNickname() + "!" + client.getUser() + "@localhost JOIN :" + name + "\r\n";
	std::string RPL_NAMREPLY = ":ircserv 353 " + client.getNickname() + " = " + name + " :manuel\r\n";
	std::string RPL_NAMREPLY1 = ":ircserv 353 " + client.getNickname() + " = " + name + " :@ale @damiano\r\n";
	std::string RPL_ENDOFNAMES = ":ircserv 366 " + client.getNickname() + " " + name + " :End of NAMES list\r\n";

	send(client.getFd(), RPL_JOIN.c_str(), RPL_JOIN.size(), 0);
	send(client.getFd(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.size(), 0);
	send(client.getFd(), RPL_NAMREPLY1.c_str(), RPL_NAMREPLY1.size(), 0);
	send(client.getFd(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.size(), 0);
}

void	Server::setChannels(const std::string &name, const std::string &pass, Client &client)
{
	if (_channels.find(name) == _channels.end())
	{
		_channels.insert(std::make_pair(name, new Channel(name, pass, &client)));
		sendJoin(name, client);
	}
	else
	{
		if (!_channels[name]->getPasskey().compare(pass))
		{
			_channels[name]->setClients(&client);
			sendJoin(name, client);
		}
		else
		{
			std::string	ERR_BADCHANNELKEY = "475 " + client.getNickname() + " " + name + " :Cannot join channel!\r\n";
			send(client.getFd(), ERR_BADCHANNELKEY.c_str(), ERR_BADCHANNELKEY.size(), 0);
		}
	}
}
