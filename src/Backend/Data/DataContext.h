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
			bool data_exist(const std::string& query);
		
		protected:
			DataContext();

		private:
			sqlite3* con = nullptr;

			int ApplyDbStructure();
		};
	}
}