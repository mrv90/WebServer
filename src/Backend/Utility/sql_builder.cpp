#include "pch.h"
#include "sql_builder.h"

sql_builder::sql_builder()
{
}

sql_builder::~sql_builder()
{
}

std::string sql_builder::to_select_query(const web::http::http_request& req)
{
	// TODO: remove id and enable from all responses
	// TODO: add exception 
	// TODO: ucout result of to_select
	std::wstring sql(L"SELECT ");

	auto pathes = req.request_uri().split_path(req.request_uri().path());
	
	if (pathes.size() > 1)
		add_pathes(req, sql);
	else
		sql.append(L"*");
		
	sql.append(L" FROM " + pathes.at(0));
	add_queries(req, sql);
	return std::string(sql.begin(), sql.end());
}

std::string sql_builder::to_create_or_replace_cmd(const web::http::http_request& req)
{
	// TODO: add exception 
	// TODO: ucout result of to_create
	std::wstring sql(L"INSERT INTO ");
	sql.append(req.request_uri().path().substr(1, req.request_uri().path().size() - 1));
	
	if (!req.request_uri().query().empty()) {
		std::wstring qkey_buffer, qval_buffer = L"";
		auto queries = req.request_uri().split_query(req.request_uri().query());

		for (std::map<std::wstring, std::wstring>::iterator it = queries.begin(); it != queries.end(); it++) {
			if (it != std::prev(queries.end())) {
				qkey_buffer.append(it->first + L", ");
				qval_buffer.append(L"'" + it->second + L"', ");
			}
			else {
				qkey_buffer.append(it->first);
				qval_buffer.append(L"'" + it->second + L"'");
			}
		}

		sql.append(L"(" + qkey_buffer + L") VALUES(" + qval_buffer + L") ");
	}
	
	return std::string(sql.begin(), sql.end());
}

std::string sql_builder::to_update_cmd(const web::http::http_request& req, const std::wstring& json_style_body)
{
	// TODO: add exception 
	// TODO: ucout result of to_update
	std::wstring sql(L"UPDATE ");
	sql.append(req.request_uri().path().substr(1, req.request_uri().path().size() -1));
	sql.append(L" SET " + convert_to_sql_format(json_style_body));// +L" WHERE ");
	add_queries(req, sql);
	return std::string(sql.begin(), sql.end());
}

std::string sql_builder::to_delete_cmd(const web::http::http_request& req)
{
	// TODO: add exception 
	// TODO: ucout result of to_delete
	std::wstring sql(L"DELETE FROM "); // no delete policy !
	auto pathes = req.request_uri().split_path(req.request_uri().path());
	add_pathes(req, sql);
	add_queries(req, sql);
	return std::string(sql.begin(), sql.end());
}

std::wstring sql_builder::to_allowed_verbs(const web::http::http_request& req)
{
	// TODO: add exception 
	// TODO: ucout result of to_allowed
	std::wstring allowed = L"{\"Allow\":";

	if (primary_key_requested(req))
		allowed.append(L"\"OPTIONS\"}");
	else
		allowed.append(L"[\"GET\", \"POST\", \"PUT\", \"PATCH\", \"DEL\", \"OPTIONS\", \"HEAD\"]}");

	return allowed;;
}

void sql_builder::add_pathes(const web::http::http_request& req, std::wstring& sql)
{
	auto pathes = req.request_uri().split_path(req.request_uri().path());
	if (req.request_uri().path().size() > 1) {
		if (pathes.size() > 1) {
			for (size_t i = 0; i <= pathes.size(); i++) {
				if (i == 0)
					break;
				sql.append(i < pathes.size() ? (pathes.at(i) + L".") : pathes.at(i));
			}
		}
		else
			(req.method() == web::http::methods::GET) ? sql.append(L" * ") : sql.append(pathes.at(0) + L" ");
	}
}

void sql_builder::add_queries(const web::http::http_request& req, std::wstring& sql)
{
	if (!req.request_uri().query().empty())
		sql.append(L" WHERE " + wrap_by_quotation(req.request_uri().query()));
}

std::wstring sql_builder::wrap_by_quotation(const std::wstring& equation)
{
	boost::regex after_equal_sign("((?<==)[a-zA-Z0-9.]+)"); // we have float in data; 17.5 has single dot
	std::string result = boost::regex_replace(
		std::string(equation.begin(), equation.end()), after_equal_sign, "'$0'");

	return std::wstring(result.begin(), result.end());
}

bool sql_builder::primary_key_requested(const web::http::http_request & req)
{
	std::wstring pathes = std::wstring(req.request_uri().path().c_str());
	boost::regex id("_id");
	if (boost::regex_search(pathes.begin(), pathes.end(), id))
		return true;

	return false;
}

std::wstring sql_builder::convert_to_sql_format(const std::wstring& json_style) {
	std::string body = std::string(json_style.begin(), json_style.end());
	boost::regex key("[a-zA-Z0-9_]+(?=\":)");
	boost::regex val("[a-zA-Z0-9_]+(?=(\"}|}))");
	boost::smatch key_result;
	boost::smatch val_result;

	boost::regex_search(body, key_result, key);
	boost::regex_search(body, val_result, val);

	std::string result = key_result[0].str() + "='" + val_result[0].str() + "'";
	return std::wstring(result.begin(), result.end());
}