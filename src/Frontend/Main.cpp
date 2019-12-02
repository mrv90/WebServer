#pragma once
#include "pch.h"
#include "../BackEnd/Net/Server.h"

int wmain(int argc, wchar_t *argv[]) {
	utility::string_t port = U("34568");
	if (argc == 2)
		port = argv[1];

	utility::string_t address = U("http://localhost:");
	address.append(port);

	uri_builder uri(address);
	//uri.append_path(U("Server/Action/"));

	// on initilize
	auto addr = uri.to_uri().to_string();
	BackEnd::Net::Server g_http(addr, false);
	g_http.open().wait();

	ucout << utility::string_t(U("Listening for request at: ")) << addr << std::endl;
	std::cout << "Press ENTER to exit." << std::endl;
	ucout << "------------------------------------------------------------------" << endl;

	std::string line;
	std::getline(std::cin, line);

	// on shutdown
	g_http.close().wait();
	return 0;
}