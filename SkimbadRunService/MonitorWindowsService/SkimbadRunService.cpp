// MonitorWindowsService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include "Comms.h"
#include <TlHelp32.h>
#include <mmsystem.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <locale>
#include <codecvt>
#include <istream>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "resource.h"
#include "first_names.h"
#include "last_names.h"

#define MAX_DATA_SIZE 300


SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
std::vector<std::string> g_firstnames;
std::vector<std::string> g_lastnames;
std::vector<WCHAR*>		 g_uniptr;
std::vector<char*>		 g_ansiptr;

VOID WINAPI  ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI  ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
VOID process(VOID);


#define SERVICE_NAME  _T("SkimbadRunService")

int _tmain(int argc, TCHAR *argv[])
{
	OutputDebugString(_T("SkimbadRunService: Main: Entry"));

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		OutputDebugString(_T("SkimbadRunService: Main: StartServiceCtrlDispatcher returned error"));
		return GetLastError();
	}

	OutputDebugString(_T("SkimbadRunService: Main: Exit"));
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

	OutputDebugString(_T("SkimbadRunService: ServiceMain: Entry"));

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		OutputDebugString(_T("SkimbadRunService: ServiceMain: RegisterServiceCtrlHandler returned error"));
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
		OutputDebugString(_T("SkimbadRunService: ServiceMain: SetServiceStatus returned error"));
	}

	/*
	* Perform tasks neccesary to start the service here
	*/
	OutputDebugString(_T("SkimbadRunService: ServiceMain: Performing Service Start Operations"));

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		OutputDebugString(_T("SkimbadRunService: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("SkimbadRunService: ServiceMain: SetServiceStatus returned error"));
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
		OutputDebugString(_T("SkimbadRunService: ServiceMain: SetServiceStatus returned error"));
	}

	// Start the thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	OutputDebugString(_T("SkimbadRunService: ServiceMain: Waiting for Worker Thread to complete"));

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	OutputDebugString(_T("SkimbadRunService: ServiceMain: Worker Thread Stop Event signaled"));


	/*
	* Perform any cleanup tasks
	*/
	OutputDebugString(_T("SkimbadRunService: ServiceMain: Performing Cleanup Operations"));

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("SkimbadRunService: ServiceMain: SetServiceStatus returned error"));
	}

EXIT:
	OutputDebugString(_T("SkimbadRunService: ServiceMain: Exit"));

	return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	OutputDebugString(_T("SkimbadRunService: ServiceCtrlHandler: Entry"));

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		OutputDebugString(_T("SkimbadRunService: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

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
			OutputDebugString(_T("SkimbadRunService: ServiceCtrlHandler: SetServiceStatus returned error"));
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}

	OutputDebugString(_T("SkimbadRunService: ServiceCtrlHandler: Exit"));
}

//Check and launch watchdogs
void WatchDogCheck(MMStatusHeader * mmStatus)
{
	for (int i = 0; i < WD_MAX; i++)
	{
		//if no responce for 3 seconds launch
		if ((GetTickCount() - mmStatus->WD[i].LastUpdate) > 500)
		{
			ShellExecute(NULL, L"open", L"SkimbadWatchDog.exe", NULL, NULL, SW_SHOW);
		}
	}
}

DWORD FindExePid(WCHAR * exeName)
{
	HANDLE ProcessSnap;
	PROCESSENTRY32 pe32;
	//int nIndex;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	ProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(ProcessSnap, &pe32))
	{
		do
		{
			if (wcscmp(pe32.szExeFile, exeName) == 0)
			{
				return pe32.th32ProcessID;
			}
		} while (Process32Next(ProcessSnap, &pe32));
	}
	else
	{
		return 0;
	}
	return 0;
}

std::wstring s2ws(const std::string& str)
{
	std::wstring result;
	for (char x : str)
		result += x;
	return result;
}

std::string ws2s(const std::wstring& wstr)
{
	std::string result;
	for (char x : wstr)
		result += x;
	return result;
}

/* Generate card data in remote memory. A more complicated

*  method may be needed in the future.

*/

