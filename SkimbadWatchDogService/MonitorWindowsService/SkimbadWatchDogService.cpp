// MonitorWindowsService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include "Comms.h"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI  ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI  ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
INT WINAPI	 process(VOID);

#define SERVICE_NAME  _T("SkimbadWatchDogService")

int _tmain(int argc, TCHAR *argv[])
{
	OutputDebugString(_T("SkimbadWatchDogService: Main: Entry"));

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		OutputDebugString(_T("SkimbadWatchDogService: Main: StartServiceCtrlDispatcher returned error"));
		return GetLastError();
	}

	OutputDebugString(_T("SkimbadWatchDogService: Main: Exit"));
	return 0;
}


INT WINAPI process(VOID)
{
	srand((unsigned int)time(NULL));
	int WDID = rand(); //Random ID for verification
	int WDSpot = -1; //Position in array

	HANDLE hMapFile;
	MMStatusHeader *mmStatus;

	if (CreateSkimMap(hMapFile, mmStatus))
	{
		OutputDebugString(_T("SkimBard WatchDog: Memory Map Creation Failed."));
		return 0;
	}

	//Choose which watchdog spot to run in
	for (int x = 0; (x < WD_MAX) && (WDSpot == -1); x++)
	{
		//1/2 seconds since last update take this spot
		if (GetTickCount() - mmStatus->WD[x].LastUpdate > 500)
		{
			WDSpot = x;
			mmStatus->WD[x].WDID = WDID;
			mmStatus->WD[x].LastUpdate = GetTickCount();
		}
	}

	//all slots taken
	if (WDSpot == -1)
	{
		return 0;
	}

	int done = 0;
	while ((mmStatus->SettingsCommand != WD_KILL_ALL) && !done)
	{
		//monitor status of main application

		//Not updated for 1/2 seconds
		if (GetTickCount() - mmStatus->LastUpdate > 500)
		{
			//if main is not being launched
			if (!mmStatus->Loading)
			{
				mmStatus->Loading = 1;
				//TODO: need relaunch service
				//launch it
				ShellExecute(NULL, L"open", L"SkimbadRun.exe", NULL, NULL, SW_SHOW);

			}

		}

		Sleep(100);

		//check that this WD is the right one for its slot
		if (mmStatus->WD[WDSpot].WDID != WDID)
		{
			//Another watchdog took my spot
			done = 1;
		}
		else
		{
			//update time
			mmStatus->WD[WDSpot].LastUpdate = GetTickCount();
		}
	}

	CloseSkimMap(hMapFile, mmStatus);
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

	OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: Entry"));

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: RegisterServiceCtrlHandler returned error"));
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof (g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: SetServiceStatus returned error"));
	}

	/*
	* Perform tasks neccesary to start the service here
	*/
	OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: Performing Service Start Operations"));

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: SetServiceStatus returned error"));
		}
		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: SetServiceStatus returned error"));
	}

	// Start the thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: Waiting for Worker Thread to complete"));

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: Worker Thread Stop Event signaled"));


	/*
	* Perform any cleanup tasks
	*/
	OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: Performing Cleanup Operations"));

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: SetServiceStatus returned error"));
	}

EXIT:
	OutputDebugString(_T("SkimbadWatchDogService: ServiceMain: Exit"));

	return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	OutputDebugString(_T("SkimbadWatchDogService: ServiceCtrlHandler: Entry"));

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		OutputDebugString(_T("SkimbadWatchDogService: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		* Perform tasks neccesary to stop the service here
		*/

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("SkimbadWatchDogService: ServiceCtrlHandler: SetServiceStatus returned error"));
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}

	OutputDebugString(_T("SkimbadWatchDogService: ServiceCtrlHandler: Exit"));
}


DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	OutputDebugString(_T("SkimbadWatchDogService: ServiceWorkerThread: Entry"));

	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		/*
		* Perform main service function here
		*/
		process();
		//  Simulate some work by sleeping
		Sleep(3000);
	}

	OutputDebugString(_T("SkimbadWatchDogService: ServiceWorkerThread: Exit"));

	return ERROR_SUCCESS;
}