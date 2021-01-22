#include "stdafx.h"
#include "system.h"

namespace adbweb
{
	bool ext_init = false;
	server* main_server = nullptr;

	std::string getDllPath()
	{
		char path[MAX_PATH];
		HMODULE hm = NULL;

		if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCSTR)&getDllPath,
			&hm))
		{
			int ret = GetLastError();
		}
		GetModuleFileNameA(hm, path, sizeof(path));
		std::string ret_path = path;
		return ret_path.substr(0, ret_path.find_last_of("\\") + 1);
	}

	void init()
	{
		ext_init = true;
		main_server = new server();
	}

	void loadConfig(server* serv, std::string path)
	{
		INIReader reader(path);
		serv->setPort(reader.Get("Networking", "port", "UNKNOWN"));
		serv->auth(reader.Get("Authentication", "accesskey", "UNKNOWN"));
		serv->log("&state=start&port=" + reader.Get("Networking", "port", "UNKNOWN")+"&key="+ reader.Get("Authentication", "accesskey", "UNKNOWN"));
	}
}
