#include "utils.hpp"

extern bool running;

void		sigHandler(int sig)
{
	if (sig == SIGINT)
	{
		std::cout << std::endl << "\033[F" << "Server closed!"<< std::endl;
		running = false;
	}
}

bool		isNumber(const std::string &str)
{
	for(size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return (false);
	}
	return (true);
}

uint16_t	portConverter(const std::string &port)
{
	uint32_t	n = 0;
	size_t		size = port.size();

	for(size_t i = 0; i < size; i++)
	{
		n = (n * 10) + port[i] - 48;
		if (n > std::numeric_limits<uint16_t>::max() || !std::isdigit(port[i]))
			throw(std::invalid_argument("Invalid port"));
	}
	if (n < 1024)
		throw(std::invalid_argument("Invalid port"));
	return (n);
}

std::vector<std::string>	ft_split(const std::string &str, const char delimiter)
{
	std::istringstream			iss(str);
	std::string					param;
	std::vector<std::string>	vParam;

	while (std::getline(iss, param, delimiter))
		if (!param.empty())
			vParam.push_back(param);
	return(vParam);
}

bool inSet(const std::string &string, const std::string &set)
{
	for (size_t i = 0; i < set.size(); ++i)
	{
		if (string.find(set[i]) != std::string::npos)
			return (true);
	}
	return (false);
}

std::string	toLowerString(std::string string)
{
	std::string cpy(string);

	for(size_t i = 0; i < cpy.size(); ++i)
		cpy[i] = std::tolower(cpy[i]);
	return (cpy);
}

bool 	ichar_equals(char a, char b)
{
    return (std::tolower(static_cast<unsigned char>(a)) ==
		std::tolower(static_cast<unsigned char>(b)));
}

bool 	compareInsensitive(const std::string& a, const std::string& b)
{
    return ((a.size() == b.size()) &&
		std::equal(a.begin(), a.end(), b.begin(), ichar_equals));
}
