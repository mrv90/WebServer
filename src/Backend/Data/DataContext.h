#pragma once
#include "pch.h"

namespace BackEnd {
	namespace Data {
		class DataContext
		{
		public:
			DataContext(bool in_memory = false);
			DataContext(const DataContext& other);
			DataContext(DataContext&& other);
			DataContext& operator=(const DataContext& other);
			DataContext& operator=(DataContext&& other);
			~DataContext();

			int exe_cmd(const std::string& cmd);
			int exe_query(const std::string& query, web::json::value &resp);
			int exe_query_scalar(const std::string& query, web::json::value &resp);
			bool verify_query_and_data(const std::string& query);
			bool verify_data(const std::string& query);
			std::vector<std::wstring> get_data_entities();
			bool chk_data_field(const std::wstring& entity, const std::wstring& field);
		
		protected:
			DataContext();

		private:
			sqlite3* con = nullptr;

			int ApplyDbStructure();
		};
	}
}