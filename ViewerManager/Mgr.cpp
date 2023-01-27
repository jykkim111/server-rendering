#include "Mgr.h"

Mgr::Mgr() {
	m_logger_dev = spdlog::get("logger_dev");
	m_logger_daily = spdlog::get("logger_daily");
}

void registerLogger(std::shared_ptr<spdlog::logger> logger) {
	spdlog::register_logger(logger);
}