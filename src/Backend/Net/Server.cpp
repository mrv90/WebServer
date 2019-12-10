#include "pch.h"
#include "Server.h"

BackEnd::Net::Server::Server(const web::uri & uri, bool in_memory_db) : http_listener(uri), data_cntx(std::move(BackEnd::Data::DataContext(in_memory_db)))
{
	http_listener.support(methods::GET, std::bind(&Server::handle_get, this, std::placeholders::_1));
	http_listener.support(methods::POST, std::bind(&Server::handle_post, this, std::placeholders::_1));
	http_listener.support(methods::PUT, std::bind(&Server::handle_put, this, std::placeholders::_1));
	http_listener.support(methods::PATCH, std::bind(&Server::handle_patch, this, std::placeholders::_1));
	http_listener.support(methods::DEL, std::bind(&Server::handle_delete, this, std::placeholders::_1));
	http_listener.support(methods::OPTIONS, std::bind(&Server::handle_options, this, std::placeholders::_1));
	http_listener.support(methods::HEAD, std::bind(&Server::handle_head, this, std::placeholders::_1));
}

BackEnd::Net::Server::Server(const web::uri & uri) : BackEnd::Net::Server::Server(uri, false)
{
}

BackEnd::Net::Server::Server(const Server & other) : data_cntx(other.data_cntx)
{
}

BackEnd::Net::Server::Server(Server && other) : data_cntx(std::move(other.data_cntx))
{
}

BackEnd::Net::Server & BackEnd::Net::Server::operator=(const Server & other)
{
	if (this != &other) {
		this->data_cntx = other.data_cntx;
	}

	return *this;
}

BackEnd::Net::Server & BackEnd::Net::Server::operator=(Server && other)
{
	if (this != &other) {
		this->data_cntx = other.data_cntx;
		other.data_cntx = NULL;
	}

	return *this;
}

BackEnd::Net::Server::~Server()
{
}

