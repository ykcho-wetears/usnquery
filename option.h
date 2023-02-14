#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "sc_split.h"

struct options
{
    wchar_t targetdrive;
    std::vector<std::wstring> namefilters;
    std::vector<std::wstring> extfilters;
    bool onlydirectory;

    bool verbose;

    options()
    {
        targetdrive = L'C';
        onlydirectory = false;
        verbose = false;
    }

    void print()
    {
        std::cout << " usnquery [option] <driveletter>" << std::endl;
        std::cout << " driveletter : * = all drives" << std::endl;
        std::cout << " -f %1 : filaname filter" << std::endl;
        std::cout << " -e %1|%2|%3 : extension filter" << std::endl;
        std::cout << " -d : directory only" << std::endl;
    }

    bool parse(const wchar_t* pexename, int argc, wchar_t* argv[])
    {
        std::wstring strexename(pexename);
        targetdrive = 0;
        onlydirectory = false;

        verbose = false;

        if (argc <= 1)
        {
            return false;
        }

        for (int i = 1; i < argc; i++)
        {
            if (std::wcscmp(argv[i], L"-f") == 0)
            {
                if (i + 2 >= argc)
                    return false;

                std::wstring strfilter(argv[i + 1]);
                std::transform(strfilter.begin(), strfilter.end(), strfilter.begin(), tolower);

                namefilters = sc_split(strfilter, L'|');

                i++;
                continue;
            }
            else if (std::wcscmp(argv[i], L"-e") == 0)
            {
                if (i + 2 >= argc)
                    return false;

                std::wstring strfilter(argv[i + 1]);
                std::transform(strfilter.begin(), strfilter.end(), strfilter.begin(), tolower);

                extfilters = sc_split(strfilter, L'|');

                i++;
                continue;
            }
            else if (std::wcscmp(argv[i], L"-d") == 0)
            {
                onlydirectory = true;

                continue;
            }
            else
            {
                if (targetdrive != 0)
                    return false;

                targetdrive = std::toupper(argv[i][0]);
            }
        }

        return (strexename == L"usnquery");
    }
};

struct option_and_state : public options
{
    option_and_state(options& option)
    {
        targetdrive = option.targetdrive;
        namefilters = option.namefilters;
        extfilters = option.extfilters;
        onlydirectory = option.onlydirectory;
        verbose = option.verbose;
    }
    std::wstring directory_path;
    __int64 root_usn;
};
