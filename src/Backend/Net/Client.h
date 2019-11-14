#pragma once
#include "pch.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;

namespace BackEnd{
	namespace Net {
		class Client
		{
		public:
			Client(const web::uri& uri);
			/*Client(const Client& other);
			Client(Client&& other) = delete;
			Client& operator=(const Client& other);
			Client& operator=(Client&& other) = delete;*/
			virtual ~Client();

			pplx::task<http_response> make_request(const web::http::method & verb,
				const utility::string_t & path, const json::value & body);

		private:
			http_client hc;
		};
	}
}