void GenerateCardDataW(HANDLE hProcess, WCHAR* lpBaseAddress, std::vector<std::string> &firstnames, std::vector<std::string> &lastnames)
{
	WCHAR* buf;
	int luhnsum = 0;
	int offset = 0;
	WCHAR* buf2;
	
	int numbers[18];	

	buf = (WCHAR *)malloc(sizeof(WCHAR)* MAX_DATA_SIZE);
	buf2 = (WCHAR *)malloc(sizeof(WCHAR)* MAX_DATA_SIZE);
	
	numbers[0] = rand() % 3 + 4;
	
	luhnsum += ((numbers[0] * 2) / 10) + ((numbers[0] * 2) % 10);
	
	for (int x = 1; x < 15; x++)
	{
		if (x % 2 == 0)
		{
			numbers[x] = rand() % 10;
			luhnsum += ((numbers[x] * 2) / 10) + ((numbers[x] * 2) % 10);
		}
		else
		{
			numbers[x] = rand() % 10;
			luhnsum += numbers[x];
		}
	}
	
	numbers[15] = (luhnsum * 9) % 10;

	time_t t = time(0);
	t += (rand() % (12 * 4)) * 60 * 60 * 24 * 31; 
	struct tm fut;
	localtime_s(&fut, &t);
	numbers[16] = (fut.tm_year % 100);
	numbers[17] = (fut.tm_mon);
	WriteProcessMemory(hProcess, lpBaseAddress, L"%B", sizeof(WCHAR)* 2, NULL);
	offset += 2;

	for (int x = 0; x < 16; x++)
	{
		swprintf(buf, sizeof(WCHAR)* MAX_DATA_SIZE, L"%d", numbers[x]);
		WriteProcessMemory(hProcess, lpBaseAddress + offset + x, buf, sizeof(WCHAR), NULL);
	}
	offset += 16;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, L"^", sizeof(WCHAR)* 1, NULL);
	offset++;
	std::wstring lname;
	lname = s2ws(lastnames[rand() % lastnames.size()]);
	WriteProcessMemory(hProcess, lpBaseAddress + offset, lname.c_str(), sizeof(WCHAR)* lname.size(), NULL);
	offset += lname.size();
	WriteProcessMemory(hProcess, lpBaseAddress + offset, L"/", sizeof(WCHAR)* 1, NULL);
	offset++;
	std::wstring fname;
	fname = s2ws(firstnames[rand() % firstnames.size()]);
	WriteProcessMemory(hProcess, lpBaseAddress + offset, fname.c_str(), sizeof(WCHAR)* fname.size(), NULL);
	offset += fname.size();
	WriteProcessMemory(hProcess, lpBaseAddress + offset, L"^", sizeof(WCHAR)* 1, NULL);
	offset++;
	swprintf(buf, sizeof(WCHAR)* MAX_DATA_SIZE, L"%02d%02d", numbers[16], numbers[17]);
	WriteProcessMemory(hProcess, lpBaseAddress + offset, buf, sizeof(WCHAR)* 4, NULL);
	offset += 4;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, L"1010000000000000005?;", sizeof(WCHAR)* 21, NULL);
	offset += 21;
	for (int x = 0; x < 16; x++)
	{
		swprintf(buf, sizeof(WCHAR)* MAX_DATA_SIZE, L"%d", numbers[x]);
		WriteProcessMemory(hProcess, lpBaseAddress + offset + x, buf, sizeof(WCHAR), NULL);
	}
	offset += 16;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "=", sizeof(char)* 1, NULL);
	offset += 1;
	swprintf(buf, sizeof(WCHAR)* MAX_DATA_SIZE, L"%02d%02d", numbers[16], numbers[17]);
	WriteProcessMemory(hProcess, lpBaseAddress + offset, buf, sizeof(WCHAR)* 4, NULL);
	offset += 4;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, L"10100000000005?", sizeof(WCHAR)* 21, NULL);
	ReadProcessMemory(hProcess, lpBaseAddress, buf2, sizeof(WCHAR)* MAX_DATA_SIZE, NULL);
	
	free(buf);
	free(buf2);
}

void GenerateCardDataA(HANDLE hProcess, char* lpBaseAddress, std::vector<std::string> &firstnames, std::vector<std::string> &lastnames)

