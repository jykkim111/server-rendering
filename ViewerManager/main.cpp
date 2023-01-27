
#include <string>
#include <iostream>
#include <vector>
#include <json.hpp>
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>
#include "RenderingFunctionMgr.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include "Rendering_server.h"





int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	int port = 8081;

	Rendering_server* server = new Rendering_server(port);

	//server->saveLocalPath("D:/mask", "D:/mask_", true);
	//server->saveLocalPath("D:/nia_dataset", "D:/dataset_", false);

	QObject::connect(&RenderThread::instance(), &RenderThread::sigSendByteMessage, server, &Rendering_server::slotSendByteMessage);
	QObject::connect(&RenderThread::instance(), &RenderThread::sigSendStateMessage, server, &Rendering_server::slotSendStateMessage);
	QObject::connect(server, &Rendering_server::closed, &a, &QCoreApplication::quit);

	return a.exec();
}






/*
int main() {
	// Create a server endpoint
	server echo_server;

	try {
		// Set logging settings
		echo_server.set_access_channels(websocketpp::log::alevel::all);
		echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

		// Initialize Asio
		echo_server.init_asio();

		// Register our message handler
		echo_server.set_message_handler(bind(&on_message, &echo_server, ::_1, ::_2));

		// Listen on port 9002
		echo_server.listen(8081);
		std::cout << "Now listening..." << std::endl;

		// Start the server accept loop
		echo_server.start_accept();
		

		// Start the ASIO io_service run loop
		echo_server.run();
	}
	catch (websocketpp::exception const& e) {
		std::cout << e.what() << std::endl;
	}
	//catch (...) {
	//	std::cout << "other exception" << std::endl;
	//}
}
*/

