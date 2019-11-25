#include "pch.h"
#include "DataContext.h"
#include "Exception.h"

BackEnd::Data::DataContext::DataContext()
{
	if (SQLITE_OK != sqlite3_open("", &con)) // temporary database
		auto err = sqlite3_errmsg(con);

	ApplyDbStructure();
}

BackEnd::Data::DataContext::DataContext(bool in_memory)
{
	if (in_memory) {
		if (SQLITE_OK != sqlite3_open("", &con)) // temporary database
			auto err = sqlite3_errmsg(con);
	}
	else {
		if (SQLITE_OK != sqlite3_open_v2("WebServer.db", &con, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL))
			auto err = sqlite3_errmsg(con);
	}

	ApplyDbStructure();
}

BackEnd::Data::DataContext::DataContext(const DataContext & other) : con {other.con}
{
}

BackEnd::Data::DataContext::DataContext(DataContext && other) : con(std::move(other.con))
{
	other.con = nullptr;
}

BackEnd::Data::DataContext & BackEnd::Data::DataContext::operator=(const DataContext & other)
{
	if (this != &other)
		this->con = other.con;
	
	return *this;
}

BackEnd::Data::DataContext & BackEnd::Data::DataContext::operator=(DataContext && other)
{
	if (this != &other) {
		con = other.con;
		other.con = nullptr;
	}

	return *this;
}

BackEnd::Data::DataContext::~DataContext()
{
	if ((nullptr != con) && (SQLITE_OK != sqlite3_close(con)))
		auto err = sqlite3_errmsg(con);
}

