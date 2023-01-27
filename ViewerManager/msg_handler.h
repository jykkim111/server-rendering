#pragma once


#ifndef  WEBSOCKETPP_RENDERING_SERVER_HANDLER_HPP
#define WEBSOCKETPP_RENDERING_SERVER_HANDLER_HPP

class msg_handler : public server::handler {
	void on_message(connection_ptr con, std::string msg) {
		con->write(msg);
	}
};

#endif // ! WEBSOCKET_RENDERING_SERVER_HANDLER_HPP
