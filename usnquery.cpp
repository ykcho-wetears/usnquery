
#include <WinError.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

#include "ntfsusn.h"
#include "winerrors.h"

#include "option.h"

bool callback_enum_all(void* pUserData, HANDLE hDrive, const PUSN_RECORD pUsnRecord);
bool callback_enum_parent(void* pUserData, HANDLE hDrive, const PUSN_RECORD pUsnRecord);

bool callback_enum_justcount(void* pUserData, HANDLE hDrive, const PUSN_RECORD pUsnRecord);
int file_count = 0;
int dir_count = 0;

int wmain(int argc, wchar_t* argv[])
{
	setlocale(LC_ALL, "");

	options option;
	if (!option.parse(L"usnquery", argc, argv))
	{
		option.print();
		return 0;
	}

	wchar_t d_start = option.targetdrive;
	wchar_t d_end = option.targetdrive;
	if (option.targetdrive == L'*')
	{
		d_start = L'C';
		d_end = L'Z';
	}

	for (wchar_t d = d_start; d <= d_end; d++)
	{
		enum_all_usn(d, callback_enum_justcount, nullptr);
		std::wcout << L"file count = " << file_count << L", directory count = " << dir_count << std::endl;

		USN_RECORD usn_root{};
		USN root_usn = get_root_usn(d, &usn_root);

		option_and_state optioncopy = option;
		optioncopy.targetdrive = d;
		optioncopy.root_usn = root_usn;

		int ret = enum_all_usn(d, callback_enum_all, (void*)&optioncopy);
		if (ret == ERROR_USNQUERY_CREATEFILEERROR)
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_FILE_NOT_FOUND && option.targetdrive == L'*')
				continue;
		}
		if (ret == ERROR_USNQUERY_CREATEFILEERROR || ret == ERROR_USNQUERY_DEVICEIOERROR)
		{
			std::wcout << L"enum_all_usn (" << optioncopy.targetdrive << L":) " << "failed(" << ret << L") ";
			DWORD dwError = GetLastError();
			std::wcout << L"LastError = " << dwError << L" : " << GetLastErrorStringW(dwError); // << std::endl;
		}
	}
	return 0;
}

bool callback_enum_all(void *pUserData, HANDLE hDrive, const PUSN_RECORD pUsnRecord)
{
	if (pUsnRecord == nullptr || pUserData == nullptr)
		return false;

	option_and_state* pOption = (option_and_state*)pUserData;

	if (pOption->onlydirectory == true)
	{
		if ((pUsnRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) // if not directory
			return true;
	}

	std::wstring strFileName(pUsnRecord->FileName, pUsnRecord->FileNameLength / 2);
	std::wstring strLowerCaseFileName(strFileName);
	std::transform(strLowerCaseFileName.begin(), strLowerCaseFileName.end(), strLowerCaseFileName.begin(), tolower);

	if (pOption->extfilters.empty() == false)
	{
		size_t pos = strLowerCaseFileName.find_last_of(L".");
		if (pos == std::string::npos)
			return true;

		std::wstring ext = strLowerCaseFileName.substr(pos + 1);
		if (!ext.empty())
		{
			if (std::find(pOption->extfilters.begin(), pOption->extfilters.end(), ext) == pOption->extfilters.end())
				return true;
		}
	}

	if (pOption->namefilters.empty() == false)
	{
		size_t found = std::string::npos;
		for (auto it : pOption->namefilters)
		{
			found = strLowerCaseFileName.find(it.c_str(), 0);
			if (found != std::string::npos)
				break;
		}
		if (found == std::string::npos)
			return true;
	}

	std::wstring strDirectory;
	strDirectory = pOption->targetdrive;
	strDirectory += L":\\";

	pOption->directory_path = strDirectory;
	int ret = enum_parent_usn(hDrive, pUsnRecord->ParentFileReferenceNumber, pOption->root_usn, callback_enum_parent, pOption);
	if (ret != 0)
	{
		//std::wcout << strFileName << L" - enum_parent_usn error - " << ret << std::endl;
	}
	else
	{
		std::wcout << strFileName << L" - ";
		std::wcout << pOption->directory_path << std::endl;
	}

	return true;
}

bool callback_enum_parent(void* pUserData, HANDLE hDrive, const PUSN_RECORD pUsnRecord)
{
	if (pUsnRecord == nullptr || pUserData == nullptr)
		return false;

	option_and_state *pstate = (option_and_state*)pUserData;

	std::wstring strFileName(pUsnRecord->FileName, pUsnRecord->FileNameLength / 2);

	pstate->directory_path.append(strFileName);
	pstate->directory_path.append(L"\\");

	return true;
}

bool callback_enum_justcount(void* pUserData, HANDLE hDrive, const PUSN_RECORD pUsnRecord)
{
	if (pUsnRecord == nullptr)
		return false;

	if ((pUsnRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) // if not directory
		file_count++;
	else
		dir_count++;

	return true;
}
