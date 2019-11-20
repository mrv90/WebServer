#include "pch.h"
#include "TestServer.h"

Test::TestServer::TestServer(const web::uri& uri, bool in_memory_db) : Server(uri, in_memory_db)
{
	WriteSampleData();
}

Test::TestServer::~TestServer()
{
}

void Test::TestServer::WriteSampleData()
{
	const char* data = 
		 "INSERT INTO course VALUES (1, 'Math'), (2, 'Physics'); \
		  \
		  INSERT INTO student VALUES (1, 'Ali', 'Naseri', '13700101'), (2, 'Michael', 'Jackson', '13700301'), \
					 (3, 'Bob', 'Fisher', '13700401'), (4, 'Robin', 'Williams', '13700501'); \
		  \
		  INSERT INTO class VALUES (1, 1, 1, null), (2, 1, 2, null), (3, 1, 3, null); \
		  \
		  INSERT INTO quiz VALUES (1, 1, 1, 17.5), (2, 1, 2, 15.0), (3, 1, 3, 20.0); \
		  \
		  INSERT INTO score VALUES (1, 1, 1, 12.5, 15, 17), (2, 1, 2, 17, 14, 20), (3, 1, 3, 19, 14, 18.25);";

	data_cntx.exe_cmd(data);
}