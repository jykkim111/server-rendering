#pragma once

#include "cgip_exception.h"
#include <exception>
#include <stdexcept>
#include <string>

namespace cgip {
	struct CGIPEXCEPTION_DLL CgipOpenGLOOMException : public std::exception {
		const char* what() const throw() {
			return "glError: out of memory";
		}
	};

	class CGIPEXCEPTION_DLL CgipOpenGLException : public std::exception {
	public:
		CgipOpenGLException(std::string msg) {
			m_err_msg = msg;
		};

		const char* what() const throw() {
			return m_err_msg.c_str();
		}

		std::string m_err_msg;
	};

	struct CGIPEXCEPTION_DLL CgipSlotNullException : public std::exception {
		const char* what() const throw() {
			return "cannot allocate memory";
		}
	};

	class CGIPEXCEPTION_DLL CgipException : public std::exception {
	public:
		CgipException(std::string msg);

		const char* what() const throw() override {
			return m_err_msg.c_str();
		}

	protected:
		std::string m_err_msg;

	};

	class CGIPEXCEPTION_DLL CgipUnexistException : public CgipException {
	public:
		CgipUnexistException(std::string msg);

		const char* what() const throw() override {
			return m_err_msg.c_str();
		}
	};

	class CGIPEXCEPTION_DLL CgipUnknownException : public CgipException {
	public:
		CgipUnknownException(std::string msg);

		const char* what() const throw() override {
			return m_err_msg.c_str();
		}
	};

	class CGIPEXCEPTION_DLL CgipOutofRangeException : public std::out_of_range {
	public:
		CgipOutofRangeException(std::string msg);

		const char* what() const throw() override {
			return m_err_msg.c_str();
		}

	private:
		std::string m_err_msg;

	};

	class CGIPEXCEPTION_DLL CgipCLException : public CgipException {
	public:
		CgipCLException(std::string msg);

		const char* what() const throw() override {
			return m_err_msg.c_str();
		}
	};
}
