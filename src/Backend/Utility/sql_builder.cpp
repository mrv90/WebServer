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
	
	add_path_hierarchy(url, sql);
	
	if (sql.compare(sql.c_str()) == 0)
		sql.append("* ");

	add_root_path(url, sql, "FROM ");

	if (!req.relative_uri().query().empty())
		add_pair_queries(url, sql, " WHERE ");

	return sql;
}

std::string sql_builder::to_create_or_replace_cmd(const web::http::http_request& req)
{
	std::string url = utility::conversions::utf16_to_utf8(req.relative_uri().to_string());
	std::string sql("INSERT ");
	add_root_path(url, sql, "INTO ");

	if (!req.relative_uri().query().empty())
		add_value_of_queries(url, sql, " VALUES (");
	
	return sql.append(")");
}

std::string sql_builder::to_update_cmd(const web::http::http_request& req, const std::wstring& json_style_body)
{
	const std::string url = utility::conversions::utf16_to_utf8(req.relative_uri().to_string());
	std::string sql("UPDATE ");
	
	add_root_path(url, sql, "");
	sql.append(" SET " + convert_to_sql_format(utility::conversions::utf16_to_utf8(json_style_body)));
	add_pair_queries(url, sql, " WHERE ");

	return sql;
}

std::string sql_builder::to_delete_cmd(const web::http::http_request& req)
{
	const std::string url = utility::conversions::utf16_to_utf8(req.relative_uri().to_string());
	std::string sql("DELETE ");

	add_root_path(url, sql, "FROM ");
	/*add_path_hierarchy(url, sql);*/ // path hierarchy ??
	add_pair_queries(url, sql, " WHERE ");

	return sql;
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

void sql_builder::add_root_path(const std::string &url, std::string &sql, const char* key_word)
{
	const boost::regex path_ptrn("(?<=^\/)([a-zA-Z0-9]+)"); // main math or simply path
	boost::smatch path_rslt;
	boost::regex_search(url, path_rslt, path_ptrn);
	sql.append(key_word).append(path_rslt.empty() == false ? path_rslt[0].str() : "* ");
}

void sql_builder::add_queries(const web::http::http_request& req, std::wstring& sql)
{
	if (!req.request_uri().query().empty())
		sql.append(L" WHERE " + wrap_by_quotation(req.request_uri().query()));
}

void sql_builder::add_pair_queries(const std::string &url, std::string &sql, const char* kew_word)
{
	const boost::regex ptrn("[a-zA-Z0-9_=-]+((?=&)|(?=$))");
	boost::sregex_token_iterator it(url.begin(), url.end(), ptrn, 0);
	const boost::sregex_token_iterator end;
	sql.append(kew_word);
	auto start = it;
	for (; it != end; it++)
		*start != *it ? sql.append(" AND " + wrap_by_quotation(*it)) : sql.append(wrap_by_quotation(*it));
}

void sql_builder::add_value_of_queries(const std::string &url, std::string &sql, const char* kew_word) {

	const boost::regex ptrn("((?<==)[a-zA-Z0-9._-]+)");
	boost::sregex_token_iterator it(url.begin(), url.end(), ptrn, 0);
	const boost::sregex_token_iterator end;
	sql.append(kew_word);
	auto start = it;
	for (; it != end; it++) {
		*start != *it ? sql.append(", '" + *it + "'") : sql.append("'" + *it + "'");
	}
}

void sql_builder::add_path_hierarchy(const std::string &url, std::string &sql)
{
	const boost::regex opath_ptrn("(?<=(.\/))(\w+)"); // other pathes pattern
	boost::sregex_token_iterator opath_itr(url.begin(), url.end(), opath_ptrn, 0);
	const boost::sregex_token_iterator end;

	auto start = opath_itr;
	for (; opath_itr != end; opath_itr++) {
		*start != *opath_itr ? sql.append(*opath_itr + ", ") : sql.append(*opath_itr);
	}
}

std::wstring sql_builder::wrap_by_quotation(const std::wstring& equation)
{
	boost::regex after_equal_sign("((?<==)[a-zA-Z0-9._-]+)"); // we have float: 17.5 has single dot
	std::string result = boost::regex_replace(
		std::string(equation.begin(), equation.end()), after_equal_sign, "'$&'");

	return std::wstring(result.begin(), result.end());
}

std::string sql_builder::wrap_by_quotation(const std::string& equation)
{
	boost::regex after_equal_sign("((?<==)[a-zA-Z0-9._-]+)"); // we have float: 17.5 has single dot
	return boost::regex_replace(equation, after_equal_sign, "'$&'");
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

std::string sql_builder::convert_to_sql_format(const std::string& json_style) {
	std::string body = std::string(json_style.begin(), json_style.end());
	boost::regex key("[a-zA-Z0-9_]+(?=\":)");
	boost::regex val("[a-zA-Z0-9_]+(?=(\"}|}))");
	boost::smatch key_result;
	boost::smatch val_result;

	boost::regex_search(body, key_result, key);
	boost::regex_search(body, val_result, val);

	return key_result[0].str() + "='" + val_result[0].str() + "'";
}