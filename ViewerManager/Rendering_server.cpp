
#include "Rendering_server.h"

QT_USE_NAMESPACE

/**
*	Name:		Rendering_server
*	Params:
*		- port:	quint16
*			port number to listen from
*	Desc:
*		Constructor of Rendering_server
*/
Rendering_server::Rendering_server(quint16 port, QObject* parent) :
	QObject(parent),
	m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
		QWebSocketServer::NonSecureMode, this)),
	access(5)
{
	if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
		std::cout << "rendering server listening on port" << std::endl;
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
			this, &Rendering_server::onNewConnection);
		connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &Rendering_server::closed);
	}

}


/**
*	Name:		~Rendering_server
*	Params:		None
*	Desc:
*		Deconstructor of Rendering_server
*/
Rendering_server::~Rendering_server()
{
	m_pWebSocketServer->close();
	qDeleteAll(m_clients.begin(), m_clients.end());
}


/**
*	Name:		onNewConnection
*	Params:		None
*	Desc:
*		function that  connects websocket signals with rendering server slots on new client connection
*/
void Rendering_server::onNewConnection()
{
	QWebSocket* pSocket = m_pWebSocketServer->nextPendingConnection();

	connect(pSocket, &QWebSocket::textMessageReceived, this, &Rendering_server::processTextMessage);
	connect(pSocket, &QWebSocket::binaryMessageReceived, this, &Rendering_server::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &Rendering_server::socketDisconnected);

	m_clients << pSocket;
}
//! [onNewConnection]

//! [processTextMessage]
/**
*	Name:		processTextMessage
*	Params:
*			- message : QString
*				message received from client
*	Desc:
*		parses message from clients and uses the parsed information to assign rendering jobs
*		to the right client through the job map
*/
void Rendering_server::processTextMessage(QString message)
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());

	if (pClient) {
		std::string msg = message.toUtf8();
		auto render_data = json::parse(msg);

		std::string user_id = render_data["UserID"];
		std::string rendering_type = render_data["RenderingType"];
		std::string action = render_data["Action"];

		socket_id_map.insert(std::pair<QWebSocket*, std::string>(pClient, user_id));

		if (rendering_type == "Connection") {
			if (action == "disconnect") {
				access.acquire();
				JobMap.erase(user_id);
				access.release();
				return;
			}
		}

		JobMgr job_manager;
		job_manager.rendering_type = rendering_type;
		job_manager.action_type = action;
		job_manager.render_data = render_data;
		job_manager.pClient = pClient;

		{
			//std::lock_guard<std::mutex> lock(sMutex);
			access.acquire();
			//writeLock w_lock(PACSlock);
			if (JobMap.find(user_id) == JobMap.end()) {
				// user's first job
				std::queue<JobMgr> q;

				printf("new user: %s / ip: %s %d\n", user_id, pClient->localAddress().toString().toStdString().c_str(), pClient->localPort());
				writeLock w_lock(PACSlock);

				JobMap.insert(std::pair<std::string, std::queue<JobMgr>>(user_id, q));

				std::thread thread(&RenderThread::run, user_id);
				thread.detach();

				JobMap[user_id].push(job_manager);
			}
			else {
				if (rendering_type == "HARDRESET") {
					std::queue<JobMgr> empty;
					JobMap[user_id] = empty;
					JobMap[user_id].push(job_manager);
				}
				else {
					// insert queue
					JobMap[user_id].push(job_manager);
				}
				
			}
			access.release();
		}
	}
}
//! [processTextMessage]

void Rendering_server::slotSendByteMessage(QWebSocket* pClient, QByteArray json_msg) {
	pClient->sendBinaryMessage(json_msg);
}



