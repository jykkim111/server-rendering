#pragma once

#include <QObject>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "viewermanager.h"

class VIEWERMANAGER_DLL Mgr : public QObject{
	Q_OBJECT
public:
	Mgr();
protected:
	std::shared_ptr<spdlog::logger> m_logger_dev;
	std::shared_ptr<spdlog::logger> m_logger_daily;
};

VIEWERMANAGER_DLL void registerLogger(std::shared_ptr<spdlog::logger> logger);