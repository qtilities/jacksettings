#include "src/utils.h"

#include <sstream>

namespace jack
{
std::vector<char *> argsFrom(std::string command)
{
	command.insert (0, "dummy ");
	std::vector<char *> args;
	std::istringstream iss(command);
	std::string token;
	while(iss >> token)
	{
		char *arg = new char[token.size() + 1];
		copy(token.begin(), token.end(), arg);
		arg[token.size()] = '\0';
		args.push_back(arg);
	}
	return args;
}
} // namespace jack
