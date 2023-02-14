#pragma once

#include <Windows.h>

#include <string>


std::string GetLastErrorStringA(DWORD dwError);
std::wstring GetLastErrorStringW(DWORD dwError);