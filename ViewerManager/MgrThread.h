#pragma once

#include <QThread>
#include <QWebsocketServer>
#include <QWebsocket>

class MgrThread : public QThread
{
	Q_OBJECT
public:
	explicit MgrThread(int ID, QObject* parent = 0);
	void run();

signals:
	void error(QAbstractSocket::SocketError socketError);

public slots:
	void readyRead();
	void disconnected();

public slots:
	QWebSocket* socket;
	int socketDescriptor;


};

