#pragma once


#define SOCKET_TIMEOUT 250 
#define RESPONSIVE_TIMEOUT 30000 

#define MAX_KEY_LEN 120 
#define MAX_CONTENT_LEN 262144 

#include "version.h"

#define WIN32_LEAN_AND_MEAN		

#include <windows.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include "INIReader.h"
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <tchar.h>
#include "TCHAR.h"
#include <PdhMsg.h>
#include "pdh.h"
#include <WebEx/WinHttpClient.h>
#include <direct.h>

#define GetCurrentDir _getcwd
