#pragma once
#pragma message("compiling pch.h in test project.\n")

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
//#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

#include "sqlite3.h"

#include <utility>

#include <uriparser\Uri.h>
#include <uriparser\UriBase.h>
#include <uriparser\UriDefsAnsi.h>
#include <uriparser\UriDefsConfig.h>
#include <uriparser\UriDefsUnicode.h>
#include <uriparser\UriIp4.h>