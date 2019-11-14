#include "pch.h"
#include "CppUnitTest.h"
#include "../BackEnd/Net/Client.h"
#include "TestServer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test
{		
	TEST_CLASS(BizScenes)
	{
	public:
		TEST_METHOD(Client_OnCreatingSampleStudent_ShouldBeSuccessful) {
			auto path = uri_builder(local2).append_path(U("student")).append_query(U("Name=Mohsen"))
				.append_query(U("Family=Shiri")).append_query(U("Birth=13710101"));
			auto req = cli.make_request(methods::POST, path.to_string(), 0).then([](http_response response)
			{
				Assert::AreSame<int>(response.status_code(), status_codes::OK);
				
			}).wait();
		}

		TEST_METHOD(Client_OnSearchingStudent_ShouldBeSuccessful) {
			auto path = uri_builder(local2).append_path(U("student")).append_query(U("Family=Naseri"));
			auto req = cli.make_request(methods::GET, path.to_string(), 0).then([](http_response response)
			{
				Assert::AreSame<int>(response.status_code(), status_codes::OK);
				// TODO مقادیر مورد نظر با مقادیر واقعی مطابقت داده شود
			}).wait();
		}

		TEST_METHOD(Client_OnUpdatingStudent_ShouldBeSuccessful) {
			auto path = uri_builder(local2).append_path(U("student")).append_query(U("student_id=1"));
			auto body = web::json::value("Name=Alireza");
			auto req = cli.make_request(methods::PUT, path.to_string(), body).then([](http_response response)
			{
				Assert::AreSame<int>(response.status_code(), status_codes::OK);
			}).wait();
		}

		TEST_METHOD(Client_OnDeletingStudent_ShouldBeSuccesful) {
			auto path = uri_builder(local2).append_path(U("student")).append_query(U("student_id=1"));
			auto req = cli.make_request(methods::DEL, path.to_string(), 0).then([](http_response response)
			{
				Assert::AreSame<int>(response.status_code(), status_codes::OK);
			}).wait();
		}

		TEST_METHOD(Client_OnBadRequesting_ShouldFail) {
			auto path = uri_builder(local2).append_path(U("studentS")).append_query(U("Family=Naseri"));
			auto req = cli.make_request(methods::GET, path.to_string(), 0).then([](http_response response)
			{
				Assert::AreSame<int>(response.status_code(), status_codes::BadRequest);
			}).wait();
		}

	private:
		web::uri local1 = uri_builder(U("http://localhost:1234")).to_uri();
		Test::Biz::TestServer srv = Test::Biz::TestServer(local1, true);
		
		web::uri local2 = uri_builder(U("http://localhost:5678")).to_uri();
		BackEnd::Net::Client cli = BackEnd::Net::Client(local2);
	};
}