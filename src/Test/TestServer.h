#pragma once
#include "../BackEnd/Net/Server.h"

namespace Test {
	class TestServer : public BackEnd::Net::Server
	{
	public:
		TestServer() = delete;
		TestServer(const web::uri& uri, bool in_memory_db);
		virtual ~TestServer();

	private:
		void WriteSampleData();
	};
}