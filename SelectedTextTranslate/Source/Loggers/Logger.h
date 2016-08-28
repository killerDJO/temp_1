#pragma once
#include "PrecompiledHeaders\stdafx.h"
#include <chrono>
#include <iomanip>

class Logger 
{
public:
	static void Log(wstring record);

private:
	static wstring GetCurrentDateTime();
	static wstring GetCurrentDate();
	static wstring GetLocalComputerName();
	static wstring GetLogFileName();
};