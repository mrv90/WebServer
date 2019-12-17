#pragma once
#include "pch.h"

enum class Type {
	Integer = SQLITE_INTEGER,
	Float = SQLITE_FLOAT,
	Text = SQLITE_TEXT,
	Blob = SQLITE_BLOB,
	Null = SQLITE_NULL
};