int BackEnd::Data::DataContext::exe_cmd(const std::string& cmd)
{
	int ret = -1;
	char** errmsg = nullptr;

	// TODO: remove id and enable from all responses
	// TODO: add exception 
	// TODO: ucout result of execmd
	sqlite3_exec(con, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if (SQLITE_OK != (ret = sqlite3_exec(con, cmd.c_str(), NULL, NULL, errmsg))) {
		auto err = (const char*)&errmsg;
		return ret;
	}
	sqlite3_exec(con, "END TRANSACTION;", NULL, NULL, NULL);

	return ret;
}

int BackEnd::Data::DataContext::exe_query(const std::string& query, web::json::value &resp)
{
	int ret = -1;
	sqlite3_stmt* stmt = NULL;
	
	// TODO: add exception 
	// TODO: ucout result of exequery
	if (SQLITE_OK != (ret = sqlite3_prepare_v2(con, query.c_str(), -1, &stmt, NULL))) {
		auto err = sqlite3_errmsg(con);
		return ret;
	}

	if (SQLITE_ROW == (ret = sqlite3_step(stmt))) {
		utility::stringstream_t istrm;
		istrm << L"{";
		for (unsigned int i = 0; i <= sqlite3_data_count(stmt); i++) {
			
			switch (sqlite3_column_type(stmt, i)) 
			{
			case (SQLITE_INTEGER):
				(0 < i && i < sqlite3_data_count(stmt)) ?
					istrm << L", \"" << sqlite3_column_name(stmt, i) << L"\": " << sqlite3_column_int(stmt, i) :
					istrm << L"\"" << sqlite3_column_name(stmt, i) << L"\": " << sqlite3_column_int(stmt, i);
				break;
			case (SQLITE_TEXT):
				(0 < i && i < sqlite3_data_count(stmt)) ?
					istrm << L", \"" << sqlite3_column_name(stmt, i) << L"\": \"" << (const char*)sqlite3_column_text(stmt, i) << L"\"" :
					istrm << L"\"" << sqlite3_column_name(stmt, i) << L"\": \"" << (const char*)sqlite3_column_text(stmt, i) << L"\"";
				break;
			case (SQLITE_FLOAT):
				(0 < i && i < sqlite3_data_count(stmt)) ?
					istrm << L", \"" << sqlite3_column_name(stmt, i) << L"\": " << sqlite3_column_double(stmt, i) :
					istrm << L"\"" << sqlite3_column_name(stmt, i) << L"\": " << sqlite3_column_double(stmt, i);
				break;
			case (SQLITE_BLOB):
				(0 < i && i < sqlite3_data_count(stmt)) ?
					istrm << L", \"" << sqlite3_column_name(stmt, i) << L"\": \"" << sqlite3_column_blob(stmt, i) << L"\"" :
					istrm << L"\"" << sqlite3_column_name(stmt, i) << L"\": \"" << sqlite3_column_blob(stmt, i) << L"\"";
				break;
			case (SQLITE_NULL): // end-of-data flag
				break;
			default:
				(0 < i && i < sqlite3_data_count(stmt)) ?
					istrm << L", \"" << sqlite3_column_name(stmt, i) << L"\": \"" << sqlite3_column_value(stmt, i) << L"\"" :
					istrm << L"\"" << sqlite3_column_name(stmt, i) << L"\": \"" << sqlite3_column_value(stmt, i) << L"\"";
				break;
			}
		}
		
		istrm << L"}";
		resp = web::json::value::parse(istrm);
	}
	else {
		auto err = sqlite3_errmsg(con);
		return ret;
	}

	if (NULL != stmt)
		sqlite3_finalize(stmt);

	return SQLITE_OK;
}

bool BackEnd::Data::DataContext::verify_query_and_data(const std::string& query) {
	int ret = -1;
	sqlite3_stmt* stmt = NULL;

	// TODO: ucout result of exequery
	if (SQLITE_OK != (ret = sqlite3_prepare_v2(con, query.c_str(), -1, &stmt, NULL))) {
		auto err = sqlite3_errmsg(con);
		throw BackEnd::Data::exception(err, ret);
	}

	if (SQLITE_ROW == (ret = sqlite3_step(stmt)))
		return true;
	
	else {
		auto err = sqlite3_errmsg(con);
		throw BackEnd::Data::exception(err, ret);
	}
}

bool BackEnd::Data::DataContext::verify_data(const std::string& query) {
	int ret = -1;
	sqlite3_stmt* stmt = NULL;

	// TODO: ucout result of exequery
	if (SQLITE_OK != (ret = sqlite3_prepare_v2(con, query.c_str(), -1, &stmt, NULL))) {
		return false;
	}

	if (SQLITE_ROW == (ret = sqlite3_step(stmt)))
		return true;
	else if (SQLITE_DONE == (ret = sqlite3_step(stmt)))
		return false;
	else
		throw std::exception("unhandeled sqlite error code");
}

int BackEnd::Data::DataContext::ApplyDbStructure()
{
	// TODO: add exception 
	// TODO: ucout result of execmd
	const char* alter_db_structure = 
		"CREATE TABLE IF NOT EXISTS course (course_id INTEGER PRIMARY KEY, name TEXT);\
		\
		CREATE TABLE IF NOT EXISTS student(student_id INTEGER PRIMARY KEY, name TEXT, family_name TEXT, birth_date TEXT);\
		\
		CREATE TABLE IF NOT EXISTS score\
		(\
			score_id INTEGER PRIMARY KEY,\
			class_ref INTEGER REFERENCES class(class_id) ON UPDATE CASCADE ON DELETE RESTRICT,\
			student_ref INTEGER REFERENCES student(student_id) ON UPDATE CASCADE ON DELETE RESTRICT,\
			sem_avg FLOAT,\
			mid_term FLOAT,\
			final_term INTEGER\
		);\
		\
		CREATE TABLE IF NOT EXISTS class\
		(\
			class_id INTEGER PRIMARY KEY,\
			course_ref INTEGER REFERENCES course(course_id) ON UPDATE CASCADE ON DELETE RESTRICT,\
			student_ref INTEGER REFERENCES student(student_id) ON UPDATE CASCADE ON DELETE RESTRICT,\
			score_ref INTEGER REFERENCES score(score_id) ON UPDATE CASCADE ON DELETE RESTRICT\
			);\
		\
		CREATE TABLE IF NOT EXISTS quiz\
		(\
			quiz_id INTEGER PRIMARY KEY,\
			class_ref INTEGER REFERENCES class(class_id) ON UPDATE CASCADE,\
			student_ref INTEGER REFERENCES student(student_id) ON UPDATE CASCADE,\
			score INTEGER\
		);";

	int ret = -1;
	if (SQLITE_OK != (ret = exe_cmd(alter_db_structure))) {
		auto err = sqlite3_errmsg(con);
		return ret;
	}

	return SQLITE_OK;
}