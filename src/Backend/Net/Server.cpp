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
			else
				answer_request(req, status_codes::NotFound);
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
		if (contains_id_equal_to_minus_one(req)) {
			std::string chk_exist = sql_builder().to_select_query(req);
			if (data_cntx.verify_data(chk_exist) == false) {
				std::string create = sql_builder().to_create_or_replace_cmd(req);
				answer_request(data_cntx.exe_cmd(create), req);
			}
			else
				answer_request(req, status_codes::Conflict); // duplicated data
		}
		else
			answer_request(req, status_codes::MethodNotAllowed);
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

			auto raw = req.extract_json(true).get();
			std::wstring body = raw.is_string() ? raw.as_string() : json::value(raw).serialize();
			std::string update = sql_builder().to_update_cmd(req, body);
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
		std::string update = sql_builder().to_update_cmd(req, req.extract_json(true).get().as_string());
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

void BackEnd::Net::Server::answer_request(const int query_status, const web::http::http_request & req, const web::json::value & body)
{
	http_response resp;

	switch (query_status) {
	case (SQLITE_OK): {
		if (req.method() == methods::OPTIONS) {
			resp = http_response(status_codes::OK);
			resp.headers().add(U("Allow"), U("GET, POST, PUT, PATCH, DELETE, OPTIONS, HEAD"));
			resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			resp.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, PATCH, DELETE, OPTIONS, HEAD"));
			resp.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
		}
		else if (req.method() == methods::POST) {
			resp = http_response(status_codes::Created);
			resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			resp.set_body(body);
			std::wcout << "response: " << body.serialize().c_str() << "." << endl;
		}
		else {
			resp = http_response(status_codes::OK);
			resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));

			if (req.method() == methods::GET)
				resp.set_body(body);
		}
	}
					  break;
	case (SQLITE_MISUSE || SQLITE_ERROR):
		resp = http_response(status_codes::BadRequest);
		std::wcout << "Result: BadRequest" << endl;
		break;
	case (SQLITE_INTERNAL):
		resp = http_response(status_codes::InternalError);
		std::wcout << "Result: InternalError" << endl;
		break;
	default:
		std::wcout << "Result: no result defined!" << endl;
		break;
	}

	std::wcout << "Result: " << (req.method() == methods::POST ? "Created" : "OK") << ";\n";
	req.reply(resp);
	std::wcout << "------------------------------------------------------------------" << endl;
}

void BackEnd::Net::Server::answer_request(const web::http::http_request & req, const int http_code) {
	http_response resp(http_code);
	resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	std::wcout << "Result: " << http_code << endl;

	req.reply(resp);

	std::wcout << "------------------------------------------------------------------" << endl;
}

bool BackEnd::Net::Server::contains_id_equal_to_minus_one(const web::http::http_request& req) {

	const boost::wregex id(L"_id=-1");
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
	if (req.method() == methods::GET)
		return must_have_atleast_one_path(req) && contains_valid_pathes(req)
		&& contains_valid_queries(req);
	else if (req.method() == methods::POST)
		return must_have_atleast_one_path(req) && contains_valid_pathes(req);
	else if (req.method() == methods::PUT || req.method() == methods::PATCH)
		return must_have_atleast_one_path(req) && contains_valid_pathes(req)
		&& must_have_atleast_one_query(req) && contains_valid_queries(req)
		&& contains_json_body(req);
	else if (req.method() == methods::DEL)
		return must_have_atleast_one_path(req) && contains_valid_pathes(req)
		&& must_have_atleast_one_query(req) && contains_valid_queries(req);
	else if (req.method() == methods::HEAD || req.method() == methods::OPTIONS)
		return could_have_some_pathes(req);
	else {
		std::wcout << "Error: " << "undefined validation on :" << req.method() << " requst" << endl;
	}

	return false;
}

bool BackEnd::Net::Server::could_have_some_pathes(const web::http::http_request& req) {
	const boost::wregex wr(L"\/{1}[a-zA-Z0-9/]*");
	if (!boost::regex_search(req.relative_uri().to_string(), wr)) {
		std::wcout << "Error: " << "expecting correct url with optional path" << endl;
		return false;
	}

	return true;
}

bool BackEnd::Net::Server::must_have_atleast_one_path(const web::http::http_request& req) {
	const boost::wregex wr(L"\/{1}[a-zA-Z0-9/]+");
	if (!boost::regex_search(req.relative_uri().to_string(), wr)) {
		std::wcout << "Error: " << "expecting atleast one path" << endl;
		return false;
	}

	return true;
}

bool BackEnd::Net::Server::must_have_atleast_one_query(const web::http::http_request& req) {
	const boost::wregex wr(L"\&?[a-zA-Z0-9]+\=[a-zA-Z0-9]+");
	if (!boost::regex_search(req.relative_uri().to_string(), wr)) {
		std::wcout << "Error: " << "expecting atleast one path & query" << endl;
		return false;
	}

	return true;
}

bool BackEnd::Net::Server::contains_valid_pathes(const web::http::http_request& req) {
	if (req.request_uri().path().size() > 1) {
		auto pathes = req.request_uri().split_path(req.request_uri().path());
		auto entities = data_cntx.get_data_entities();

		if (!pathes.empty()) {
			for (auto p : pathes) {
				if (std::find(entities.begin(), entities.end(), p.c_str()) != entities.end())
					break;
				else {
					std::wcout << "Error: " << "invalid pathes detected!" << endl;
					return false;
				}
			}
		}
	}

	return true;
}

bool BackEnd::Net::Server::contains_valid_queries(const web::http::http_request& req) {
	if (req.request_uri().query().size() > 1) {
		auto queries = req.request_uri().split_query(req.request_uri().query());

		if (!queries.empty()) {
			for (auto q : queries) {
				auto path = req.request_uri().path().substr(1, req.request_uri().path().size() - 1);
				if (data_cntx.chk_data_field(path, q.first.c_str()))
					break;
				else {
					std::wcout << "Error: " << "invalid queries detected inside request!" << endl;
					return false;
				}
			}
		}
	}

	return true;
}

bool BackEnd::Net::Server::contains_json_body(const web::http::http_request & req)
{
	try
	{
		/*auto raw = req.extract_json(true).get();
		web::json::value body = web::json::value::string(raw.is_string() ? raw.as_string() : json::value(raw).serialize());
		req.set_body(body);

		if (!body.is_null())*/
		return true;

		/*return false;*/
	}
	catch (const std::exception&)
	{
		return false;
	}
}