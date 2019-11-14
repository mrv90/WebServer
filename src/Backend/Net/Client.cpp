#include "pch.h"
#include "Client.h"

BackEnd::Net::Client::Client(const web::uri & uri) : hc(uri)
{
}

//BackEnd::Net::Client::Client(const Client & other) : hc(other.hc)
//{
//}
//
//BackEnd::Net::Client & BackEnd::Net::Client::operator=(const Client & other)
//{
//	if (this != &other)
//		this->hc = other.hc;
//	
//	return *this;
//}

BackEnd::Net::Client::~Client()
{
}

pplx::task<http_response> BackEnd::Net::Client::make_request(const web::http::method & verb, const utility::string_t & path , 
	const json::value & body)
{
	return (verb == methods::GET || verb == methods::HEAD) ? hc.request(verb, path) : hc.request(verb, path, body);
}