void Rendering_server::slotSendStateMessage(QWebSocket* pClient, QString msg, QString state) {
	json post_render;

	if (state == "ERROR") {
		post_render["state"] = -1;
	}
	else if (state == "LOADING") {
		post_render["state"] = 1;
	}
	post_render["message"] = msg.toStdString();

	std::string render_info = post_render.dump();
	const char* buf = render_info.c_str();
	QByteArray json_msg(buf, render_info.size());
	QByteArray msg_type("s");
	json_msg.append(msg_type);

	try {
		pClient->sendBinaryMessage(json_msg);
	}
	catch (const std::exception& e) {
		std::cout << "SEND EXCEPTION::" << e.what() << std::endl;
	}
}

//! [processBinaryMessage]
void Rendering_server::processBinaryMessage(QByteArray message)
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());

	if (pClient) {
		try {
			pClient->sendBinaryMessage(message);
		}
		catch (const std::exception& e) {
			std::cout << "SEND EXCEPTION::" << e.what() << std::endl;
		}
	}
}
//! [processBinaryMessage]

//! [socketDisconnected]
/**
*	Name:		socketDisconnected
*	Params:		None
*	Desc:
*		removes the all connected clients due disconnected socket
*/
void Rendering_server::socketDisconnected()
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	if (pClient) {
		m_clients.removeAll(pClient);
		JobMap.erase(socket_id_map[pClient]);
		socket_id_map.erase(pClient);
		pClient->deleteLater();
	}
	std::cout << "disconnected...";
}
//! [socketDisconnected]

void RenderThread::run(std::string user_id) {
	printf("run %s thread\n", user_id.c_str());

	RenderingFunctionMgr* rendering_func_mgr = nullptr;

	while (true) {
		JobMgr job_manager;
		bool is_render = false;

		{
			//std::lock_guard<std::mutex> lock(sMutex);
			//readLock r_lock(PACSlock);
			if (JobMap.find(user_id) == JobMap.end()) {
				break;
			}

			if (!JobMap[user_id].empty()) {
				is_render = true;
			}

			if (is_render) {
				job_manager = JobMap[user_id].front();
				JobMap[user_id].pop();

				if (job_manager.rendering_type == "Connection") {
					rendering_func_mgr = new RenderingFunctionMgr();
				}
				else if (JobMap[user_id].size() > 0 && (job_manager.rendering_type == "MPR" || job_manager.rendering_type == "VR")) {
					JobMgr next_job_manager = JobMap[user_id].front();

					if (job_manager.action_type == next_job_manager.action_type) {
						job_manager.skip = true;
					}
				}
			}
		}

		if (is_render) {
			try {
				rendering(rendering_func_mgr, job_manager);
			}
			catch (const cgip::CgipOpenGLException& e) {
				emit instance().sigSendStateMessage(job_manager.pClient, QString(e.what()), "ERROR");
			}
			catch (const cgip::CgipOpenGLOOMException& e) {
				//std::cout << "OUT OF MEMORY::" << e.what() << std::endl;
				emit instance().sigSendStateMessage(job_manager.pClient, "GPU: Out of Memory", "ERROR");
			}
			catch (const cgip::CgipSlotNullException& e) {
				//std::cout << "OUT OF MEMORY::" << e.what() << std::endl;
				emit instance().sigSendStateMessage(job_manager.pClient, "CPU: Out of Memory", "ERROR");
			}
			catch (const std::exception& e) {
				//std::cout << "EXCEPTION::" << e.what() << std::endl;
				emit instance().sigSendStateMessage(job_manager.pClient, QString(e.what()), "ERROR");
			}
		}
	}

	printf("del %s thread\n", user_id.c_str());
	delete rendering_func_mgr;
}

