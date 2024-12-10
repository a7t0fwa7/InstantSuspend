#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <ntstatus.h>
#include <tlhelp32.h>

typedef LONG(NTAPI* PNtProcessFunc)(IN HANDLE);

PNtProcessFunc GetNtProcessFunction(LPCSTR pszFunctionName)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll");
    if (hNtdll == nullptr)
    {
        wprintf(L"Failed to get ntdll module handle.\n");
        return nullptr;
    }

    PNtProcessFunc pFunc = (PNtProcessFunc)GetProcAddress(hNtdll, pszFunctionName);
    if (pFunc == nullptr)
    {
        wprintf(L"Failed to get %S function address.\n", pszFunctionName);
    }
    return pFunc;
}

BOOL ProcessAction(HANDLE hProcess, LPCSTR pszAction)
{
    PNtProcessFunc pFunc = GetNtProcessFunction(pszAction);
    if (pFunc != nullptr)
    {
        if (pFunc(hProcess) == STATUS_SUCCESS)
        {
            wprintf(L"Process %S successfully.\n", pszAction);
            return TRUE;
        }
        else
        {
            wprintf(L"Failed to %S the process.\n", pszAction);
        }
    }
    return FALSE;
}

HANDLE StartProcessSuspended(LPCTSTR pszProcessPath)
{
    STARTUPINFO siStartup = { sizeof(siStartup) };
    PROCESS_INFORMATION piProcessInfo;

    TCHAR szCmdLine[MAX_PATH];
    wcscpy_s(szCmdLine, pszProcessPath);

    if (CreateProcessW(nullptr, szCmdLine, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &siStartup, &piProcessInfo))
    {
        wprintf(L"Process started in suspended state.\n");
        CloseHandle(piProcessInfo.hThread);
        return piProcessInfo.hProcess;
    }
    else
    {
        wprintf(L"Failed to start process. Error code: %d\n", GetLastError());
        return nullptr;
    }
}

void ResumeProcessAndExit(HANDLE hProcess)
{
    if (ProcessAction(hProcess, "NtResumeProcess"))
    {
        wprintf(L"Process resumed successfully. Exiting current process.\n");
    }
    else
    {
        wprintf(L"Failed to resume the process.\n");
    }
    CloseHandle(hProcess);
    ExitProcess(0);
}

BOOL AwaitAndSuspendProcess(LPCTSTR pszProcessName)
{
    wprintf(L"Waiting for process %s to start...\n", pszProcessName);
    while (true)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
            wprintf(L"Failed to create process snapshot. Error code: %d\n", GetLastError());
            return FALSE;
        }

        PROCESSENTRY32 peProcessEntry;
        peProcessEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32FirstW(hSnapshot, &peProcessEntry))
        {
            do
            {
                if (_wcsicmp(peProcessEntry.szExeFile, pszProcessName) == 0)
                {
                    wprintf(L"Found process with PID: %d\n", peProcessEntry.th32ProcessID);
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, peProcessEntry.th32ProcessID);
                    if (hProcess != nullptr)
                    {
                        if (ProcessAction(hProcess, "NtSuspendProcess"))
                        {
                            wprintf(L"Process %s suspended successfully.\n", pszProcessName);
                            wprintf(L"Press Enter to resume the process...");
                            std::cin.get();

                            if (ProcessAction(hProcess, "NtResumeProcess"))
                            {
                                wprintf(L"Process %s resumed successfully.\n", pszProcessName);
                            }
                            CloseHandle(hProcess);
                            CloseHandle(hSnapshot);
                            return TRUE;
                        }
                        CloseHandle(hProcess);
                    }
                    else
                    {
                        wprintf(L"Failed to open process %s. Error code: %d\n", pszProcessName, GetLastError());
                    }
                }
            } while (Process32NextW(hSnapshot, &peProcessEntry));
        }

        CloseHandle(hSnapshot);
        Sleep(100);
    }
}

int wmain(int argc, TCHAR* argv[])
{
    if (argc < 3)
    {
        wprintf(L"Usage: InstantSuspend <command> <executable_name_or_path>\n");
        wprintf(L"\nCommands:\n");
        wprintf(L"  start <path_to_executable> - Starts a process in a suspended state\n");
        wprintf(L"  await <executable_name> - Waits for a process to start and suspends it\n");
        return 1;
    }

    LPCTSTR pszCommand = argv[1];
    LPCTSTR pszArgument = argv[2];

    if (_wcsicmp(pszCommand, L"start") == 0)
    {
        wprintf(L"Starting %s in suspended state...\n", pszArgument);
        HANDLE hProcess = StartProcessSuspended(pszArgument);
        if (hProcess != nullptr)
        {
            wprintf(L"Press Enter to resume the process...");
            std::cin.get();
            ResumeProcessAndExit(hProcess);
        }
    }
    else if (_wcsicmp(pszCommand, L"await") == 0)
    {
        if (AwaitAndSuspendProcess(pszArgument))
        {
            wprintf(L"Process management completed successfully.\n");
        }
        else
        {
            wprintf(L"Failed to manage the process.\n");
        }
    }
    else
    {
        wprintf(L"Unknown command: %s\n", pszCommand);
    }

    return 0;
}
