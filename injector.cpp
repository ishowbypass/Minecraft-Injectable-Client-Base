#include "windows.h"
#include "commdlg.h"

// ishowbypass(手淫) 2025

void main()
{
    OPENFILENAMEW ofn = {sizeof(ofn)};
    WCHAR dllPath[MAX_PATH];
    ofn.lpstrFile = dllPath;
    ofn.nMaxFile = MAX_PATH;

    if (GetOpenFileNameW(&ofn))
    {
        if (HWND mcWindow = FindWindowW(L"GLFW30", 0))
        {
            if (DWORD mcPID; GetWindowThreadProcessId(mcWindow, &mcPID))
            {
                if (HANDLE mcProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, 0, mcPID))
                {
                    if (LPVOID allocatedPathString = VirtualAllocEx(mcProcess, 0, sizeof(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))
                    {
                        if (WriteProcessMemory(mcProcess, allocatedPathString, dllPath, sizeof(dllPath), 0))
                        {
                            if (HMODULE kernel32Module = GetModuleHandleW(L"kernel32.dll"))
                            {
                                if (FARPROC loadLibraryAddress = GetProcAddress(kernel32Module, "LoadLibraryW"))
                                {
                                    if (HANDLE remoteThread = CreateRemoteThread(mcProcess, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryAddress), allocatedPathString, 0, 0))
                                    {
                                        WaitForSingleObject(remoteThread, INFINITE);
                                        CloseHandle(remoteThread);
                                    }
                                }
                            }
                        }
                        VirtualFreeEx(mcProcess, allocatedPathString, 0, MEM_RELEASE);
                    }
                    CloseHandle(mcProcess);
                }
            }
        }
    }
}