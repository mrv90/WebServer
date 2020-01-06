#include "pch.h"
#include "../BackEnd/Net/Client.h"
#include "../Backend/Utility/sql_builder.h"


namespace Test {
	namespace End2End {
		class SqlBuilder : public ::testing::Test {
		protected:
			web::uri local = uri_builder(U("http://127.0.0.1:34568")).to_uri();
			BackEnd::Net::Client cli = BackEnd::Net::Client(local);
		};

		TEST_F(SqlBuilder, PUT_OnRequestWithBody_ProperSqlGeneration) {
			/*auto req = uri_builder(local).append_path(U("class")).append_query(U("class_id=-1"))
				.append_query(U("course_ref=1")).append_query(U("student_ref=2")).append_query(U("score_ref=1"));
			auto body = web::json::value(U("{\"name\":\"Mathematics\"}"));
			cli.make_request(methods::GET, req.to_string(), body);
			
			std::string put = sql_builder().to_update_cmd(req, body.as_string());*/
		}
	}
}