void RenderThread::rendering(RenderingFunctionMgr* rendering_func_mgr, JobMgr job_manager) {
	json render_data = job_manager.render_data;
	bool skip = job_manager.skip;
	QWebSocket* pClient = job_manager.pClient;

	std::string user_id = render_data["UserID"];
	std::string rendering_type = render_data["RenderingType"];
	std::string action = render_data["Action"];

	int mpr_sampling_width = std::stoi(render_data["MPRSamplingRateWidth"].dump());
	int mpr_sampling_height = std::stoi(render_data["MPRSamplingRateHeight"].dump());
	int vr_sampling = std::stoi(render_data["VRSamplingRate"].dump());
	int compression_rate = std::stoi(render_data["CompressionRate"].dump());

	bool image_type = !render_data["ImageType"].dump().compare("true");

	bool is_dvr_downsampling = !render_data["isDVRDownsampling"].dump().compare("true");
	bool is_mpr_downsampling = !render_data["isMPRDownsampling"].dump().compare("true");


	json param = render_data["Params"];

	if (rendering_type == "Connection") {
		rendering_func_mgr->initSamplingCnt(mpr_sampling_width, mpr_sampling_height, vr_sampling);
	}
	else if (rendering_type == "Tree") {
		json post_render;
		json path;

		if (action == "data") {
			std::ifstream i("D:/dataset_path.json");
			i >> path;
		}
		else if (action == "mask") {
			std::ifstream i("D:/mask_path.json");
			i >> path;
		}

		post_render["tree"] = path;

		std::string render_info = post_render.dump();
		const char* buf = render_info.c_str();
		QByteArray json_msg(buf, render_info.size());

		if (action == "data") {
			QByteArray msg_type("t");
			json_msg.append(msg_type);
		}
		else if (action == "mask") {
			QByteArray msg_type("m");
			json_msg.append(msg_type);
		}

		emit instance().sigSendByteMessage(pClient, json_msg);
		//slotSendByteMessage(pClient, json_msg);
	}
	else if (rendering_type == "MPR" || rendering_type == "VR" || rendering_type == "CPR" || rendering_type == "HARDRESET") {
		if (mpr_sampling_width != rendering_func_mgr->getMPRSamplingCnt()) {
			rendering_func_mgr->setMPRSamplingCnt(mpr_sampling_width, mpr_sampling_height, is_mpr_downsampling);
			skip = false;
		}

		if (vr_sampling != rendering_func_mgr->getVRSamplingCnt()) {
			rendering_func_mgr->setVRSamplingCnt(vr_sampling, is_dvr_downsampling);
			skip = false;
		}

		if (rendering_type == "MPR" || rendering_type == "HARDRESET") {
			if (action == "load_data") {
				emit instance().sigSendStateMessage(pClient, "loading data", "LOADING");
				rendering_func_mgr->action_manager(MPR, LOAD_DATA, param);
			}
			else if (action == "load_mask") {
				emit instance().sigSendStateMessage(pClient, "loading mask", "LOADING");
				rendering_func_mgr->action_manager(MPR, LOAD_MASK, param);
			}
			else if (action == "save_mask")
				rendering_func_mgr->action_manager(MPR, SAVE_MASK, param);
			else if (action == "data")
				rendering_func_mgr->action_manager(MPR, DATA, param);
			else if (action == "zoom")
				rendering_func_mgr->action_manager(MPR, ZOOM, param, skip);
			else if (action == "pan")
				rendering_func_mgr->action_manager(MPR, PAN, param, skip);
			else if (action == "window")
				rendering_func_mgr->action_manager(MPR, WINDOW, param, skip);
			else if (action == "center")
				rendering_func_mgr->action_manager(MPR, CENTER, param);
			else if (action == "reset")
				rendering_func_mgr->action_manager(MPR, RESET, param);
			else if (action == "inverse")
				rendering_func_mgr->action_manager(MPR, INVERSE, param);
			else if (action == "move_slice")
				rendering_func_mgr->action_manager(MPR, MOVE_SLICE, param, skip);
			else if (action == "guideline_move")
				rendering_func_mgr->action_manager(MPR, GUIDELINE_MOVED, param, skip);
			else if (action == "oblique")
				rendering_func_mgr->action_manager(MPR, OBLIQUE, param, skip);
			else if (action == "thickness")
				rendering_func_mgr->action_manager(MPR, THICKNESS, param, skip);
			else if (action == "thickness_mode")
				rendering_func_mgr->action_manager(MPR, THICKNESS_MODE, param, skip);
			else if (action == "mask_visible")
				rendering_func_mgr->action_manager(MPR, MASK_VISIBLE, param);
			else if (action == "mask_opacity")
				rendering_func_mgr->action_manager(MPR, MASK_OPACITY, param);
			else if (action == "mask_color")
				rendering_func_mgr->action_manager(MPR, MASK_COLOR, param);
		}
		if (rendering_type == "VR") {
			if (action == "load_data") {
				emit instance().sigSendStateMessage(pClient, "loading data", "LOADING");
				rendering_func_mgr->action_manager(VR, LOAD_DATA, param);
			}
			else if (action == "data")
				rendering_func_mgr->action_manager(VR, DATA, param);
			else if (action == "pan")
				rendering_func_mgr->action_manager(VR, PAN, param, skip);
			else if (action == "window")
				rendering_func_mgr->action_manager(VR, WINDOW, param);
			else if (action == "rotate")
				rendering_func_mgr->action_manager(VR, ROTATE, param, skip);
			else if (action == "reset")
				rendering_func_mgr->action_manager(VR, RESET, param);
			else if (action == "zoom")
				rendering_func_mgr->action_manager(VR, ZOOM, param, skip);
			else if (action == "sculpt")
				rendering_func_mgr->action_manager(VR, SCULPT, param);
			else if (action == "voi")
				rendering_func_mgr->action_manager(VR, VOI, param);
			else if (action == "reset_sculpt")
				rendering_func_mgr->action_manager(VR, RESET_SCULPT, param);
			else if (action == "orientation")
				rendering_func_mgr->action_manager(VR, ORIENTATION, param);
			else if (action == "projection")
				rendering_func_mgr->action_manager(VR, PROJECTION, param, skip);
			else if (action == "settings")
				rendering_func_mgr->action_manager(VR, SETTINGS, param, skip);

		}
		if (rendering_type == "CPR") {
			if (action == "init") {
				rendering_func_mgr->action_manager(CPR, INIT, param);
			}

			if (action == "curve") {
				rendering_func_mgr->action_manager(CPR, CURVE, param);
			}

			if (action == "path") {
				rendering_func_mgr->action_manager(CPR, PATH, param);
			}
		}

		if (!skip) {
			PacketUpdateInfo info = rendering_func_mgr->getPacketUpdateInfo();

			if (info.error) {
				json error_json;
				std::string error_info = rendering_func_mgr->getErrorMessage();
				const char* buf = error_info.c_str();
				QByteArray json_msg(buf, error_info.size());
				QByteArray msg_type("e");
				json_msg.append(msg_type);

				emit instance().sigSendByteMessage(pClient, json_msg);
			}
			else {
				json post_render;

				post_render["volume"] = rendering_func_mgr->getVolumeInfobyJson();
				post_render["axial"]["features"] = rendering_func_mgr->getMPRInfobyJson(AXIAL_SLICE);
				post_render["sagittal"]["features"] = rendering_func_mgr->getMPRInfobyJson(SAGITTAL_SLICE);
				post_render["coronal"]["features"] = rendering_func_mgr->getMPRInfobyJson(CORONAL_SLICE);
				post_render["dvr"]["features"] = rendering_func_mgr->getVRInfobyJson();
				post_render["mask"] = rendering_func_mgr->getMaskInfobyJson();
				post_render["update_info"] = rendering_func_mgr->getPacketUpdateInfobyJson();

				std::string render_info = post_render.dump();
				const char* buf = render_info.c_str();
				QByteArray json_msg(buf, render_info.size());
				QByteArray msg_type("h");
				json_msg.append(msg_type);

				//slotSendByteMessage(pClient, json_msg);
				emit instance().sigSendByteMessage(pClient, json_msg);

				unsigned char** jpegBuf = NULL;
				unsigned long* jpegSize = 0;

				if (info.axial_image) {
					if (image_type) {
						QByteArray* axial_data = encode(&rendering_func_mgr->getMPRImage(AXIAL_SLICE), jpegSize, jpegBuf, compression_rate);
						QByteArray binary_msg(axial_data->data(), axial_data->size());
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("ah");
						binary_msg.append(msg_type);
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getMPRImage(AXIAL_SLICE);
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("ah");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
				}
				if (info.sagittal_image) {
					if (image_type) {
						rendering_func_mgr->write_jpeg_file(&rendering_func_mgr->getMPRImage(SAGITTAL_SLICE), "sagittal_test.jpeg", 95);
						QByteArray* sagittal_data = encode(&rendering_func_mgr->getMPRImage(SAGITTAL_SLICE), jpegSize, jpegBuf, compression_rate);
						QByteArray binary_msg(sagittal_data->data(), sagittal_data->size());
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("sh");
						binary_msg.append(msg_type);
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getMPRImage(SAGITTAL_SLICE);
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}

						QByteArray msg_type("sh");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
				}
				if (info.coronal_image) {
					if (image_type) {
						QByteArray* coronal_data = encode(&rendering_func_mgr->getMPRImage(CORONAL_SLICE), jpegSize, jpegBuf, compression_rate);
						//post_render["coronal"]["img"] = coronal_data;
						QByteArray binary_msg(coronal_data->data(), coronal_data->size());
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("ch");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getMPRImage(CORONAL_SLICE);
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("ch");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}

				}
				if (info.cpr_image) {
					if (image_type) {
						QByteArray* cpr_data = encode(&rendering_func_mgr->getMPRImage(rendering_func_mgr->getCurrentCPRMode()), jpegSize, jpegBuf, compression_rate);
						QByteArray binary_msg(cpr_data->data(), cpr_data->size());
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("ph");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getMPRImage(rendering_func_mgr->getCurrentCPRMode());
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("ph");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}

				}
				if (info.cpr_curve_image) {
					if (image_type) {
						QByteArray* cpr_curve_data = encode(&rendering_func_mgr->getCPRImage(), jpegSize, jpegBuf, compression_rate);
						QByteArray binary_msg(cpr_curve_data->data(), cpr_curve_data->size());
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("vh");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getCPRImage();
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("vh");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}

				}

				if (info.cpr_path_image) {
					if (image_type) {
						QByteArray* cpr_path_data = encode(&rendering_func_mgr->getCPRPathImage(), jpegSize, jpegBuf, compression_rate);
						QByteArray binary_msg(cpr_path_data->data(), cpr_path_data->size());
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("th");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getCPRPathImage();
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (mpr_sampling_width == 1024) {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(mpr_sampling_width).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("th");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}

				}


				if (info.dvr) {
					if (image_type) {
						QByteArray* dvr_data = encode(&rendering_func_mgr->getVRImage(), jpegSize, jpegBuf, compression_rate);
						//post_render["dvr"]["img"] = dvr_data;
						QByteArray binary_msg(dvr_data->data(), dvr_data->size());
						if (vr_sampling == 1024) {
							QByteArray resolution_char(std::to_string(vr_sampling).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(vr_sampling).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}

						QByteArray msg_type("dh");
						binary_msg.append(msg_type);;
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}
					else {
						cgip::CgipPlane plane = rendering_func_mgr->getVRImage();
						int length = plane.getWidth() * plane.getHeight() * plane.getChannel();
						std::vector<CgipShort> data = plane.getPlane();
						unsigned char* image_buffer = (unsigned char*)malloc(length);

#pragma omp parallel for
						for (int i = 0; i < length; i++) {
							image_buffer[i] = data[i];
						}
						const char* img = reinterpret_cast<const char*>(image_buffer);
						QByteArray binary_msg(img, length);
						if (vr_sampling == 1024) {
							QByteArray resolution_char(std::to_string(vr_sampling).c_str());
							binary_msg.append(resolution_char);
						}
						else {
							QByteArray resolution_char(std::to_string(vr_sampling).c_str());
							binary_msg.append("0");
							binary_msg.append(resolution_char);
						}
						QByteArray msg_type("dh");
						binary_msg.append(msg_type);
						emit instance().sigSendByteMessage(pClient, binary_msg);
					}

				}

				free(jpegBuf);
			}

		}
	}
}


void encode_jpeg_to_memory(unsigned char* image, int width, int height, int quality,
	const char* comment, unsigned long* jpegSize, unsigned char** jpegBuf) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);
	cinfo.image_width = width;
	cinfo.image_height = height;


	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	// libjpeg-turbo will alloc buffer and size.
	jpeg_mem_dest(&cinfo, jpegBuf, jpegSize);

	jpeg_start_compress(&cinfo, TRUE);

	// Add comment section if any..
	if (comment) {
		jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET*)comment, strlen(comment));
	}

	row_stride = width * cinfo.input_components;

	// Encode
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &image[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
}

