/*
  ==============================================================================

    TCPServerConnectionManager.cpp
    Created: 4 Jul 2019 11:34:50am
    Author:  bkupe

  ==============================================================================
*/

#include "TCPServerConnectionManager.h"

TCPServerConnectionManager::TCPServerConnectionManager() :
	Thread("TCP Server Connections"),
	portToBind(0)
{
}

TCPServerConnectionManager::~TCPServerConnectionManager()
{
	close();
}

void TCPServerConnectionManager::setupReceiver(int port)
{
	close();
	portToBind = port;
	startThread();
}

void TCPServerConnectionManager::removeConnection(StreamingSocket* connection)
{
	connections.removeObject(connection);
}

void TCPServerConnectionManager::close()
{
	if (receiver.isConnected()) receiver.close();
	signalThreadShouldExit();
	waitForThreadToExit(100);
}

void TCPServerConnectionManager::run()
{
	bool result = receiver.createListener(portToBind);
	connectionManagerListeners.call(&ConnectionManagerListener::receiverBindChanged, result);

	if (result)
	{
		while (!threadShouldExit())
		{
			StreamingSocket* socket = receiver.waitForNextConnection();
			if (socket != nullptr)
			{
				connections.add(socket);
				connectionManagerListeners.call(&ConnectionManagerListener::newConnection, socket);
			}
		}
	}
	else
	{
		LOGERROR("Could not bind to port " << portToBind);
	}

	receiver.close();
}