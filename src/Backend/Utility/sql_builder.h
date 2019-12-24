#pragma once

#include "pch.h"

class sql_builder final
{
public:
	sql_builder();
	~sql_builder();

	std::string to_select_query(const web::http::http_request& req);
	std::string to_create_or_replace_cmd(const web::http::http_request& req);
	std::string to_update_cmd(const web::http::http_request& req, const std::wstring& json_style_body);
	std::string to_delete_cmd(const web::http::http_request& req);
	std::wstring to_allowed_verbs(const web::http::http_request& req);

private:
	[[deprecated("This method replaced by add_root_path and add path_hierarchy")]]
	void add_pathes(const web::http::http_request& req, std::wstring &sql);
	void add_root_path(const std::string &url, std::string &sql, const char* key_word);
	void add_path_hierarchy(const std::string &url, std::string &sql);
	[[deprecated("This method replaced by add_pair_queries and add_value_of_queries")]]
	void add_queries(const web::http::http_request& req, std::wstring &sql);
	void add_pair_queries(const std::string &url, std::string &sql, const char* kew_word);
	void add_value_of_queries(const std::string &url, std::string &sql, const char* kew_word);
	std::wstring wrap_by_quotation(const std::wstring& equation);
	std::string wrap_by_quotation(const std::string& equation);
	bool primary_key_requested(const web::http::http_request& req);
	std::wstring convert_to_sql_format(const std::wstring& json_style);
	std::string convert_to_sql_format(const std::string& json_style);
};
