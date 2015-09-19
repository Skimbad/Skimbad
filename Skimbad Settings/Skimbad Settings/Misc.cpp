
#include "stdafx.h"
#include "Misc.h"
#include <winsvc.h>

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
		LPTSTR errorstring = NULL;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorstring, 0, NULL);
		MessageBox(NULL, errorstring, errorstring, MB_OK | MB_ICONERROR);
		free(errorstring);
		return 0;
	}

	return 0;
}

void startServer(SC_HANDLE manager)
{
	//run service
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);
	CString FullPath;
	FullPath += " ";
	FullPath += pwd;
	FullPath += "\\SkimbadRunService.exe";
	LPCTSTR szPathToBinary = (LPCTSTR)(FullPath);
	//check that file is exist in installed folder
	//MessageBox(HWND_DESKTOP, szPathToBinary, szPathToBinary, MB_OK);
	SC_HANDLE service = ::CreateService(
		manager,
		L"SkimbadRunService",
		L"SkimbadRunService",
		GENERIC_READ | GENERIC_EXECUTE,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_IGNORE,
		szPathToBinary,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
		);
}

void restartService()
{
	//TODO: finish me
	SC_HANDLE schSCManager = ::OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (schSCManager)
	{
		SC_HANDLE SHandle = OpenService(schSCManager, L"SkimbadRunService", SC_MANAGER_ALL_ACCESS);
		startServer(schSCManager);
		if (SHandle == NULL)
		{
			//Service not found. try to start it.
			startServer(schSCManager);
		}
		else
		{
			TCHAR pwd[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, pwd);
			MessageBox(HWND_DESKTOP, pwd, pwd, MB_OK);
			//service found, but need to be started.
			SERVICE_STATUS Status;
			QueryServiceStatus(SHandle, &Status);
			switch (Status.dwCurrentState)
			{
			case SERVICE_RUNNING:
				//service already running
				return;
			case SERVICE_STOPPED:
			default:
				if (StartService(SHandle, 0, NULL) ==0)
				{
				}
				return;
			}
		}
	}
}

void stopService()
{
	//TODO: finish me
	SC_HANDLE schSCManager = ::OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (schSCManager)
	{

		SC_HANDLE SHandle = OpenService(schSCManager, L"SkimbadRunService", SC_MANAGER_ALL_ACCESS);
		SERVICE_STATUS Status;
		QueryServiceStatus(SHandle, &Status);
		switch (Status.dwCurrentState)
		{
		case SERVICE_RUNNING:
			//service already running
			return;
		case SERVICE_STOPPED:
		default:
			StartService(SHandle, 0, NULL);
			return;
		}
	}
}

DWORD StartSkimbadRun()
{
	restartService();
	return 0;
}

DWORD StopSkimbadRun()
{
	/*
	HANDLE hProcess;
	DWORD dwPID = FindExePid(L"SkimbadRun.exe");

	if (dwPID != 0)
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwPID);
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	else
	{
		return -1;
	}
	*/

	HANDLE hMapFile;
	MMStatusHeader *mmStatus;
	//StopService();
	if (CreateSkimMap(hMapFile, mmStatus))
	{
		MessageBox(NULL, L"Memory Map Creation Failed.", L"Skimbad", MB_OK | MB_ICONERROR);
		return 0;
	}

	mmStatus->SettingsCommand = WD_KILL_ALL;

	CloseSkimMap(hMapFile, mmStatus);

	return 0;
}

DWORD RestartSkimbadRun()
{
	StopSkimbadRun();
	StartSkimbadRun();
	return 0;
}


int CALLBACK PidCompareA(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
	CString    strItem1 = pListCtrl->GetItemText(static_cast<int>(lParam1), 0);
	CString    strItem2 = pListCtrl->GetItemText(static_cast<int>(lParam2), 0);

	int x1 = _tstoi(strItem1.GetBuffer());
	int x2 = _tstoi(strItem2.GetBuffer());
	int result = 0;
	if ((x1 - x2) < 0)
		result = -1;
	else if ((x1 - x2) == 0)
		result = 0;
	else
		result = 1;

	return result;
}

int CALLBACK PNameCompareA(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
	CString    strItem1 = pListCtrl->GetItemText(static_cast<int>(lParam1), 1);
	CString    strItem2 = pListCtrl->GetItemText(static_cast<int>(lParam2), 1);

	return wcscmp(strItem1.MakeUpper().GetBuffer(), strItem2.MakeUpper().GetBuffer());
}


int CALLBACK PidCompareD(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
	CString    strItem1 = pListCtrl->GetItemText(static_cast<int>(lParam1), 0);
	CString    strItem2 = pListCtrl->GetItemText(static_cast<int>(lParam2), 0);

	int x1 = _tstoi(strItem1.GetBuffer());
	int x2 = _tstoi(strItem2.GetBuffer());
	int result = 0;
	if ((x1 - x2) < 0)
		result = -1;
	else if ((x1 - x2) == 0)
		result = 0;
	else
		result = 1;

	return -result;
}

int CALLBACK PNameCompareD(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
	CString    strItem1 = pListCtrl->GetItemText(static_cast<int>(lParam1), 1);
	CString    strItem2 = pListCtrl->GetItemText(static_cast<int>(lParam2), 1);

	return -wcscmp(strItem1.MakeUpper().GetBuffer(), strItem2.MakeUpper().GetBuffer());
}