

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include "Comms.h"
int CreateSkimMap(HANDLE &hMapFile, MMStatusHeader * &mmStatus)
{

	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		WD_DATA_SIZE,                // maximum object size (low-order DWORD)
		WD_COMM_FILE);           // name of mapping object

	if (hMapFile == NULL)
	{
		//MessageBox(NULL, L"Memory Map Creation Failed.1", L"Skimbad", MB_OK | MB_ICONERROR);
		//failed
		return 1;
	}

	mmStatus = (MMStatusHeader *)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		WD_DATA_SIZE);

	if (mmStatus == NULL)
	{
		//MessageBox(NULL, L"Memory Map Creation Failed.2", L"Skimbad", MB_OK | MB_ICONERROR);
		//failed
		return 1;
	}

	return 0;
}

void CloseSkimMap(HANDLE &hMapFile, MMStatusHeader * &mmStatus)
{
	UnmapViewOfFile(mmStatus);
	CloseHandle(hMapFile);
}