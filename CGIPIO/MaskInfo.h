#pragma once

#include <string>

typedef struct MaskInfo {
	std::string uid;
	std::string ref_uid;
	std::string worker;
	std::string performed_physician;
	long date;
	std::string path;
} MaskInfo;
