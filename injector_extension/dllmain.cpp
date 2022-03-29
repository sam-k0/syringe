// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "gms.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <libloaderapi.h>
#include <WinUser.h>
#include <tlHelp32.h>
using namespace std;
#define notInjectDLL
#define _CRT_SECURE_NO_WARNINGS


wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

#ifndef notInjectDLL
bool InjectDLL(DWORD pid, const string& DLL_Path)
{
    long dll_size = DLL_Path.length() + 1;
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (hProc == NULL)
    {
        cerr << "[!]Fail to open target process!" << endl;
        return false;
    }
    cout << "[+]Opening Target Process..." << endl;

    LPVOID MyAlloc = VirtualAllocEx(hProc, NULL, dll_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (MyAlloc == NULL)
    {
        cerr << "[!]Fail to allocate memory in Target Process." << endl;
        return false;
    }

    cout << "[+]Allocating memory in Target Process." << endl;
    int IsWriteOK = WriteProcessMemory(hProc, MyAlloc, DLL_Path.c_str(), dll_size, 0);
    if (IsWriteOK == 0)
    {
        cerr << "[!]Fail to write in Target Process memory." << endl;
        return false;
    }
    cout << "[+]Creating Remote Thread in Target Process" << endl;

    DWORD dWord;
    LPTHREAD_START_ROUTINE addrLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibrary("kernel32"), "LoadLibraryA");
    HANDLE ThreadReturn = CreateRemoteThread(hProc, NULL, 0, addrLoadLibrary, MyAlloc, 0, &dWord);
    if (ThreadReturn == NULL)
    {
        cerr << "[!]Fail to create Remote Thread" << endl;
        return false;
    }

    if ((hProc != NULL) && (MyAlloc != NULL) && (IsWriteOK != ERROR_INVALID_HANDLE) && (ThreadReturn != NULL))
    {
        cout << "[+]DLL Successfully Injected :)" << endl;
        return true;
    }

    return false;
}
#endif


void getProcessId(const char* window_title, DWORD& process_id)
{
    GetWindowThreadProcessId(FindWindowW(NULL, convertCharArrayToLPCWSTR(window_title)), &process_id);
}

// sus
DWORD GetPid(const char* window_title)
{
    DWORD procID = NULL;
    getProcessId(window_title, procID);


    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap && snap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(snap, &pe))
        {
            do
            {
                if (procID == pe.th32ProcessID)//!_stricmp(pe.th32ProcessID, 
                {
                    CloseHandle(snap);
                    return pe.th32ProcessID;
                }
            } while (Process32Next(snap, &pe));
        }
    }
    return 0;
}

gmx gmbool inj_try_injection(stringToDLL dllName, stringToDLL windowName)
{
    const char* dllpath = dllName;
    const char* targetProcess = windowName; // wnd name of targ.proc.
    long pid = 0;
   
    cout << "Looking for " << dllpath << " and " << targetProcess << endl;

    int failtimer = 0;
    while (!pid)
    {
        pid = GetPid(targetProcess);
        Sleep(10);
        failtimer++;
        if (failtimer == 10)
        {
            return gmint(2);
        }
    }
    cout << "Found Process!" << endl;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        cout << "Valid Handle" << endl;
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!WriteProcessMemory(hProc, loc, dllpath, strlen(dllpath) + 1, 0))
        {
            cout << "WriteProcessMemory err!" << endl;
            return gmfalse;
        }
        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
        CloseHandle(hThread);
        cout << "Closed Handle Thread" << endl;

    }
    cout << "Closed Handle Proc!" << endl;
    CloseHandle(hProc);
    cout << "done!" << endl;

    return gmtrue;
}









