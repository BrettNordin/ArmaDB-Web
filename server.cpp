#include "stdafx.h"
#include "mongoose.h"
#include "server.h">
namespace adbweb
{
	void server::sendHeaders(struct mg_connection *conn)
	{
		mg_send_header(conn, "Content-Type", "text/plain");
		mg_send_header(conn, "Access-Control-Allow-Origin", "*");
		mg_send_header(conn, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
		const char* ac_requestHeaders = mg_get_header(conn, "Access-Control-Request-Headers");
		if (ac_requestHeaders != NULL)
			mg_send_header(conn, "Access-Control-Allow-Headers", ac_requestHeaders);
	}


	int server::event_handler(struct mg_connection *conn, enum mg_event ev)
	{
		adbweb::server* server = static_cast<adbweb::server*>(conn->server_param);
		if (ev == MG_AUTH)
			return MG_TRUE;
		else if (ev == MG_REQUEST)
		{
			if (server->responsive)
			{
				char key[MAX_KEY_LEN];
				if ((mg_get_var(conn, "key", key, MAX_KEY_LEN) > 0))
				{
					
					if (server->authkey == key)
					{
						char content[MAX_CONTENT_LEN];
						if (mg_get_var(conn, "script", content, MAX_CONTENT_LEN) > 0)
						{
							adbweb::package* pkg = new package();
							pkg->fill(content);
							conn->connection_param = pkg;
							server->queue(pkg);
							return MG_MORE;
						}
						mg_send_status(conn, 400); 
						server->sendHeaders(conn);
						mg_printf_data(conn, "NSD");
						return MG_TRUE;
					}
					
					mg_send_status(conn, 403);
					server->sendHeaders(conn);
					mg_printf_data(conn, "AF2");
					return MG_TRUE;
				}
				mg_send_status(conn, 400); 
				server->sendHeaders(conn);
				mg_printf_data(conn, "AF");
				return MG_TRUE;
			}
			mg_send_status(conn, 503);
			server->sendHeaders(conn);
			mg_printf_data(conn, "SQFSTU");
			return MG_TRUE;
		}
		else if (ev == MG_POLL)
		{
			if (conn->connection_param != nullptr)
			{
				adbweb::package* pkg;
				pkg = static_cast<adbweb::package*>(conn->connection_param);
				if (pkg->check())
				{
					mg_send_status(conn, 200);
					server->sendHeaders(conn);
					mg_printf_data(conn, pkg->content().c_str());
					server->input_storage.unsafe_erase(pkg->id());
					mg_write(conn, "0\r\n\r\n", 5);
					delete pkg;
					return MG_TRUE;
				}
				return MG_MORE;
			}
		}
		return MG_FALSE;
	}

	void server::watchdog()
	{
		while (running)
		{
			if (responsive)
			{
				std::unique_lock<std::mutex> lk(mutex);
				responsive = false;
				wd_wait.wait_for(lk, std::chrono::milliseconds(RESPONSIVE_TIMEOUT), [this](){ return (responsive || !running); });
			}
			if (running)
				mg_poll_server(mongoose, SOCKET_TIMEOUT);
		}
	}

	void server::queue(adbweb::package* pkg)
	{
		output_queue.push(pkg);
	}

	void server::log(std::string log)
	{
		WinHttpClient ipsrv(L"http://api.ipify.org/");
		ipsrv.SendHttpRequest();
		wstring water = ipsrv.GetResponseContent();
		string pubip = std::string(water.begin(), water.end());
		WinHttpClient client(L"https://api.sphub.ca?SFSDIDL=123x8u12mn3cx8jasf0afsd&FPARM=MBACr-12f32ja");
		client.SetRequireValidSslCertificates(false);
		string data = log + "&ip="+ pubip;
		client.SetAdditionalDataToSend((BYTE *)data.c_str(), data.size());
		wchar_t szSize[50] = L"";
		swprintf_s(szSize, L"%d", data.size());
		wstring headers = L"Content-Length: ";
		headers += szSize;
		headers += L"\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		client.SetAdditionalRequestHeaders(headers);
		client.SendHttpRequest(L"POST");
	}

	void server::chatapi(std::string log)
	{
		WinHttpClient client(L"https://api.sphub.ca?SFSDIDL=123x8u12mn3cx8jasf0afsd&FPARM=MBACr-1312312fja");
		client.SetRequireValidSslCertificates(false);
		string data = "data=" + log + "&key=" + authkey;
		client.SetAdditionalDataToSend((BYTE*)data.c_str(), data.size());
		wchar_t szSize[50] = L"";
		swprintf_s(szSize, L"%d", data.size());
		wstring headers = L"Content-Length: ";
		headers += szSize;
		headers += L"\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		client.SetAdditionalRequestHeaders(headers);
		client.SendHttpRequest(L"POST");
	}

	server::server() : running(false), responsive(false), port("8080"), ssl_certificate_path("")
	{
	}

	server::~server()
	{
	}

	std::string server::request(std::string& input, size_t size)
	{
		if (!running)
		{
			return (std::string("3") + "Request failed: Server is not running.");
		}
		if (input.length() > 0)
		{
			package* pkg = input_storage.at(atoi(input.c_str()));
			char trunc = '1';
			std::string content;
			pkg->take();
			if (pkg->partial(content, size))
			{
				trunc = '0';
			}
			pkg->release();
			return (std::string("1") + trunc + content);
		}
		else
		{
			package* pkg;
			if (output_queue.try_pop(pkg))
			{
				char trunc = '0';
				std::string content;
				pkg->take();
				size_t pid = pkg->id();
				if (!pkg->partial(content, size))
				{
					trunc = '1';
				}
				input_storage[pid] = pkg;
				pkg->release();
				return (std::string("1") + trunc + std::to_string(pid) + '@' + content);
			}
			else
			{
				return "0";
			}
		}
	}

	std::string server::store(std::string& input)
	{
		if (!running)
		{
			return (std::string("3") + "Storage failed: Server is not running.");
		}
		size_t sep = input.find_first_of('@');
		size_t pid = atoi(input.substr(0, sep).c_str());
		try {
			package* pkg = input_storage.at(pid);
			if (pkg != nullptr)
			{
				pkg->take();
				pkg->fill(input.substr(sep + 1));
				pkg->close();
				pkg->release();
				return "0";
			} else
				return (std::string("3") + "Storage failed: Package found but deleted.");
		}
		catch (std::out_of_range ret)
		{
			return (std::string("3") + "Storage failed: No relative package found.");
		};
	}

	void server::auth(std::string key)
	{
		authkey = key;

	}

	void server::responsiveCheck() {
		std::unique_lock<std::mutex> lk(mutex);
		responsive = true;
		wd_wait.notify_one();
	}

	std::string server::start()
	{

		WinHttpClient client(L"https://api.sphub.ca?SFSDIDL=123x8u12mn3cx8jasf0afsd&FPARM=MBACr-534534fgdsg43gs4xdh5");
		client.SetRequireValidSslCertificates(false);
		string data = "key="+ authkey;
		client.SetAdditionalDataToSend((BYTE *)data.c_str(), data.size());
		wchar_t szSize[50] = L"";
		swprintf_s(szSize, L"%d", data.size());
		wstring headers = L"Content-Length: ";
		headers += szSize;
		headers += L"\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		client.SetAdditionalRequestHeaders(headers);
		client.SendHttpRequest(L"POST");
		wstring water = client.GetResponseContent();
		string blocked = std::string(water.begin(), water.end());
		if (blocked == "0")
		{
			running = true;
			mongoose = mg_create_server(this, event_handler);
			mg_set_option(mongoose, "access_control_list", "-0.0.0.0,+104.24.7.29,+104.205.11.37,+74.208.81.101,+84.28.84.215,+77.72.0.174,+86.12.57.144,+74.208.27.26,+74.208.27.48,+127.0.0.1,+192.168.1.100");
			mg_set_option(mongoose, "listening_port", port.c_str());
			wd_thread = new std::thread(&adbweb::server::watchdog, this);
			return ("1");
		}
		else
		{
			return ("0");
		}
		
	}

	bool server::stop()
	{
		if (!running)
			return false;
		running = false;
		mg_destroy_server(&mongoose);
		wd_wait.notify_one();
		wd_thread->join();
		delete wd_thread;
		adbweb::package* pkg;
		while (output_queue.try_pop(pkg))
		{
			delete pkg;
		}
		for (concurrency::concurrent_unordered_map<size_t, adbweb::package*>::iterator it = input_storage.begin(); it != input_storage.end(); ++it)
		{
			delete it->second;
		}
		input_storage.clear();
		authkey.clear();
		return true;
	}

	std::string server::bancheck(std::string uid)
	{
		WinHttpClient client(L"https://api.sphub.ca?SFSDIDL=123x8u12mn3cx8jasf0afsd&FPARM=MBACr-729e2dBREQ");
		client.SetRequireValidSslCertificates(false);
		string data = "uid=" + uid + "&key=" + authkey;
		client.SetAdditionalDataToSend((BYTE*)data.c_str(), data.size());
		wchar_t szSize[50] = L"";
		swprintf_s(szSize, L"%d", data.size());
		wstring headers = L"Content-Length: ";
		headers += szSize;
		headers += L"\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		client.SetAdditionalRequestHeaders(headers);
		client.SendHttpRequest(L"POST");
		wstring water = client.GetResponseContent();
		string blocked = std::string(water.begin(), water.end());
		return (blocked);

	}
}