void BackEnd::Net::Server::handle_get(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (is_a_valid_request(req)) {
		std::string get = sql_builder().to_select_query(req);
		try
		{
			if (data_cntx.verify_query_and_data(get)) {
				web::json::value resp{};
				answer_request(data_cntx.exe_query(get, resp), req, resp);
			}
		}
		catch (const BackEnd::Data::exception& e)
		{
			if (e.error_type() == SQLITE_ERROR)
				answer_request(req, status_codes::BadRequest);
			else if (e.error_type() == SQLITE_DONE)
				answer_request(req, status_codes::NotFound);
			else
				answer_request(req, status_codes::InternalError);
		}
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::handle_post(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (is_a_valid_request(req)) {
		if (contains_id(req))
			answer_request(req, status_codes::MethodNotAllowed);

		std::string chk_exist = sql_builder().to_select_query(req);
		if (data_cntx.verify_data(chk_exist) == false) {
			std::string create = sql_builder().to_create_or_replace_cmd(req);
			answer_request(data_cntx.exe_cmd(create), req);
		}
		else
			answer_request(req, status_codes::Conflict); // duplicated data
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::handle_put(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (is_a_valid_request(req)) {
		try
		{
			std::string chk_exist = sql_builder().to_select_query(req);
			if (data_cntx.verify_query_and_data(chk_exist) == false)
				answer_request(req, status_codes::NotFound);

			std::wstring req_body = L"";
			auto body = req.extract_string().then([&req_body](std::wstring ret_body) {
				req_body = ret_body;
				req_body.erase(std::remove(req_body.begin(), req_body.end(), '\"'), req_body.end());
			}).wait();
			std::string update = sql_builder().to_update_cmd(req, req_body);

			answer_request(data_cntx.exe_cmd(update), req);
		}
		catch (const BackEnd::Data::exception& e)
		{
			if (e.error_type() == SQLITE_ERROR)
				answer_request(req, status_codes::BadRequest);
			else if (e.error_type() == SQLITE_DONE)
				answer_request(req, status_codes::NotFound);
			else
				answer_request(req, status_codes::InternalError);
		}
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::handle_patch(http_request req)
{
	print_current_date_time();
	print_requst_date(req);
	
	if (is_a_valid_request(req)) {
		std::wstring req_body = L"";
		auto body = req.extract_string().then([&req_body](std::wstring ret_body) {
			req_body = ret_body;
			req_body.erase(std::remove(req_body.begin(), req_body.end(), '\"'), req_body.end());
		}).wait();
		std::string update = sql_builder().to_update_cmd(req, req_body);

		answer_request(data_cntx.exe_cmd(update.c_str()), req);
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::handle_delete(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (is_a_valid_request(req)) {
		std::string chk_exist = sql_builder().to_select_query(req);
		if (data_cntx.verify_query_and_data(chk_exist)) {
			std::string del = sql_builder().to_delete_cmd(req);
			answer_request(data_cntx.exe_cmd(del), req);
		}
		else
			answer_request(req, status_codes::NotFound);
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::handle_options(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (is_a_valid_request(req)) {
		std::wstring verbs = sql_builder().to_allowed_verbs(req);
		answer_request(SQLITE_OK, req, web::json::value::parse(verbs));
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::handle_head(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (is_a_valid_request(req)) {
		std::string select = sql_builder().to_select_query(req);

		web::json::value resp{};
		answer_request(data_cntx.exe_query(select, resp), req);
	}
	else
		answer_request(req, status_codes::BadRequest);
}

void BackEnd::Net::Server::answer_request(const int query_status, const web::http::http_request & req, const web::json::value & resp)
{
	switch (query_status) {
	case (SQLITE_OK): {
		if (req.method() == methods::GET)
			req.reply(status_codes::OK, resp);
		else if (req.method() == methods::POST)
			req.reply(status_codes::Created);
		else if (req.method() == methods::PUT)
			req.reply(status_codes::OK);
		else if (req.method() == methods::PATCH)
			req.reply(status_codes::OK);
		else if (req.method() == methods::DEL)
			req.reply(status_codes::OK);
		else if (req.method() == methods::HEAD)
			req.reply(status_codes::OK);
		else if (req.method() == methods::OPTIONS)
			req.reply(status_codes::OK, resp);
	
		std::wcout << "Result: " << (req.method() == methods::POST ? "Created" : "OK") << ";\n" 
			<< "response: " << resp.serialize().c_str() << "." << endl;
	}
		break;
	case (SQLITE_MISUSE || SQLITE_ERROR):
		req.reply(status_codes::BadRequest);
		std::wcout << "Result: BadRequest" << endl;
		break;
	case (SQLITE_INTERNAL):
		req.reply(status_codes::InternalError);
		std::wcout << "Result: InternalError" << endl;
		break;
	default:
		std::wcout << "Result: no result defined!" << endl;
		break;
	}

	std::wcout << "------------------------------------------------------------------" << endl;
}

void BackEnd::Net::Server::answer_request(const web::http::http_request & req, const int http_code) {
	std::wcout << "Result: " << http_code << endl;
	req.reply(http_code);

	std::wcout << "------------------------------------------------------------------" << endl;
}

bool BackEnd::Net::Server::contains_id(const web::http::http_request& req) {

	const boost::wregex id(L"_id");
	return boost::regex_search(req.absolute_uri().to_string(), id);
}

void BackEnd::Net::Server::print_current_date_time() {
	auto local = boost::posix_time::second_clock::local_time();
	std::wcout << local.date() << " : " << local.time_of_day() << endl;
}

void BackEnd::Net::Server::print_requst_date(const web::http::http_request& req) {
	const::web::http::method method = req.method();
	if (method == methods::GET ||
		method == methods::HEAD ||
		method == methods::OPTIONS ||
		method == methods::DEL) {
		std::wcout << req.method().c_str() << ": " << req.relative_uri().to_string() << endl;
	}
	else if (method == methods::POST ||
		method == methods::PUT ||
		method == methods::PATCH) {
		std::wcout << req.method().c_str() << ": " << req.relative_uri().to_string() << endl;
		std::wcout << "Request body: " << req.body() << endl;
	}
	else {
		std::wcout << "Request method detected:" << method.c_str() << endl;
	}
}

bool BackEnd::Net::Server::is_a_valid_request(const web::http::http_request& req) {
	//TODO check url structure using regex with atleast one fragment and optional queries
	// replace it with check empty uri

	if (req.request_uri().is_empty()) {
		std::wcout << "Error: " << "unable to process empty request!" << endl;
		return false;
	}
	
	else if (!has_valid_fragments(req)) {
		std::wcout << "Error: " << "invalid fragments detected!" << endl;
		return false;
	}

	else if (!has_valid_queries(req)) {
		std::wcout << "Error: " << "invalid queries detected inside request!" << endl;
		return false;
	}

	// check http_verb for suffient fragment | query

	return true;
}

bool BackEnd::Net::Server::has_valid_fragments(const web::http::http_request& req) {
	if (req.request_uri().path().size() > 1) {
		auto pathes = req.request_uri().split_path(req.request_uri().path());
		auto entities = data_cntx.get_data_entities();

		if (pathes.size() > 1) {
			for (auto p : pathes) {
				if (std::find(pathes.begin(), pathes.end(), p.c_str()) != pathes.end())
					break;
				else
					return false;
			}
		}
	}

	return true;
}

bool BackEnd::Net::Server::has_valid_queries(const web::http::http_request& req) {
	if (req.request_uri().query().size() > 1) {
		auto queries = req.request_uri().split_query(req.request_uri().query());

		if (queries.size() > 1) {
			for (auto q : queries) {
				auto fields = data_cntx.get_data_fields(q.first);
				if (std::find(fields.begin(), fields.end(), q.first.c_str()) != fields.end())
					break;
				else
					return false;
			}
		}
	}

	return true;
}