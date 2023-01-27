#include "CgipException.h"

namespace cgip {
	CgipException::CgipException(std::string msg){
		m_err_msg = msg;
	}

	CgipUnexistException::CgipUnexistException(std::string msg) : CgipException(msg) {
	}

	CgipUnknownException::CgipUnknownException(std::string msg) : CgipException(msg) {
	}

	CgipOutofRangeException::CgipOutofRangeException(std::string msg) : std::out_of_range(msg) {
		m_err_msg = msg;
	}

	CgipCLException::CgipCLException(std::string msg) : CgipException(msg) {
	}
};
