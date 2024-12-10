#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <ntstatus.h>

typedef LONG(NTAPI* PNtProcessFunc)(IN HANDLE);

// Helper function to get function pointers for NtSuspendProcess or NtResumeProcess
PNtProcessFunc GetNtProcessFunction(LPCSTR functionName)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll");
    if (hNtdll == nullptr)
    {
        wprintf(L"Failed to get ntdll module handle.\n");
        return nullptr;
    }

    PNtProcessFunc pFunc = (PNtProcessFunc)GetProcAddress(hNtdll, functionName);
    if (pFunc == nullptr)
    {
        wprintf(L"Failed to get %S function address.\n", functionName);
    }
    return pFunc;
}

// Generic function to suspend or resume a process
BOOL ProcessAction(HANDLE hProcessHandle, LPCSTR action)
{
    PNtProcessFunc pFunc = GetNtProcessFunction(action);
    if (pFunc != nullptr)
    {
        if (pFunc(hProcessHandle) == STATUS_SUCCESS)
        {
            wprintf(L"Process %S successfully.\n", action);
            return TRUE;
        }
        else
        {
            wprintf(L"Failed to %S the process.\n", action);
        }
    }
    return FALSE;
}

// Function to start the process in a suspended state
HANDLE StartProcessSuspended(LPCTSTR processPath)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Use a mutable buffer for the process command line (required by CreateProcess)
    TCHAR szCmdLine[MAX_PATH];
    _tcscpy_s(szCmdLine, processPath);

    // Start the process in a suspended state
    if (CreateProcessW(nullptr, szCmdLine, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi))
    {
        wprintf(L"Process started in suspended state.\n");
        CloseHandle(pi.hThread);
        return pi.hProcess;
    }
    else
    {
        wprintf(L"Failed to start process. Error code: %d\n", GetLastError());
        return nullptr;
    }
}

// Function to resume the process and then exit the current program
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

int wmain(int argc, TCHAR* argv[])
{
    if (argc < 2)
    {
        wprintf(L"Usage: %s <path_to_executable>\n", argv[0]);
        return 1;
    }

    LPCTSTR szExecutablePath = argv[1];
    wprintf(L"Starting %s in suspended state...\n", szExecutablePath);

    HANDLE hProcess = StartProcessSuspended(szExecutablePath);
    if (hProcess != nullptr)
    {
        wprintf(L"Press Enter to resume the process...\n");
        std::cin.get();

        ResumeProcessAndExit(hProcess);
    }
    else
    {
        wprintf(L"Press Enter to exit...\n");
        std::cin.get();
    }

    return 0;
}
