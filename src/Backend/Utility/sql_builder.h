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
	void add_pathes(const web::http::http_request& req, std::wstring &sql);
	void add_queries(const web::http::http_request& req, std::wstring &sql);
	std::wstring wrap_by_quotation(const std::wstring& equation);
	std::string wrap_by_quotation(const std::string& equation);
	bool primary_key_requested(const web::http::http_request& req);
	std::wstring convert_to_sql_format(const std::wstring& json_style);
};