{
	int luhnsum = 0;
	int offset = 0;	
	char* buf;		
	char* buf2;		
	int numbers[18];
	buf = (char *)malloc(sizeof(char)* MAX_DATA_SIZE);
	buf2 = (char *)malloc(sizeof(char)* MAX_DATA_SIZE);
	
	numbers[0] = rand() % 3 + 4; 
	
	luhnsum += ((numbers[0] * 2) / 10) + ((numbers[0] * 2) % 10);

	for (int x = 1; x < 15; x++)
	{
		if (x % 2 == 0)
		{
			numbers[x] = rand() % 10;
			luhnsum += ((numbers[x] * 2) / 10) + ((numbers[x] * 2) % 10);
		}
		else
		{
			numbers[x] = rand() % 10;
			luhnsum += numbers[x];
		}
	}
	numbers[15] = (luhnsum * 9) % 10;
	time_t t = time(0);
	t += (rand() % (12 * 4)) * 60 * 60 * 24 * 31;
	struct tm fut;
	localtime_s(&fut, &t);
	numbers[16] = (fut.tm_year % 100);
	numbers[17] = (fut.tm_mon);
	WriteProcessMemory(hProcess, lpBaseAddress, "%B", sizeof(char)* 2, NULL);
	offset += 2;
	for (int x = 0; x < 16; x++)
	{
		sprintf_s(buf, sizeof(char)* MAX_DATA_SIZE, "%d", numbers[x]);
		WriteProcessMemory(hProcess, lpBaseAddress + offset + x, buf, sizeof(char), NULL);
	}
	offset += 16;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "^", sizeof(char)* 1, NULL);
	offset++;
	std::string name;
	name = lastnames[rand() % lastnames.size()];
	WriteProcessMemory(hProcess, lpBaseAddress + offset, name.c_str(), sizeof(char)* name.size(), NULL);
	offset += name.size();
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "/", sizeof(char)* 1, NULL);
	offset++;
	name = firstnames[rand() % firstnames.size()];
	WriteProcessMemory(hProcess, lpBaseAddress + offset, name.c_str(), sizeof(char)* name.size(), NULL);
	offset += name.size();
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "^", sizeof(char)* 1, NULL);
	offset++;
	sprintf_s(buf, sizeof(char)* MAX_DATA_SIZE, "%02d%02d", numbers[16], numbers[17]);
	WriteProcessMemory(hProcess, lpBaseAddress + offset, buf, sizeof(char)* 4, NULL);
	offset += 4;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "1010000000000000005?;", sizeof(char)* 21, NULL);
	offset += 21;
	for (int x = 0; x < 16; x++)
	{
		sprintf_s(buf, sizeof(char)* MAX_DATA_SIZE, "%d", numbers[x]);
		WriteProcessMemory(hProcess, lpBaseAddress + offset + x, buf, sizeof(char), NULL);
	}
	offset += 16;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "=", sizeof(char)* 1, NULL);
	offset += 1;
	sprintf_s(buf, sizeof(char)* MAX_DATA_SIZE, "%02d%02d", numbers[16], numbers[17]);
	WriteProcessMemory(hProcess, lpBaseAddress + offset, buf, sizeof(char)* 4, NULL);
	offset += 4;
	WriteProcessMemory(hProcess, lpBaseAddress + offset, "10100000000005?", sizeof(char)* 21, NULL);
	ReadProcessMemory(hProcess, lpBaseAddress, buf2, sizeof(char)* MAX_DATA_SIZE, NULL);
	free(buf);
	free(buf2);
}

void LoadNames(char * filename, std::vector<std::string> &names)
{
	std::ifstream inf(filename);
	std::string line;
	while (std::getline(inf, line))
	{
		names.push_back(line);
	}
}

void LoadNamesRC(int textresource, std::vector<std::string> &names)
{
	
	HRSRC hRes = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(textresource), L"text");
	DWORD dwSize = SizeofResource(GetModuleHandle(NULL), hRes);
	HGLOBAL hGlob = LoadResource(GetModuleHandle(NULL), hRes);
	const BYTE* pData = reinterpret_cast<const BYTE*>(::LockResource(hGlob));
	
	std::string tmp((char *)pData);
	
	std::istringstream tstream(tmp);
	std::string line;
	while (std::getline(tstream, line))
	{
		names.push_back(line);
	}
}

