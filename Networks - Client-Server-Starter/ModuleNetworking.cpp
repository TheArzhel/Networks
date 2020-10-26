#include "Networks.h"
#include "ModuleNetworking.h"


static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

//implement the socket selector
bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()

	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	fd_set readfs;								// New socket set
	FD_ZERO(&readfs);

	for (int i = 0; i < sockets.size(); i++)	// Fill the set
	{
		FD_SET(sockets[i], &readfs);
	}

	timeval timeout;							// Timeout (return immediately)
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// TODO(jesus): select those sockets that have a read operation available

	int ret = select(0, &readfs, nullptr, nullptr, &timeout);

	if (ret == SOCKET_ERROR)
	{
		reportError("Connection Error Looking for available sockets");
	}

	// Fill this array with disconnected sockets
	std::list< SOCKET > disconnectedSockets;

	if (sockets.size() > 0)
	{
		for (int i = 0; i < sockets.size(); i++)
		{
			if (FD_ISSET(sockets[i], &readfs))
			{
				// TODO(jesus): for those sockets selected, check wheter or not they are
				// a listen socket or a standard socket and perform the corresponding
				// operation (accept() an incoming connection or recv() incoming data,
				// respectively).

				if (isListenSocket(sockets[i])) //Is a server socket
				{
					SOCKET newSocket = INVALID_SOCKET;
					sockaddr_in newAddr;
					int size = sizeof(newAddr);

					newSocket = accept(sockets[i], (sockaddr*)&newAddr, &size);
					if (newSocket == INVALID_SOCKET)
					{
						reportError("Connection Error Accepting Socket");
					}
					else
					{
						// On accept() success, communicate the new connected socket to the
						// subclass (use the callback onSocketConnected()), and add the new
						// connected socket to the managed list of sockets.

						onSocketConnected(newSocket, newAddr);
						addSocket(newSocket);
					}
				}
				else
				{
					ret = recv(sockets[i], (char*)incomingDataBuffer, incomingDataBufferSize, 0);

					if (ret == SOCKET_ERROR)
					{
						reportError("Connection Error Receiving the Information");
						disconnectedSockets.push_back(sockets[i]);
					}
					else
					{
						if (ret == 0 || ret == ECONNRESET)
						{
							// TODO(jesus): handle disconnections. Remember that a socket has been
							// disconnected from its remote end either when recv() returned 0,
							// or when it generated some errors such as ECONNRESET.
							// Communicate detected disconnections to the subclass using the callback
							// onSocketDisconnected().
							disconnectedSockets.push_back(sockets[i]);
						}
						else
						{
							// On recv() success, communicate the incoming data received to the
							// subclass (use the callback onSocketReceivedData()).
							incomingDataBuffer[ret] = '\0';
							onSocketReceivedData(sockets[i], incomingDataBuffer);
						}

					}
				}
			}
		}
	}

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.

	for (std::list<SOCKET>::iterator it = disconnectedSockets.begin(); it != disconnectedSockets.end(); it++)
	{
		if (sockets.size() > 0)
		{
			for (int i = 0; i < sockets.size(); i++)
			{
				if (*it == sockets[i])
				{
					onSocketDisconnected(sockets[i]);
					sockets.erase(sockets.begin() + i);
				}
			}
		}
	}

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
