#include "pch.h"
#include "../BackEnd/Net/Client.h"
#include "TestServer.h"

namespace Test {
	class CodeOnDemand : public ::testing::Test {
	protected:
		web::uri local = uri_builder(U("http://127.0.0.1:34568")).to_uri();
		TestServer srv = TestServer(local, true);
		BackEnd::Net::Client cli = BackEnd::Net::Client(local);

		virtual void SetUp() {
			srv.open();
		}

		virtual void TearDown() {
			srv.close();
		}
	};

	// TODO implement CoD to make server Restful
}