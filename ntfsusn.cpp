#include <windows.h>
#include <wchar.h>
#include <winnt.h>
#include <winioctl.h>
#include <winerror.h>

#include <string>
#include <memory>
#include "ntfsusn.h"

#include <iostream>

ULONGLONG get_root_usn(const wchar_t driveletter, USN_RECORD* pusn_record)
{
	std::wstring strdrive;
	strdrive = L"\\\\.\\";
	strdrive += driveletter;
	strdrive += L":\\";
	USN root_usn = 0;

	HANDLE rootdir_handle;
	rootdir_handle = CreateFile(strdrive.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (rootdir_handle == INVALID_HANDLE_VALUE)
	{
		return root_usn;
	}

	USN_RECORD usn_record{};
	DWORD dwBytesReturned = 0;
	if (DeviceIoControl(rootdir_handle, FSCTL_READ_FILE_USN_DATA, NULL, 0, &usn_record, sizeof(usn_record), &dwBytesReturned, NULL) == TRUE)
	{
		root_usn = usn_record.FileReferenceNumber;
		if (pusn_record != nullptr)
			std::memcpy((void*)pusn_record, (void*)&usn_record, sizeof(USN_RECORD));
	}
	else
	{
		std::wcout << L"FSCTL_READ_FILE_USN_DATA error - " << GetLastError() << std::endl;
	}

	CloseHandle(rootdir_handle);

	return root_usn;
}

int enum_all_usn(const wchar_t driveletter, enum_callback handler, void* pUserData)
{
	//std::function<enum_callback> const handler = f;

	std::wstring strdrive;
	strdrive = L"\\\\.\\";
	strdrive += driveletter;
	strdrive += L":";

	HANDLE hDrive = CreateFileW(strdrive.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDrive == INVALID_HANDLE_VALUE)
	{
		return ERROR_USNQUERY_CREATEFILEERROR;
	}

	USN_JOURNAL_DATA usnJournal{};
	DWORD dwBytesReturned = 0;
	BOOL fResult = DeviceIoControl(hDrive, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &usnJournal, sizeof(usnJournal), &dwBytesReturned, NULL);
	if (fResult == FALSE)
	{
		CloseHandle(hDrive);
		return ERROR_USNQUERY_DEVICEIOERROR;
	}

	MFT_ENUM_DATA_V0 usnReadData{};
	PUSN_RECORD pUsnRecord;
	dwBytesReturned = 0;

	usnReadData.StartFileReferenceNumber = 0;
	usnReadData.LowUsn = 0;
	usnReadData.HighUsn = usnJournal.MaxUsn;

	int sizeofbuffer = 1024 * 1024;
	std::unique_ptr<char[]>  buffer(new char[sizeofbuffer] {});

	while (1)
	{
		if (!DeviceIoControl(hDrive, FSCTL_ENUM_USN_DATA, &usnReadData, sizeof(usnReadData), buffer.get(), sizeofbuffer, &dwBytesReturned, NULL))
		{
			DWORD dwError = GetLastError();
			if (dwError != ERROR_HANDLE_EOF)
			{
				CloseHandle(hDrive);
				SetLastError(dwError);
				return ERROR_USNQUERY_DEVICEIOERROR;
			}
			break;
		}

		if (dwBytesReturned < sizeof(USN) + sizeof(USN_RECORD))
		{
			CloseHandle(hDrive);
			return ERROR_USNQUERY_RETURNSIZEERROR;
		}

		dwBytesReturned -= sizeof(USN);

		pUsnRecord = (PUSN_RECORD)(((PUCHAR)buffer.get()) + sizeof(USN));

		while (dwBytesReturned > 0)
		{
			if (handler(pUserData, hDrive, pUsnRecord) == false)
			{
				CloseHandle(hDrive);
				return ERROR_USNQUERY_STOPPED;
			}

			dwBytesReturned -= pUsnRecord->RecordLength;
			pUsnRecord = (PUSN_RECORD)(((PCHAR)pUsnRecord) + pUsnRecord->RecordLength);
		}

		usnReadData.StartFileReferenceNumber = *(USN*)buffer.get();
	}

	CloseHandle(hDrive);
	return ERROR_USNQUERY_SUCCESS;
}

int enum_parent_usn(HANDLE hDrive, ULONGLONG ParentUsnJournalID, ULONGLONG RootJournalID, enum_callback handler, void* pUserData)
{
	if (RootJournalID == 0)
		return ERROR_USNQUERY_INVALIDROOT;

	//MFT_ENUM_DATA usnReadData{};
	MFT_ENUM_DATA_V0 usnReadData{};
	PUSN_RECORD pUsnRecord;
	DWORD dwBytesReturned = 0;

	usnReadData.StartFileReferenceNumber = ParentUsnJournalID;
	usnReadData.LowUsn = 0;
	usnReadData.HighUsn = ParentUsnJournalID;

	int sizeofbuffer = 64 + _MAX_PATH * 2 * 2;
	std::unique_ptr<char[]>  buffer(new char[sizeofbuffer] {});

	if (!DeviceIoControl(hDrive, FSCTL_ENUM_USN_DATA, &usnReadData, sizeof(usnReadData), buffer.get(), sizeofbuffer, &dwBytesReturned, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError != ERROR_HANDLE_EOF)
		{
			//SetLastError(dwError);
			return ERROR_USNQUERY_DEVICEIOERROR;
		}
	}

	if (dwBytesReturned < sizeof(USN) + sizeof(USN_RECORD))
	{
		return ERROR_USNQUERY_RETURNSIZEERROR;
	}

	dwBytesReturned -= sizeof(USN);

	pUsnRecord = (PUSN_RECORD)(((PUCHAR)buffer.get()) + sizeof(USN));

	if ((pUsnRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return ERROR_USNQUERY_ATTRIBUTE;
	}

	if (RootJournalID != pUsnRecord->ParentFileReferenceNumber)
	{
		enum_parent_usn(hDrive, pUsnRecord->ParentFileReferenceNumber, RootJournalID, handler, pUserData);
	}

	if (handler(pUserData, hDrive, pUsnRecord) == false)
		return ERROR_USNQUERY_STOPPED;

	return ERROR_USNQUERY_SUCCESS;
}


