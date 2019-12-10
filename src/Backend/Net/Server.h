#pragma once
#include "pch.h"
#include "../Utility/sql_builder.h"
#include "../Data/DataContext.h"
#include "../../Backend/Data/exception.h"
		
using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

namespace BackEnd {
	namespace Net {
		class Server
		{
		public:
			Server(const web::uri & uri, bool in_memory_db = false);
			Server(const Server& other);
			Server(Server&& other);
			Server& operator=(const Server& other);
			Server& operator=(Server&& other);
			virtual ~Server();

			pplx::task<void> open() { return http_listener.open(); }
			pplx::task<void> close() { return http_listener.close(); }
		
		protected:
			Server(const web::uri & uri);
			BackEnd::Data::DataContext data_cntx;

		private:
			virtual void handle_get(http_request req);
			virtual void handle_post(http_request req);
			virtual void handle_put(http_request req);
			virtual void handle_patch(http_request req);
			virtual void handle_delete(http_request req);
			virtual void handle_options(http_request req);
			virtual void handle_head(http_request req);

			void answer_request(const int query_status, const web::http::http_request & req, const web::json::value & resp = NULL);
			void answer_request(const web::http::http_request & req, const int http_code);
			bool contains_id(const web::http::http_request& req);
			void print_current_date_time();
			void print_requst_date(const web::http::http_request& req);
			bool is_a_valid_request(const web::http::http_request& req);
			bool has_valid_fragments(const web::http::http_request& req);
			bool has_valid_queries(const web::http::http_request& req);

			http_listener http_listener;
		};
	}
}