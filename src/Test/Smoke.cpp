#include "pch.h"
#include "../BackEnd/Net/Client.h"
#include "TestServer.h"

namespace Test {
	namespace End2End {
		class Smoke : public ::testing::Test {
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

		TEST_F(Smoke, Student_OnNotExisting_NotFound) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("name=George"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::NotFound);
			}).wait();
		}

		TEST_F(Smoke, Student_OnRequestingNotExistingEntity_BadRequest) {
			auto req = uri_builder(local).append_path(U("students")).append_query(U("phone_number=09011234567"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::BadRequest);
			}).wait();
		}

		TEST_F(Smoke, Class_OnDictatingId_MethodNotAllowed) {
			auto req = uri_builder(local).append_path(U("class")).append_query(U("class_id=4"))
				.append_query(U("course_ref=1")).append_query(U("student_ref=1")).append_query(U("score_ref=null"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::MethodNotAllowed); // MethodNotAllowed for dictating id of new entity
			}).wait();
		}

		TEST_F(Smoke, Course_OnDoubleCreating_Conflict) {
			auto req = uri_builder(local).append_path(U("course")).append_query(U("name=Physics"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Conflict); // Conflict for already existing data
			}).wait();
		}

		TEST_F(Smoke, Score_OnUpdatingNonExisting_NotFound) {
			auto req = uri_builder(local).append_path(U("score")).append_query(U("class_ref=4"));
			auto body = web::json::value(U("final_term=20"));
			cli.make_request(methods::PUT, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::NotFound); // NotFound on put requst using a non-existing data
			}).wait();
		}

		TEST_F(Smoke, Student_OnRequestingHead_OK) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("name=Ali"));
			cli.make_request(methods::HEAD, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK); // OK for HEAD if the resource exist and valid
			}).wait();
		}

		TEST_F(Smoke, Class_OnRequestingOptions_OK) {
			auto req = uri_builder(local).append_path(U("class")).append_query(U("student_ref=1"));
			cli.make_request(methods::OPTIONS, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK); // OK for OPTIONS 
				response.extract_json().then([](web::json::value result) { // Allowd http verbs on resource
					web::json::value expected = web::json::value::parse(L"{\"Allow\":[\"GET\",\"POST\",\"PUT\",\"PATCH\",\"DEL\",\"OPTIONS\",\"HEAD\"]}");
					EXPECT_EQ(result.serialize(), expected.serialize());
				}).wait();
			}).wait();
		}

		TEST_F(Smoke, Course_OnDeletingInterconnected_Forbidden) {
			auto req = uri_builder(local).append_path(U("course")).append_query(U("name=Math"));
			cli.make_request(methods::DEL, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Forbidden); // NO DELETE CASCADE !
			}).wait();
		}

		TEST_F(Smoke, None_OnBareFullyNakedGetRequest_BadRequest) {
			auto req = uri_builder(local);
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::BadRequest);
			}).wait();
		}

		TEST_F(Smoke, None_OnIllegalRequest_BadRequest) {
			auto req = uri_builder(local).append_path(U("favicon.ico"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::BadRequest);
			}).wait();
		}

		// TODO: OK for TRACE: create a loopback for the request message

		// TODO: stress_test to produce service_not_available

		// TODO: authentication (base64 encoded not plain text)

		// TODO: using cookies for authentication

		// TODO: authorization

		// TODO: caching (on front end??)
	}
}