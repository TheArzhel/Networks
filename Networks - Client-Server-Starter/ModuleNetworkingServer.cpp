#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	int ret = 0;
	// TODO(jesus): TCP listen socket stuff
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);   // Create TCP socket
	if (listenSocket == INVALID_SOCKET)
	{
		reportError("Fail to Create the Server Socket");
	}

	// before invoking ​bind​, the socket configured to force reusing the given 
	// address/portbefore calling the function ​bind
	int enable = 1;
	ret = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (ret == SOCKET_ERROR)
	{
		reportError("Given Addres Failure before invoking the Bind in Server");
	}

	// -To represent addresses the lib uses struct sockaddr
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; //ipv4
	bindAddr.sin_port = htons(port); // port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // local ip adress

	// int​ bind​(SOCKET s​, const​​struct​ sockaddr ​*​ address​, int​ address_len​);
	ret = bind(listenSocket, (const sockaddr*)&bindAddr, sizeof(bindAddr));
	if (ret == SOCKET_ERROR)
	{
		reportError("Fail to Bind Server Socket");
	}

	// Enter in listen mode
	// Ask Jesus the effects
	ret = listen(listenSocket, 10);
	if (ret == SOCKET_ERROR)
	{
		reportError("Server fails to Listen");
	}
	else
	{
		// Add Socket to the list 
		// Cuidado
		addSocket(listenSocket);

		state = ServerState::Listening;
	}

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		if (ImGui::Button("Close Server"))
		{
			disconnect();
			state = ServerState::Stopped;
		}

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, byte * data)
{
	// Set the player name of the corresponding connected socket proxy
	for (auto &connectedSocket : connectedSockets)
	{
		if (connectedSocket.socket == socket)
		{
			connectedSocket.playerName = (const char *)data;
		}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

