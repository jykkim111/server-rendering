#pragma once

// Custom logger
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>
#include <QDir>
#include <iostream>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
//#include <stdio.h>
//#include <WinSock2.h>
//#include <ws2tcpip.h>
#include <bitset>
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>
#include "jpge.h"
#include <jpeg8/jpeglib8.h>

#include "RenderingFunctionMgr.h"
#include "global.h"

#include <json.hpp>
using json = nlohmann::json;

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

QByteArray* encode(cgip::CgipPlane* plane, unsigned long* jpegSize, unsigned char** jpegBuf, int compression_rate);
void encode_jpeg_to_memory(unsigned char* image, int width, int height, int quality,
	const char* comment, unsigned long* jpegSize, unsigned char** jpegBuf);

typedef struct JobMgr {
	std::string rendering_type;
	std::string action_type;
	bool skip=false;
	QWebSocket* pClient;
	json render_data;
}JobMgr;

typedef std::shared_mutex Lock;
typedef std::unique_lock<Lock> writeLock;
typedef std::unique_lock<Lock> readLock;
static Lock PACSlock;


static std::map<std::string, std::queue<JobMgr>> JobMap;
static std::map<QWebSocket*, std::string> socket_id_map;


class RenderThread : public QObject {
	Q_OBJECT
	using QObject::QObject;

public:
	static RenderThread& instance() {
		static RenderThread m_instance;
		return m_instance;
	}
	static void run(std::string user_id);
	static void rendering(RenderingFunctionMgr* rendering_func_mgr, JobMgr);

Q_SIGNALS:
	void sigSendByteMessage(QWebSocket*, QByteArray);
	void sigSendStateMessage(QWebSocket*, QString, QString);
};




/** General semaphore with N permissions **/
class Semaphore {
	const size_t num_permissions;
	size_t avail;
	std::mutex m;
	std::condition_variable cv;
public:
	/** Default constructor. Default semaphore is a binary semaphore **/
	explicit Semaphore(const size_t& num_permissions = 1) : num_permissions(num_permissions), avail(num_permissions) { }

	/** Copy constructor. Does not copy state of mutex or condition variable,
		only the number of permissions and number of available permissions **/
	Semaphore(const Semaphore& s) : num_permissions(s.num_permissions), avail(s.avail) { }

	void acquire() {
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [this] { return avail > 0; });
		avail--;
		lk.unlock();
	}

	void release() {
		m.lock();
		avail++;
		m.unlock();
		cv.notify_one();
	}

	size_t available() const {
		return avail;
	}
};




class Rendering_server : public QObject
{
	Q_OBJECT

public:
	explicit Rendering_server(quint16 port, QObject* parent = nullptr);
	~Rendering_server();

	//static void run(std::string user_id);
	//static void rendering(RenderingFunctionMgr* rendering_func_mgr, JobMgr);
	void saveLocalPath(std::string root = "E:/", std::string save_dir="E:/", bool is_mask=false);
	
Q_SIGNALS:
	void closed();
	//void sigSendByteMessage(QWebSocket*, QByteArray);

public Q_SLOTS:
	void slotSendByteMessage(QWebSocket*, QByteArray);
	void slotSendStateMessage(QWebSocket*, QString, QString);

private Q_SLOTS:
	void onNewConnection();
	void processTextMessage(QString message);
	void processBinaryMessage(QByteArray message);
	void socketDisconnected();

private:
	QWebSocketServer* m_pWebSocketServer;
	QList<QWebSocket*> m_clients;
	bool m_debug;

	// Directory
	QDir m_dir;
	int file_num = 0;
	bool m_jpeg_on = true;
	json path_json;
	Semaphore access;

	void _saveLocalPath(QFileInfo, int parent, int num);

};