/*
encode function changes CgipPlane into a QByteArray so that it can be sent over the socket
*/
QByteArray* encode(cgip::CgipPlane* plane, unsigned long* jpegSize, unsigned char** jpegBuf, int compression_rate) {
	//printf("plane size: %d, %d, %d", plane->getWidth(), plane->getHeight(), plane->getChannel());
	int length = plane->getWidth() * plane->getHeight() * plane->getChannel();
	std::vector<CgipShort> data = plane->getPlane();
	unsigned char* image_buffer = (unsigned char*)malloc(length);


#pragma omp parallel for
	for (int i = 0; i < length; i++) {
		image_buffer[i] = data[i];
	}

	unsigned long jSize = 0;
	unsigned char* jBuf = NULL;
	encode_jpeg_to_memory(image_buffer, plane->getWidth(), plane->getHeight(), compression_rate, "comment", &jSize, &jBuf);

	//printf("JPEG size (bytes): %ld", jSize);

	int bufSize = static_cast<int>(jSize);
	const char* img = reinterpret_cast<const char*>(jBuf);
	QByteArray* img_data = new (std::nothrow) QByteArray(img, bufSize);
	if (img_data == NULL) {
		throw cgip::CgipSlotNullException();
	}

	free(jBuf);
	free(image_buffer);

	return img_data;
}

