# usnquery

Simple file search utility using NTFS MFT / USN Change journal 


[Change Journals - Win32 apps | Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/fileio/change-journals)


```
typedef struct {
 
    DWORD RecordLength;
    WORD   MajorVersion;
    WORD   MinorVersion;
    DWORDLONG FileReferenceNumber;
    DWORDLONG ParentFileReferenceNumber;
    USN Usn;
    LARGE_INTEGER TimeStamp;
    DWORD Reason;
    DWORD SourceInfo;
    DWORD SecurityId;
    DWORD FileAttributes;
    WORD   FileNameLength;
    WORD   FileNameOffset;
    WCHAR FileName[1];
 
} USN_RECORD_V2, *PUSN_RECORD_V2;
```

used FileReferenceNumber and ParentFileReferenceNumber members for reconstructing directory  


### Logic
1. first, get ROOT file USN and save
2. enumerate all files in TargetDrive and call callback
3. in callback, compare file name with finding option
4. if found, get parent folder names to root
5. continue enum files (2nd step)
  

### Execute
- need administrator previledge
- cannot find in recycle folder
- cannot find hardlink files

```
c:\> fsutil hardlink list C:\Windows\SysWOW64\eventvwr.exe
\Windows\SysWOW64\eventvwr.exe
\Windows\WinSxS\wow64_eventviewersettings_31bf3856ad364e35_10.0.25267.1000_none_e931c8cba9c8100d\eventvwr.exe
```


### Usage
```
 usnquery [option] <driveletter>
 driveletter : * = all drives
 -f %1 : filaname filter
 -e %1|%2|%3 : extension filter
 -d : directory only
```

### Example
```
PS D:\git\usnquery\Release> .\usnquery.exe -f eventvwr -e exe C
file count = 1601532, directory count = 298161
eventvwr.exe - C:\Windows\WinSxS\amd64_eventviewersettings_31bf3856ad364e35_10.0.25295.1000_none_660b78eb9cfa762d\
eventvwr.exe - C:\Windows\WinSxS\wow64_eventviewersettings_31bf3856ad364e35_10.0.25295.1000_none_7060233dd15b3828\
```

