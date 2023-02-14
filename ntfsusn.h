#pragma once

#include <windows.h>
#include <wchar.h>
#include <winnt.h>
#include <winioctl.h>
#include <string>
#include <vector>

typedef bool (*enum_callback)(void*, HANDLE, const PUSN_RECORD);

ULONGLONG get_root_usn(const wchar_t driveletter, USN_RECORD* pusn_record);
int enum_all_usn(const wchar_t driveletter, enum_callback f, void* pUserData);
int enum_parent_usn(HANDLE hDrive, ULONGLONG ParentUsnJournalID, ULONGLONG RootJournalID, enum_callback f, void* pUserData);

#define ERROR_USNQUERY_SUCCESS 0
#define ERROR_USNQUERY_PARAMERROR 1
#define ERROR_USNQUERY_CREATEFILEERROR 2   // GetLastError() -> cause
#define ERROR_USNQUERY_DEVICEIOERROR 3     // GetLastError() -> cause
#define ERROR_USNQUERY_RETURNSIZEERROR 4
#define ERROR_USNQUERY_STOPPED 5
#define ERROR_USNQUERY_ATTRIBUTE 6
#define ERROR_USNQUERY_INVALIDROOT 7
#define ERROR_USNQUERY_INVALIDUSN 8

