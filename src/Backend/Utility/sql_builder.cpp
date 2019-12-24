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
	std::string url = utility::conversions::utf16_to_utf8(req.relative_uri().to_string());
	std::string sql("SELECT ");
	const boost::sregex_token_iterator end;
	
	const boost::regex opath_ptrn("(?<=(.\/))(\w+)"); // other pathes pattern or opath_ptrn
	boost::sregex_token_iterator opath_itr(url.begin(), url.end(), opath_ptrn, 0);
	auto start = opath_itr;
	for (; opath_itr != end; opath_itr++)
		*start != *opath_itr ? sql.append(*opath_itr + ", ") : sql.append(*opath_itr);
	if (sql.compare(sql.c_str()) == 0)
		sql.append("* ");

	const boost::regex path_ptrn("(?<=^\/)([a-zA-Z0-9]+)"); // main math or simply path
	boost::smatch path_rslt;
	boost::regex_search(url, path_rslt, path_ptrn);
	sql.append("FROM ").append(path_rslt.empty() == false ? path_rslt[0].str() : "* ");

	if (!req.relative_uri().query().empty()) {
		const boost::regex q_ptrn("[a-zA-Z0-9_=-]+((?=&)|(?=$))"); // q or queries
		boost::sregex_token_iterator q_itr(url.begin(), url.end(), q_ptrn, 0);
		sql.append(" WHERE ");
		start = q_itr;
		for (; q_itr != end; q_itr++)
			*start != *q_itr ? sql.append(" AND " + wrap_by_quotation(*q_itr)) : sql.append(wrap_by_quotation(*q_itr));
	}

	return sql;
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

std::string sql_builder::wrap_by_quotation(const std::string& equation)
{
	boost::regex after_equal_sign("((?<==)[a-zA-Z0-9.]+)"); // we have float in data; 17.5 has single dot
	return boost::regex_replace(equation, after_equal_sign, "'$0'");
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