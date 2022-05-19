// A free antivirus engine for computers.
// Started in a French classroom on 19/05/2022 08:37 (sorry meneer dronkers)
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <winsvc.h>
#include <LMCons.h>
#include "ServiceHeader.h"

BOOL IsElevated() {
    BOOL fRet = false;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess( ),0x0008,&hToken ) ) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if( hToken ) {
        CloseHandle( hToken );
    }
    return fRet;
}

using namespace std;

int main(int argc, char** argv) {
    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPWSTR)TEXT(""), &ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(serviceTable))
        return 0;
    else if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
        if (!IsElevated()) {
            cout << "Requesting admin priveleges..." << endl;
            // get program path
            LPWSTR path = new WCHAR[MAX_PATH];
            GetModuleFileName(NULL, path, MAX_PATH);
            wchar_t szPath[MAX_PATH];
            if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
            {
                // Launch itself as administrator. 
                SHELLEXECUTEINFO sei = { sizeof(sei) };
                sei.lpVerb = L"runas";
                sei.lpFile = szPath;
                sei.hwnd = NULL;
                sei.nShow = SW_NORMAL;

                if (!ShellExecuteEx(&sei))
                {
                    DWORD dwError = GetLastError();
                    if (dwError == ERROR_CANCELLED)
                    {
                        // The user refused the elevation. 
                        // Do nothing ... 
                        cout << "Cancelled." << endl;
                        exit(0);
                    }
                }
                else
                {
                    cout << "Success." << endl;
                    exit(0);
                }
            }
        }
        else {
            cout << "Admin priveleges granted." << endl;
            cout << "Starting nexusANTI..." << endl;
            cout << "Loading..." << endl;
            // print current username
            // remove console window
            HWND hwnd = GetConsoleWindow();
            ShowWindow(hwnd, SW_HIDE);
            // shamelessly stealing process hacker code
            ULONG win32Result = 0;
            SC_HANDLE scManagerHandle = nullptr;
            SC_HANDLE serviceHandle;
            if (!(scManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
                cout << "Failed to open service manager." << endl;
                exit(0);
            }
            // create service as system
            wchar_t szPath[MAX_PATH];
            GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
            wcout << szPath << endl;
            if (!(serviceHandle = CreateService(
                scManagerHandle,
                L"nexusANTI Temporary System Service",
                L"nexusANTI Temporary System Service",
                SERVICE_ALL_ACCESS,
                SERVICE_WIN32_OWN_PROCESS,
                SERVICE_DEMAND_START,
                SERVICE_ERROR_IGNORE,
                szPath, NULL, NULL, NULL, L"LocalSystem", L""))) {
                cout << "Service creation failure." << endl;
                exit(0);
            }
            StartService(serviceHandle, 0, NULL);
            DeleteService(serviceHandle);
            cout << "Success." << endl;
        }
        return 0;
    }
    else
        return -2; // Other error.
}