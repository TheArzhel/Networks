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

	// New socket set
	fd_set readfs;
	FD_ZERO(&readfs);

	// Fill the set
	for (int i = 0; i < sockets.size(); i++)	
	{
		FD_SET(sockets[i], &readfs);
	}

	// Timeout (return immediately)
	timeval timeout;			
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// TODO(jesus): select those sockets that have a read operation available

	int ret = select(0, &readfs, nullptr, nullptr, &timeout);
	
	//Report Error
	if (ret == SOCKET_ERROR)
	{
		reportError("Module Networking. Failed Connection while loocking for available sockets");
	}

	// create a list with disconnected sockets
	std::list< SOCKET > disconnectedSockets;

	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).





	if (!sockets.empty())
	{
		for (auto s : sockets )
		{
			if (FD_ISSET(s, &readfs))
			{
				
				// check the selected socket to see if its a listener or a standard one.
				// if standar -> receive
				// if listener -> accept

				//in case of listener
				if (isListenSocket(s)) 
				{
					//create new socket
					SOCKET ListenerSocket = INVALID_SOCKET;
					//create data to accept socket
					sockaddr_in listenerAddress;
					int size = sizeof(listenerAddress);

					//accept
					ListenerSocket = accept(s, (sockaddr*)&listenerAddress, &size);

					// Error handle
					if (ListenerSocket == INVALID_SOCKET)
					{
						reportError("Module Networking. identify listener socket. failed to accept");
					}
					else
					{
						// add to list
						// if everything is accepted, use onSocketConnected(SOCKET, sockaddr_in)

						addSocket(ListenerSocket);
						onSocketConnected(ListenerSocket, listenerAddress);
					}
				}
				else // if standard socket
				{
					// receive information
					// recv​(​s​,​ inputBuffer​,​ inputBufferLen​,​​0​)
					ret = recv(s, (char*)incomingDataBuffer, incomingDataBufferSize, 0);
					
					
					// error handle
					if (ret == SOCKET_ERROR)
					{
						disconnectedSockets.push_back(s);
						reportError("Module Networking. identify standard socket. Failed top recieve data");
					}
					else
					{
						if (ret == 0 || ret == ECONNRESET)
						{
							// to handle disconections, if recv -> 0 or error ECONNRESET
							// use the callback onSocketDisconnected() to disconnect
							disconnectedSockets.push_back(s);
							
						}
						else // if everything work properly
						{
							// pass the recived data with onSocketReceivedData()

							// to set the end of the string
							//ask jesus to knwo if the problem is the incorrect incomingdatabuffersize
							incomingDataBuffer[ret] = '\0';
							onSocketReceivedData(s, incomingDataBuffer);

						}
					}
				}
			}
		}
	}

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.

	// iterate if the socket is ready to be disconnected
	if (!sockets.empty() && !disconnectedSockets.empty())
	{
		for (std::list<SOCKET>::iterator it = disconnectedSockets.begin(); it != disconnectedSockets.end(); it++)
		{
			int index = 0;
			for (auto s : sockets)
			{
				if (*it == s)
				{
					// delete and disconnect the correct socket
					onSocketDisconnected(s);
					sockets.erase(sockets.begin() + index );
				}

			index++;
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
