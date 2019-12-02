#pragma once
#pragma message("compiling pch.h in backend project.\n")

#include <string>

#include <utility>
#include <algorithm>

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"

#include "sqlite3.h"

#include <boost/regex.hpp>
#include <boost/date_time.hpp>