#include "pch.h"
#include "../BackEnd/Net/Client.h"
#include "TestServer.h"

namespace Test {
	namespace End2End {
		class Crud : public ::testing::Test {
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

		// working with STUDENT entity
		TEST_F(Crud, Student_OnCreating_Success) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("student_id=-1"))
				.append_query(U("name=Mohsen")).append_query(U("family_name=Shiri")).append_query(U("birth_date=13710101"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Created);
			}).wait();
		}
		TEST_F(Crud, Student_OnSearching_Success) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("family_name=Naseri"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response)
			{
				EXPECT_EQ(response.status_code(), status_codes::OK);
				response.extract_json().then([](web::json::value result) {
					web::json::value expected = web::json::value::parse(L"[{\"student_id\": 1, \
					\"name\": \"Ali\", \"family_name\": \"Naseri\", \"birth_date\": \"13700101\"}]");
					EXPECT_EQ(result.serialize(), expected.serialize());
				}).wait();
			}).wait();
		}
		TEST_F(Crud, Student_OnUpdating_Success) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("family_name=Naseri"));
			auto body = web::json::value(U("{\"name\":\"Alireza\"}"));
			cli.make_request(methods::PUT, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
		TEST_F(Crud, Student_OnPatching_Success) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("family_name=Naseri"));
			auto body = web::json::value(U("{\"name\":\"Alireza\"}"));
			cli.make_request(methods::PATCH, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
		TEST_F(Crud, Student_OnIsolatedDeleting_Success) {
			auto req = uri_builder(local).append_path(U("student")).append_query(U("student_id=4"));
			cli.make_request(methods::DEL, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}


		// working with COURSE entity
		TEST_F(Crud, Course_OnCreating_Success) {
			auto req = uri_builder(local).append_path(U("course")).append_query(U("course_id=-1")).append_query(U("name=Geography"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Created);
			}).wait();
		}
		TEST_F(Crud, Course_OnSearching_Success) {
			auto req = uri_builder(local).append_path(U("course")).append_query(U("name=Physics"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
				response.extract_json().then([](web::json::value result) {
					web::json::value expected = web::json::value::parse(L"[{\"course_id\": 2, \"name\": \"Physics\"}]");
					EXPECT_EQ(result.serialize(), expected.serialize());
				});
			}).wait();
		}
		TEST_F(Crud, Course_OnUpdating_Success) {
			auto req = uri_builder(local).append_path(U("course")).append_query(U("name=Math"));
			auto body = web::json::value(U("{\"name\":\"Mathematics\"}"));
			cli.make_request(methods::PUT, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
		TEST_F(Crud, Course_OnIsolatedDeleting_Success) {
			auto req = uri_builder(local).append_path(U("course")).append_query(U("course_id=3"));
			cli.make_request(methods::DEL, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}


		// woring with CLASS entity
		TEST_F(Crud, Class_OnCreating_Success) {
			auto req = uri_builder(local).append_path(U("class")).append_query(U("class_id=-1"))
				.append_query(U("course_ref=1")).append_query(U("student_ref=2")).append_query(U("score_ref=1"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Created);
			}).wait();
		}
		TEST_F(Crud, Class_OnSearching_Success) {
			auto req = uri_builder(local).append_path(U("class")).append_query(U("course_ref=1"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
				response.extract_json().then([](web::json::value result) {
					web::json::value expected = web::json::value::parse(L"[{\"class_id\":1,\"course_ref\":1,\"student_ref\":1},\
					{\"class_id\":2,\"course_ref\":1,\"student_ref\":2},{\"class_id\":3,\"course_ref\":1,\"student_ref\":3}]");
					EXPECT_EQ(result.serialize(), expected.serialize());
				});
			}).wait();
		}
		TEST_F(Crud, Class_OnUpdating_Success) {
			auto req = uri_builder(local).append_path(U("class")).append_query(U("course_ref=1"));
			auto body = web::json::value(U("{\"student_ref\":1}"));
			cli.make_request(methods::PUT, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
		TEST_F(Crud, Class_OnIsolatedDeleting_Success) {
			auto req = uri_builder(local).append_path(U("class")).append_query(U("class_id=1"));
			cli.make_request(methods::DEL, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}


		// working with QUIZ entity 
		TEST_F(Crud, Quiz_OnCreating_Success) {
			auto req = uri_builder(local).append_path(U("quiz")).append_query(U("quiz_id=-1"))
				.append_query(U("class_ref=2")).append_query(U("student_ref=2")).append_query(U("grade=18"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Created);
			}).wait();
		}
		TEST_F(Crud, Quiz_OnSearching_Success) {
			auto req = uri_builder(local).append_path(U("quiz")).append_query(U("class_ref=1"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				response.extract_json().then([](web::json::value result) {
					web::json::value expected = web::json::value::parse(L"[{\"class_ref\":1,\"grade\":17.5,\"quiz_id\":1,\"student_ref\":1}\
																		  ,{\"class_ref\":1,\"grade\":15,\"quiz_id\":2,\"student_ref\":2}\
																		  ,{\"class_ref\":1,\"grade\":20,\"quiz_id\":3,\"student_ref\":3}]");
					EXPECT_EQ(result.serialize(), expected.serialize());
				});
			}).wait();
		}
		TEST_F(Crud, Quiz_OnUpdating_Success) {
			auto req = uri_builder(local).append_path(U("quiz")).append_query(U("quiz_id=2"));
			auto body = web::json::value(U("{\"grade\":20}"));
			cli.make_request(methods::PUT, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
		TEST_F(Crud, Quiz_OnIsolatedDeleting_Success) {
			auto req = uri_builder(local).append_path(U("quiz")).append_query(U("quiz_id=1"));
			cli.make_request(methods::DEL, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}


		// working with SCORE entity
		TEST_F(Crud, Score_OnCreating_Success) {
			auto req = uri_builder(local).append_path(U("score")).append_query(U("score_id=-1")).append_query(U("class_ref=1"))
				.append_query(U("student_ref=2")).append_query(U("sem_avg=14")).append_query(U("mid_term=18")).append_query(U("final_term=11"));
			cli.make_request(methods::POST, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::Created);
			}).wait();
		}
		TEST_F(Crud, Score_OnSearching_Success) {
			auto req = uri_builder(local).append_path(U("score")).append_query(U("class_ref=1"));
			cli.make_request(methods::GET, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
				response.extract_json().then([](web::json::value result) {
					web::json::value expected = web::json::value::parse(L"[{\"class_ref\":1,\"final_term\":17,\"mid_term\":15,\"score_id\":1,\
										\"sem_avg\":12.5,\"student_ref\":1},{\"class_ref\":1,\"final_term\":20,\"mid_term\":14,\"score_id\":2,\
										\"sem_avg\":17,\"student_ref\":2},{\"class_ref\":1,\"final_term\":18.25,\"mid_term\":14,\"score_id\":3,\
										\"sem_avg\":19,\"student_ref\":3}]");
					EXPECT_EQ(result.serialize(), expected.serialize());
				});
			}).wait();
		}
		TEST_F(Crud, Score_OnUpdating_Success) {
			auto req = uri_builder(local).append_path(U("score")).append_query(U("class_ref=1"));
			auto body = web::json::value(U("{\"final_term\":20}"));
			cli.make_request(methods::PUT, req.to_string(), body).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
		TEST_F(Crud, Score_OnIsolatedDeleting_Success) {
			auto req = uri_builder(local).append_path(U("score")).append_query(U("score_id=1"));
			cli.make_request(methods::DEL, req.to_string(), 0).then([](http_response response) {
				EXPECT_EQ(response.status_code(), status_codes::OK);
			}).wait();
		}
	}
}