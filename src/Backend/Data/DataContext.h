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

			int ExeCmd(const std::string& cmd);
			int ExeQuery(const std::string& query, web::json::value &resp);
		
		protected:
			DataContext();

		private:
			sqlite3* con = nullptr;

			int ApplyDbStructure();
		};
	}
}