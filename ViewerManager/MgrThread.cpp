#include "MgrThread.h"
#include <iostream>

MgrThread::MgrThread(int ID, QObject* parent) :
	QThread(parent)
{

	this->socketDescriptor = ID;
	

}

void MgrThread::run() {
	std::cout << "starting thread" << std::endl;

	socket = new QWebSocket();
}