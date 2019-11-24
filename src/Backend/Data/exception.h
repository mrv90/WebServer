#pragma once
#include "pch.h"

namespace BackEnd {
	namespace Data {
		struct exception : public std::exception {
			const int _type = -1;
			const char* _msg = "";
		
		public:
			exception(const char* msg, const int type) : _msg(msg), _type(type) {}
			~exception() noexcept {};

			virtual const char* what() const noexcept override {
				return _msg;
			}

			int error_type() const noexcept {
				return _type;
			}
		};
	}
}