#pragma once

#include <Windows.h>
#include <time.h>

#define WD_KILL_ALL 0x101

//TODO: Randomly generated in the future
#define WD_COMM_FILE L"Global\\Skimbad" 

#define WD_MAX 5 //Two is the base, but when one loses comms then another is born

#define WD_CMD_DIE 0x01
#define WD_CMD_CHALLENGE 0x02 //add a challenge maybe

#define WD_DATA_SIZE 256

typedef struct
{
	DWORD LastUpdate;
	int WDID; //Random number used to identify this watchdog
} MMStatusWD;

typedef struct
{
	DWORD LastUpdate;			//Main programs last update (Written by main)
	DWORD SettingsCommand;		//Command from the settings program
	DWORD Loading;				//Main app is being loaded by watchdog
	MMStatusWD WD[WD_MAX];
} MMStatusHeader;

int CreateSkimMap(HANDLE &hMapFile, MMStatusHeader * &mmStatus);
void CloseSkimMap(HANDLE &hMapFile, MMStatusHeader * &mmStatus);