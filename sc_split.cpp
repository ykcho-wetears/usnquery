
#include <string>
#include <sstream>
#include <vector>

std::vector<std::string> sc_split(std::string const& string, const char delim)
{
	std::vector<std::string> group;
	std::istringstream is(string);
	std::string str;
	while (std::getline(is, str, delim))
	{
		group.emplace_back(str);
	}

	return group;
}

std::vector<std::wstring> sc_split(std::wstring const& string, const wchar_t delim)
{
	std::vector<std::wstring> group;
	std::wistringstream is(string);
	std::wstring str;
	while (std::getline(is, str, delim))
	{
		group.emplace_back(str);
	}

	return group;
}

