#include "pch.h"
#include "../BackEnd/Net/Client.h"
#include "TestServer.h"

namespace Test {
	namespace End2End {
		class Extra : public ::testing::Test {
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

		TEST_F(Extra, Student_OnFragmentRequesting_Success) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("name=Ali")).append(U("#student_id"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
				response.extract_json().then([](web::json::value result) {
					web::json::value expected = web::json::value::parse(L"{\"student_id\": 1}");
					EXPECT_EQ(result.serialize(), expected.serialize());
				}).wait();
			}).wait();
		}
	}
}