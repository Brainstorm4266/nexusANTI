// Main function to be executed as entire service code.
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <winsvc.h>
#include <LMCons.h>
#include "ServiceHeader.h"

SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
HANDLE g_StopEvent;
DWORD g_CurrentState = 0;
bool g_SystemShutdown = false;

void ReportStatus(DWORD state)
{
    g_CurrentState = state;
    SERVICE_STATUS serviceStatus = {
        SERVICE_WIN32_OWN_PROCESS,
        g_CurrentState,
        state == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
        NO_ERROR,
        0,
        0,
        0,
    };
    SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}

// Handler for service control events.
DWORD WINAPI HandlerEx(DWORD control, DWORD eventType, void* eventData, void* context)
{
    switch (control)
    {
        // Entrie system is shutting down.
    case SERVICE_CONTROL_SHUTDOWN:
        g_SystemShutdown = true;
        // continue...
    // Service is being stopped.
    case SERVICE_CONTROL_STOP:
        ReportStatus(SERVICE_STOP_PENDING);
        SetEvent(g_StopEvent);
        break;
        // Ignoring all other events, but we must always report service status.
    default:
        ReportStatus(g_CurrentState);
        break;
    }
    return NO_ERROR;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    // Must be called at start.
    g_ServiceStatusHandle = RegisterServiceCtrlHandlerEx(TEXT("nexusANTI Temporary System Service"), &HandlerEx, NULL);

    // Startup code.
    ReportStatus(SERVICE_START_PENDING);
    g_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    /* Here initialize service...
    Load configuration, acquire resources etc. */
    ReportStatus(SERVICE_RUNNING);

    /* Main service code
    Loop, do some work, block if nothing to do,
    wait or poll for g_StopEvent... */
    while (WaitForSingleObject(g_StopEvent, 3000) != WAIT_OBJECT_0)
    {
        // This sample service does "BEEP!" every 3 seconds.
        //Beep(1000, 100);
    }

    ReportStatus(SERVICE_STOP_PENDING);
    /* Here finalize service...
    Save all unsaved data etc., but do it quickly.
    If g_SystemShutdown, you can skip freeing memory etc. */
    CloseHandle(g_StopEvent);
    ReportStatus(SERVICE_STOPPED);
}

void ReportProgressStatus(DWORD state, DWORD checkPoint, DWORD waitHint)
{
    g_CurrentState = state;
    SERVICE_STATUS serviceStatus = {
        SERVICE_WIN32_OWN_PROCESS,
        g_CurrentState,
        state == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
        NO_ERROR,
        0,
        checkPoint,
        waitHint,
    };
    SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}

void ReportErrorStatus(DWORD errorCode)
{
    g_CurrentState = SERVICE_STOPPED;
    SERVICE_STATUS serviceStatus = {
        SERVICE_WIN32_OWN_PROCESS,
        g_CurrentState,
        0,
        ERROR_SERVICE_SPECIFIC_ERROR,
        errorCode,
        0,
        0,
    };
    SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}