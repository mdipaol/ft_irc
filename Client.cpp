#include "Client.hpp"

Client::Client() : _fd(-1), _isRegister(true), _passTaken(true), _nickname("Manuel"), _user("mdi-paol"), _startReg(time(0))
{
}

Client::Client(int fd) : _fd(fd), _isRegister(false), _passTaken(false), _nickname(""), _user(""), _startReg(time(0))
{
}

Client &Client::operator=(const Client &obj)
{
	_fd = obj._fd;
	return *this;
}

Client::~Client()
{
}

bool Client::getIsRegistered() const {return _isRegister;}

void Client::setIsRegistered(bool reg){_isRegister = reg;}

bool Client::getPassTaken() const{return _passTaken;}

void Client::setPassTaken(bool pass){_passTaken = pass;}

const std::string &Client::getNickname() const{return _nickname;}

void Client::setNikcname(const std::string &nickname){_nickname = nickname;}

const std::string &Client::getUser() const{return _user;}

void Client::setUser(const std::string &user){_user = user;}

const time_t & Client::getStartReg() const{return _startReg;}

const int &Client::getFd() const{return _fd;}

const std::string &Client::getBuffer() const {return _buffer;}

const std::vector<Channel *>	&Client::getJoinedChannels() const { return _joinedChannels; }

void	Client::addChannel(Channel *channel)
{
	if (!channel)
		return;
	if (std::find(_joinedChannels.begin(), _joinedChannels.end(), channel) == _joinedChannels.end())
	{
		_joinedChannels.push_back(channel);
	}
}

void Client::setBuffer(const std::string &buffer) {_buffer = buffer;}

void Client::removeJoined(Channel *ch)
{
	std::vector<Channel *>::iterator it = std::find(_joinedChannels.begin(), _joinedChannels.end(), ch);
	if (!ch || it == _joinedChannels.end())
		return;
	_joinedChannels.erase(it);
}

void Client::deleteFromChannels(Server &server)
{
	Channel *tmpCh = NULL;
	std::vector<Channel *>	cpy(_joinedChannels);
	
	for(std::vector<Channel *>::iterator it = cpy.begin(); it != cpy.end(); ++it)
	{
		if(*it)
		{
			tmpCh = *it;
			(tmpCh)->deleteClientFromChannel(_nickname);
			if (!tmpCh->getSize())
				server.deleteChannel(tmpCh->getName());
			else
			{
				std::string RPL_PART = ":" + _nickname + "!" + _user + "@localhost QUIT :" + _nickname + " disconnected from the server\r\n";
				tmpCh->sendToAll(RPL_PART);
			}
		}
	}
	_joinedChannels.clear();
}
