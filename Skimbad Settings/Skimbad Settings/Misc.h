#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include "Comms.h"

DWORD FindExePid(WCHAR * exeName);
DWORD StartSkimbadRun();
DWORD StopSkimbadRun();
DWORD RestartSkimbadRun();
int CALLBACK PidCompareA(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK PNameCompareA(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK PidCompareD(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK PNameCompareD(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);