#pragma once

#include <string>
#include <vector>

std::vector<std::string> sc_split(std::string const& string, const char delim = ' ');
std::vector<std::wstring> sc_split(std::wstring const& string, const wchar_t delim = L' ');