void Rendering_server::saveLocalPath(std::string root, std::string save_dir, bool is_mask) {

	QStringList filters;

	if (is_mask) {
		filters << "*.dat";
	}
	else {
		//filters << "*.raw";
		filters << "*.dcm";
		filters << "*.vol";
	}

	m_dir.setNameFilters(filters);
	m_dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
	//dir.setSorting(QDir::Size | QDir::Reversed);

	m_dir.setPath(QString(root.c_str()));

	path_json[std::to_string(0)][std::to_string(-1)] =
	{ m_dir.absolutePath().toStdString(),
	m_dir.dirName().toStdString(),
	"false" };

	QFileInfoList list = m_dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		_saveLocalPath(fileInfo, 0, ++file_num);
	}

	printf("dir: %s\n", save_dir.c_str());

	std::ofstream  o(save_dir + "path.json");
	o << std::setw(4) << path_json << std::endl;
}

void Rendering_server::_saveLocalPath(QFileInfo fileInfo, int parent, int num) {

	std::string is_file = "false";

	if (fileInfo.isFile()) {
		is_file = "true";
	}

	path_json[std::to_string(parent)][std::to_string(num)] = { fileInfo.absoluteFilePath().toStdString(),
																fileInfo.fileName().toStdString(),
																is_file };
	//fileInfo.fileName().toStdString(); //fileInfo.absoluteFilePath().toStdString();

	//std::cout << num  << qPrintable(QString(": %1").arg(fileInfo.absoluteFilePath())) << std::endl;

	if (fileInfo.isFile()) return;

	m_dir.setPath(fileInfo.absoluteFilePath());
	QFileInfoList list = m_dir.entryInfoList();

	for (int i = 0; i < list.size(); ++i) {
		QFileInfo temp_fileInfo = list.at(i);

		path_json[std::to_string(num)][std::to_string(-1)] = { fileInfo.absoluteFilePath().toStdString(),
																fileInfo.fileName().toStdString(),
																is_file };
		_saveLocalPath(temp_fileInfo.absoluteFilePath(), num, ++file_num);
	}
}