/*
int main() {
	std::map<std::string, RenderingFunctionMgr*> RenderingMap;


	WSADATA wsaData;
	int iniResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iniResult != 0)
	{
		std::cerr << "Can't Initialize winsock! Quitiing" << std::endl;
		return -1;
	}


	SOCKET hListen;
	hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (hListen == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket! Quitting" << std::endl;
		WSACleanup();
		return -1;
	}

	int option = 1;
	setsockopt(hListen, IPPROTO_TCP, SO_REUSEADDR, (const char*)&option, sizeof(option));


	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);



	int bindResult = bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	if (bindResult == SOCKET_ERROR)
	{
		std::cerr << "Can't bind a socket! Quitting" << std::endl;
		closesocket(hListen);
		WSACleanup();
		return -1;
	}


	int listenResult = listen(hListen, SOMAXCONN);

	if (listenResult == SOCKET_ERROR)
	{
		std::cout << "socket_error code: " << listenResult;
		std::cerr << "Can't listen a socket! Quitting" << std::endl;
		closesocket(hListen);
		WSACleanup();
		return -1;
	}
	else {
		std::cout << "Rendering Manager now listening ...";
	}



	SOCKADDR_IN tClntAddr = {};

	int iClntSize = sizeof(tClntAddr);
	char addBuffer[INET_ADDRSTRLEN];
	while (1) {
		SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize);
		//cout << "client address" << inet_ntop(AF_INET, &tClntAddr.sin_addr, addBuffer, sizeof(addBuffer));
		if (hClient == INVALID_SOCKET)
		{

			std::cerr << "Can't accept a socket! Quitting" << std::endl;
			closesocket(hListen);
			WSACleanup();
			return -1;
		}
		else {
			std::cout << "connection accepted\n";
		}



		while (true) {
			std::vector<char> buffer(PACKET_SIZE);
			std::string data;
			int byte_received;
			byte_received = recv(hClient, buffer.data(), PACKET_SIZE, 0);
			if (byte_received > 0) {
				std::cout << "Recv Msg: " << buffer.data() << "\n";
				data.append(buffer.data(), byte_received);
				std::cout << "string data: " << data << std::endl;


				//DO THE RENDERING AND THE UPDATING AND WHATNOT HERE
				auto render_data = json::parse(data);

				std::string user_id = render_data["UserId"];
				std::string rendering_type = render_data["RenderingType"];
				std::string action = render_data["Action"];
				std::string dir = render_data["Directory"];
				std::string image_type = render_data["ImageType"];
				json param = render_data["Param"];

				PacketUpdateInfo info;

				if ((RenderingMap.find(user_id) == RenderingMap.end()) && !(action == "load_data")) {
					// not found
					printf("not found user %s (%s)\n", user_id.c_str(), action.c_str());
					break;
				}
				else if ((RenderingMap.find(user_id) == RenderingMap.end())) {
					// new user
					printf("new user!\n");
					RenderingFunctionMgr* rendering_func_mgr = new RenderingFunctionMgr();
					rendering_func_mgr->setDir(dir);
					rendering_func_mgr->setImageExt(image_type);
					info = rendering_func_mgr->action_manager(LOAD_DATA, param);

					RenderingMap.insert(std::pair<std::string, RenderingFunctionMgr*>(user_id, rendering_func_mgr));
					//RenderingMap[user_id] = rendering_func_mgr;
					printf("user now joined\n");
				}
				else {
					RenderingFunctionMgr* rendering_func_mgr = RenderingMap[user_id];
					rendering_func_mgr->setDir(dir);
					rendering_func_mgr->setImageExt(image_type);

					printf("rendering...\n");

					if (action == "load_data")
						info = rendering_func_mgr->action_manager(LOAD_DATA, param);
					else if (action == "zoom")
						info = rendering_func_mgr->action_manager(ZOOM, param);
					else if (action == "pan")
						info = rendering_func_mgr->action_manager(PAN, param);
					else if (action == "window")
						info = rendering_func_mgr->action_manager(WINDOW, param);
					else if (action == "center")
						info = rendering_func_mgr->action_manager(CENTER, param);
					else if (action == "reset")
						info = rendering_func_mgr->action_manager(RESET, param);
					else if (action == "move_slice")
						info = rendering_func_mgr->action_manager(MOVE_SLICE, param);
					else if (action == "guideline_move")
						info = rendering_func_mgr->action_manager(GUIDELINE_MOVED, param);
					else if (action == "oblique")
						info = rendering_func_mgr->action_manager(OBLIQUE, param);
				}

				for (auto it = RenderingMap.begin(); it != RenderingMap.end(); it++) {
					std::cout << "key: " << it->first << '\n';
				}

				//Directory
				//std::string load_dir = render_data["Directory"].dump();
				//std::cout << "testing dumping json directory" << load_dir << std::endl;


				json post_render;

				post_render["RenderingType"] = rendering_type;
				post_render["action"] = action;
				post_render["status"] = info.update_info;


				std::string send_data;
				send_data = post_render.dump();
				std::cout << "sent message: " << send_data << std::endl;
				send(hClient, send_data.c_str(), strlen(send_data.c_str()), 0);

			}
			if (byte_received == -1) {
				std::cout << "error: " << WSAGetLastError() << std::endl;
				break;
			}

			if (byte_received == 0) {
				std::cout << "client disconnected\n";
				break;
			}


		}
		closesocket(hClient);

	}
	closesocket(hListen);

	WSACleanup();
	return 0;
}
*/
/*
static void show_usage(std::string name) {
	std::cerr << "Usage: " << name << " <option(s)> SOURCES"
		<< "Options:\n"
		<< "\t-id,--help\t\tShow this help message\n"
		<< "\t-render_type, --destination DESTINATION\tSpecify the destination path\n"
		<< "\t-Action, --destination DESTINATION\tSpecify the destination path\n"
		<< "\t-Directory, --destination DESTINATION\tSpecify the destination path\n"
		<< "\t-Image_Type, --destination DESTINATION\tSpecify the destination path\n"
		<< "\t-Parameters, --destination DESTINATION\tSpecify the destination path\n"
		<< std::endl;
}
int main(int argc, char* argv[]) {
	if (argc < 6) { // We expect 6 arguments: ViewManager.exe, userID, renderingType, action, directory, ImageType, and lastly the parameters
		show_usage(argv[0]);
		return 1;
	}
	std::vector <std::string> args;
	std::vector	<std::string> params;
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (i > 5) {
			params.push_back(arg);
		}
		else {
			args.push_back(arg);
		}
	}
	std::string user_id = render_data["UserId"];
	std::string rendering_type = render_data["RenderingType"];
	std::string action = render_data["Action"];
	std::string dir = render_data["Directory"];
	std::string image_type = render_data["ImageType"];
	json param = render_data["Param"];
	std::cout << "just checking what comes out";
	return 0;
}
*/