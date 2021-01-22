#pragma once

#include "package.h"

namespace adbweb
{
	class server
	{
	private:
		std::mutex mutex;
		std::condition_variable wd_wait;
		std::thread* wd_thread;
		std::string port;
		std::string ssl_certificate_path;
		bool running;
		bool responsive;
		struct mg_server *mongoose;
		concurrency::concurrent_queue<adbweb::package*> output_queue;
		concurrency::concurrent_unordered_map<size_t,adbweb::package*> input_storage;
		std::string authkey;
		std::string burl;
		void server::sendHeaders(struct mg_connection *conn);
		static int event_handler(struct mg_connection *conn, enum mg_event ev);
		void watchdog();
		void queue(adbweb::package* pkg);
	public:
		server();
		~server();
		std::string request(std::string& input, size_t size);
		std::string store(std::string& input);
		void auth(std::string key);
		void log(std::string log);
		void responsiveCheck();
		std::string start();
		bool stop();
		std::string bancheck(std::string uid);
		void chatapi(std::string chat);
		void setPort(std::string sPort) { this->port = sPort; };
	};
}
