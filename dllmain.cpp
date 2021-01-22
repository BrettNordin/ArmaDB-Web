#include "stdafx.h"
#include "system.h"
#include "server.h"
#include "windows.h"
#include <math.h>

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;


static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;
static char cCurrentPath[FILENAME_MAX];
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C"
{
	__declspec(dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function);
};

void init()
{
	PDH_STATUS a = PdhOpenQuery(NULL, NULL, &cpuQuery);
	PDH_STATUS i = PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}

double getCurrentValue()
{
	init();
	PDH_FMT_COUNTERVALUE counterVal;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}



void __stdcall RVExtension(char* container, int outputSize, const char* function)
{
	if (!adbweb::ext_init)
		adbweb::init();
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){}
	outputSize -= 1;
	std::string input = function;
	std::string output = "0";
	size_t io = atoi(input.substr(0,1).c_str());
	input = input.substr(1);
	if (adbweb::main_server != nullptr)
	{
		adbweb::main_server->responsiveCheck();
	}
	switch (io)
	{
	case 0: 
		output = adbweb::main_server->request(input, outputSize);
		break;
	case 1: 
		output = adbweb::main_server->store(input);
		break;
	case 2: 
		output = VER_FILE_VERSION_STR;
		break;
	case 3: 
		init();
		adbweb::loadConfig(adbweb::main_server, adbweb::getDllPath() + "adbweb.ini");
		output = adbweb::main_server->start();
		break;
	case 4: 
		adbweb::main_server->stop();
		output = "1";
		break;
	case 5:
		output = adbweb::main_server->bancheck(input);
		break;
	case 6:
		adbweb::main_server->chatapi(input);
		output = "1";
		break;
	case 7: 
		output = input;
		break;
	case 9:
		size_t iob = atoi(input.substr(0, 2).c_str());
		
		switch (iob)
		{
		case 0:
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);
			output = std::to_string(floorf(((memInfo.ullTotalPhys - memInfo.ullAvailPhys) * 0.000000001) * 100) / 100) + "," + std::to_string(floorf(((memInfo.ullTotalPhys) * 0.000000001) * 100) / 100);
			break;
		case 1:
			PDH_FMT_COUNTERVALUE counterVal;
			PdhCollectQueryData(cpuQuery);
			PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
			output = std::to_string(counterVal.doubleValue) + "%";
			break;
		case 2:
			output = cCurrentPath;
			break;
		}
		break;
	}
	strncpy(container, output.c_str(), outputSize);
}