VOID process(VOID)
{
	WCHAR szPName[MAX_PATH];
	DWORD dwNumCardsPerSec;
	DWORD dwPid;
	DWORD dwCardsPerTic;
	HANDLE hMapFile;
	MMStatusHeader *mmStatus;
	
	if (CreateSkimMap(hMapFile, mmStatus))
	{
		OutputDebugString(_T("SkimbadRunService: Memory Map Creation Failed."));
		return;
	}

	if (GetTickCount() - mmStatus->LastUpdate < 500)
	{
		return;
	}

	else
	{
		mmStatus->SettingsCommand = 0;
	}
	mmStatus->LastUpdate = GetTickCount();
	
	mmStatus->Loading = 0;
	HKEY hkey;
	
	LSTATUS stat;
	stat = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Skimbad", 0, KEY_READ, &hkey);
	
	if (stat == ERROR_SUCCESS)
	{
		DWORD dwSize = MAX_PATH;
		RegQueryValueEx(hkey, L"ProcessName", NULL, NULL, (BYTE *)szPName, &dwSize);
		dwSize = sizeof(DWORD);
		RegQueryValueEx(hkey, L"NumCards", NULL, NULL, (BYTE *)&dwNumCardsPerSec, &dwSize);
		if (dwNumCardsPerSec == 0)
		{
			dwNumCardsPerSec = 100;
		}
		dwCardsPerTic = dwNumCardsPerSec / 5;
	}
	else
	{
		return;
	}
	
	
	g_firstnames.resize(SIZE_FIRST_NAME_ARRAY);
	for (unsigned i = 0; i < SIZE_FIRST_NAME_ARRAY; ++i)
	{
		g_firstnames[i] = g_firstNames[i];

	}
	OutputDebugString(_T("SkimbadRunService: First names filled"));
	g_firstnames.resize(SIZE_LAST_NAME_ARRAY);
	for (unsigned i = 0; i < SIZE_LAST_NAME_ARRAY; ++i)
	{
		g_firstnames[i] = g_lastNames[i];
	}
	g_uniptr.resize(dwNumCardsPerSec);
	g_uniptr.resize(dwNumCardsPerSec);
	//initialize pointers
	for (int x = 0; x < dwNumCardsPerSec; x++)
	{
		g_uniptr[x]  = (NULL);
		g_ansiptr[x] =  (NULL);
	}
	srand((unsigned int)time(0)); // Seed it
	//Monitor target process and die when asked to
	while (mmStatus->SettingsCommand != WD_KILL_ALL)
	{
		dwPid = FindExePid(szPName);
		//update status time
		mmStatus->LastUpdate = GetTickCount();
		//Check watchdogs
		WatchDogCheck(mmStatus);
		//If process not running, wait for it
		while (dwPid == 0 && (mmStatus->SettingsCommand != WD_KILL_ALL))
		{
			dwPid = FindExePid(szPName);
			Sleep(50);
			//update status time
			mmStatus->LastUpdate = GetTickCount();
			WatchDogCheck(mmStatus);
		}
		//GetWindowThreadProcessId(hWnd, &pid);
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);
		DWORD dwStatus;
		GetExitCodeProcess(hProcess, &dwStatus);
		//Go until target process is no longer active or kill signal is given
		while (dwStatus == STILL_ACTIVE && (mmStatus->SettingsCommand != WD_KILL_ALL))
		{
			//update status time
			mmStatus->LastUpdate = GetTickCount();
			WatchDogCheck(mmStatus);
			for (int x = 0; x < dwCardsPerTic; x++)
			{
				g_uniptr[x] = (WCHAR*)VirtualAllocEx(hProcess, NULL, sizeof(WCHAR)* MAX_DATA_SIZE, MEM_COMMIT, PAGE_READWRITE);
				GenerateCardDataW(hProcess, g_uniptr[x], g_firstnames, g_lastnames);
				g_ansiptr[x] = (char*)VirtualAllocEx(hProcess, NULL, sizeof(char)* MAX_DATA_SIZE, MEM_COMMIT, PAGE_READWRITE);
				GenerateCardDataA(hProcess, g_ansiptr[x], g_firstnames, g_lastnames);
			}
			Sleep(200);
			//Free allocated memory
			for (int x = 0; x < dwCardsPerTic; x++)
			{
				VirtualFreeEx(hProcess, g_uniptr[x], 0, MEM_RELEASE);
				VirtualFreeEx(hProcess, g_ansiptr[x], 0, MEM_RELEASE);
			}
			GetExitCodeProcess(hProcess, &dwStatus);
		}
		if (dwStatus != STILL_ACTIVE)
		{
		}
		CloseHandle(hProcess);
	}
	CloseSkimMap(hMapFile, mmStatus);
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	OutputDebugString(_T("SkimbadRunService: ServiceWorkerThread: Entry"));
	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		/*
		* Perform main service function here
		*/
		//  Simulate some work by sleeping
		process();
		Sleep(3000);
	}
	OutputDebugString(_T("SkimbadRunService: ServiceWorkerThread: Exit"));
	return ERROR_SUCCESS;
}