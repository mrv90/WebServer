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
			req.reply(status_codes::BadRequest);
		else if (e.error_type() == SQLITE_DONE)
			req.reply(status_codes::NotFound);
		else
			req.reply(status_codes::InternalError);
	}
}

void BackEnd::Net::Server::handle_post(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	if (contains_id(req))
		req.reply(status_codes::MethodNotAllowed);

	std::string chk_exist = sql_builder().to_select_query(req);
	if (data_cntx.verify_data(chk_exist) == false) {
		std::string create = sql_builder().to_create_or_replace_cmd(req);
		answer_request(data_cntx.exe_cmd(create), req);
	}
	else
		req.reply(status_codes::Conflict); // duplicated data
}

void BackEnd::Net::Server::handle_put(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	try
	{
		std::string chk_exist = sql_builder().to_select_query(req);
		if (data_cntx.verify_query_and_data(chk_exist) == false)
			req.reply(status_codes::NotFound);

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
			req.reply(status_codes::BadRequest);
		else if (e.error_type() == SQLITE_DONE)
			req.reply(status_codes::NotFound);
		else
			req.reply(status_codes::InternalError);
	}
}

void BackEnd::Net::Server::handle_patch(http_request req)
{
	print_current_date_time();
	print_requst_date(req);
	
	std::wstring req_body = L"";
	auto body = req.extract_string().then([&req_body](std::wstring ret_body) {
		req_body = ret_body;
		req_body.erase(std::remove(req_body.begin(), req_body.end(), '\"'), req_body.end());
	}).wait();
	std::string update = sql_builder().to_update_cmd(req, req_body);

	answer_request(data_cntx.exe_cmd(update.c_str()), req);
}

void BackEnd::Net::Server::handle_delete(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	std::string chk_exist = sql_builder().to_select_query(req);
	if (data_cntx.verify_query_and_data(chk_exist)) {
		std::string del = sql_builder().to_delete_cmd(req);
		answer_request(data_cntx.exe_cmd(del), req);
	}
	else
		req.reply(status_codes::NotFound);
}

void BackEnd::Net::Server::handle_options(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	std::wstring verbs = sql_builder().to_allowed_verbs(req);
	answer_request(SQLITE_OK, req, web::json::value::parse(verbs));
}

void BackEnd::Net::Server::handle_head(http_request req)
{
	print_current_date_time();
	print_requst_date(req);

	std::string select = sql_builder().to_select_query(req);

	web::json::value resp{};
	answer_request(data_cntx.exe_query(select, resp), req);
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
	
		std::wcout << "result: " << (req.method() == methods::POST ? "Created" : "OK") << ";\n" 
			<< "response: " << resp.serialize().c_str() << "." << endl;
	}
		break;
	case (SQLITE_MISUSE || SQLITE_ERROR):
		req.reply(status_codes::BadRequest);
		std::wcout << "result: BadRequest" << endl;
		break;
	case (SQLITE_INTERNAL):
		req.reply(status_codes::InternalError);
		std::wcout << "result: InternalError" << endl;
		break;
	default:
		std::wcout << "result: NoResultDefined!" << endl;
		break;
	}

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
		std::wcout << "request body: " << req.body() << endl;
	}
	else {
		std::wcout << "request method detected:" << method.c_str() << endl;
	}
}