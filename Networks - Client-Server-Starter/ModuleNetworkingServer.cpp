#include "ModuleNetworkingServer.h"


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	int ret = 0;
	// TODO(jesus): TCP listen socket stuff
	// Create TCP socket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);   

	//Report Error
	if (listenSocket == INVALID_SOCKET)
	{
		reportError("Fail to Create the Server Socket");
	}

	// before invoking ​bind​, the socket configured to force reusing the given 
	// address/portbefore calling the function ​bind
	int enable = 1;
	ret = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	//Report Error
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
	//Report Error
	if (ret == SOCKET_ERROR)
	{
		reportError("Fail to Bind Server Socket");
	}

	// Enter in listen mode
	// Ask Jesus the effects
	ret = listen(listenSocket, 10);
	//Report Error
	if (ret == SOCKET_ERROR)
	{
		reportError("Server fails to Listen");
	}
	else
	{
		// Add Socket to the list 
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

		if (ImGui::Button("Shut Death Star Server"))
		{
			disconnect();
			connectedSockets.clear();
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

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet)
{
	//// Set the player name of the corresponding connected socket proxy
	//for (auto &connectedSocket : connectedSockets)
	//{
	//	if (connectedSocket.socket == socket)
	//	{
	//		connectedSocket.playerName = (const char *)data;
	//	}
	//}

	// updated OK

	ClientMessage clientMessage;
	packet >> clientMessage;

	// create state machine
	//act depending on the client message
	if (clientMessage == ClientMessage::Hello)
	{
		std::string playername;
		std::vector<float> color;
	
		packet >> playername;
		packet >> color;

		bool notify = false;

		for (int i = 0; i < connectedSockets.size(); ++i)
		{
			if (connectedSockets[i].socket == socket)
			{
				OutputMemoryStream _packet;
				std::string welcom_message;

				if (CheckName(playername))
				{
					_packet << ServerMessage::Welcome;
					connectedSockets[i].playerName = playername;
					connectedSockets[i].playerColor = color;
					welcom_message = "Welcome To The Death Star server!";
					notify = true;
				}
				else
				{
					_packet << ServerMessage::Unwelcome;
					connectedSockets[i].playerName = playername;
					welcom_message = "Traitor! Identify yourself, Your name is already used!";
				}

				_packet << welcom_message;

				int ret = sendPacket(_packet, socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("Welcome package commette not sed. ERROR");
				}
			}
		}
		if (notify)
		{
			for (int i = 0; i < connectedSockets.size(); ++i)
			{
				if (connectedSockets[i].socket != socket)
				{
					OutputMemoryStream _packet;
					_packet << ServerMessage::Newuser;

					std::string newuser_message = playername + " joined the dark side of the force";
					_packet << newuser_message;

					int ret = sendPacket(_packet, connectedSockets[i].socket);
					if (ret == SOCKET_ERROR)
					{
						reportError("Welcome package commette not sed. ERROR");
					}
				}
			}
		}
	}
	if (clientMessage == ClientMessage::Send)
	{
		std::string playername;
		std::vector <float> textcolor;
		bool blocked = false;
		for (int i = 0; i < connectedSockets.size(); ++i)
		{
			if (connectedSockets[i].socket == socket)
			{
				playername = connectedSockets[i].playerName;
				textcolor = connectedSockets[i].playerColor;
				if (connectedSockets[i].blocked)
				{
					blocked = true;
				}
			}
		}

		std::string message;
		packet >> message;
		packet >> textcolor;

		OutputMemoryStream _packet;
		_packet << ServerMessage::Newmessage;

		std::string text = playername + ": " + message;
		_packet << text;
		_packet << textcolor;

		for (int i = 0; i < connectedSockets.size(); ++i)
		{
			if (!blocked)
			{
				int ret = sendPacket(_packet, connectedSockets[i].socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("Welcome package commette not sed. ERROR");
				}
			}
		}
	}

	if (clientMessage == ClientMessage::Command)
	{
		for (int i = 0; i < connectedSockets.size(); ++i)
		{
			if (connectedSockets[i].socket == socket)
			{
				std::string command;
				packet >> command;

				CommandToExecute(command, connectedSockets[i].socket);
			}
		}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	//updated OK
	std::string playername;

	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			playername = connectedSocket.playerName;
			connectedSockets.erase(it);
			break;
		}
	}

	OutputMemoryStream _packet;
	_packet << ServerMessage::Userdisconnected;

	std::string text = playername + " left The battle. May the Force be with you";
	_packet << text;

	for (auto s : connectedSockets) 
	{
		int ret = sendPacket(_packet, s.socket);

		if (ret == SOCKET_ERROR)
		{
			reportError("Welcome package commette not sed. ERROR");
		}
	}
}

//from now on all new

bool ModuleNetworkingServer::CheckName(std::string name)
{

	for (auto s : connectedSockets)
	{
		if (name == s.playerName)
		{
			return false;
		}
	}
	return true;
}

void ModuleNetworkingServer::CommandToExecute(std::string command, SOCKET socket)
{
	if (command.find("help") != std::string::npos)
	{
		OutputMemoryStream _packet;
		_packet << ServerMessage::Command;

		std::string newuser_message = "Command list:\n /help \n /list \n /kick [name] \n /whisper [name] [message] \n /change_name [name] \n /remove_admin \n /add_admin \n /clear chat \n /add_block [username] \n /remove_block [username]";
		_packet << newuser_message;

		int ret = sendPacket(_packet, socket);
		if (ret == SOCKET_ERROR)
		{
			reportError("Welcome package commette not sed. ERROR");
		}
	}
	else if (command.find("list") != std::string::npos)
	{
		OutputMemoryStream _packet;
		_packet << ServerMessage::Command;

		std::string newuser_message = "Users connected: \n";

		for (int i = 0; i < connectedSockets.size(); i++)
		{
			newuser_message = newuser_message + "- " + connectedSockets[i].playerName;
			if (i < connectedSockets.size() - 1)
			{
				newuser_message = newuser_message + "\n";
			}
		}

		_packet << newuser_message;

		int ret = sendPacket(_packet, socket);
		if (ret == SOCKET_ERROR)
		{
			reportError("Welcome package commette not sed. ERROR");
		}
	}
	else if (command.find("kick") != std::string::npos)
	{
		bool found = false;
		bool admin = false;
		//look to see if userkicking  is admin
		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (connectedSockets[i].socket == socket)
			{
				if (connectedSockets[i].admin)
					admin = true;
			}

		}
		//look for player
		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (command.find(connectedSockets[i].playerName) != std::string::npos && admin)
			{
				OutputMemoryStream _packet;
				_packet << ServerMessage::ComDisconnect;

				int ret = sendPacket(_packet, connectedSockets[i].socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("Welcome package commette not sed. ERROR");
				}
				found = true;
			}
			if (command.find(connectedSockets[i].playerName) != std::string::npos && !admin)
			{
				OutputMemoryStream _packet;
				_packet << ServerMessage::Command;

				std::string newuser_message = "Admin priviledge needed";

				_packet << newuser_message;

				int ret = sendPacket(_packet, socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("kick without admin. ERROR");
				}
				found = true;
			}
			
		}
		if (!found)
		{
			std::string error = "user not found. ERROR";
			reportError("user not found. ERROR"); // server message
			OutputMemoryStream _packet;
			_packet << ServerMessage::Newmessage;
			_packet << error;

			int ret = sendPacket(_packet, socket);
		}
	}
	else if (command.find("whisper") != std::string::npos)
	{
		bool found = false;
		std::string remove_1 = "/whisper ";
		command.erase(0, remove_1.size());

		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (command.find(connectedSockets[i].playerName) != std::string::npos)
			{
				std::string remove_2 = connectedSockets[i].playerName;
				command.erase(0, remove_2.size()+1);

				OutputMemoryStream _packet;
				_packet << ServerMessage::Newmessage;
				_packet << command;

				int ret = sendPacket(_packet, connectedSockets[i].socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("Welcome package commette not sed. ERROR");
				}
				found = true;
			}
		}
		if (!found)
		{
			std::string error = "user not found. ERROR";
			reportError("user not found. ERROR"); // server message
			OutputMemoryStream _packet;
			_packet << ServerMessage::Newmessage;
			_packet << error;

			int ret = sendPacket(_packet, socket);
		}
	}
	else if (command.find("change_name") != std::string::npos)
	{
		std::string remove_1 = "/change_name ";
		command.erase(0, remove_1.size());

		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (connectedSockets[i].socket == socket)
			{
				connectedSockets[i].playerName = command;

				OutputMemoryStream _packet;
				_packet << ServerMessage::NewName;
				_packet << command;

				int ret = sendPacket(_packet, connectedSockets[i].socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("Welcome package commette not sed. ERROR");
				}
			}
		}

	}
	else if (command.find("add_admin") != std::string::npos)
	{
		OutputMemoryStream _packet;
		_packet << ServerMessage::Command;

		std::string newuser_message = "Users Admin: ";

		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (connectedSockets[i].socket == socket)
			{
				connectedSockets[i].admin = true;
			}
			if (connectedSockets[i].admin)
			{
				newuser_message = newuser_message + "\n - " + connectedSockets[i].playerName;

			}
		}

		_packet << newuser_message;

		int ret = sendPacket(_packet, socket);
		if (ret == SOCKET_ERROR)
		{
			reportError("Admin. ERROR");
		}
	}
	else if (command.find("remove_admin") != std::string::npos)
	{
	OutputMemoryStream _packet;
	_packet << ServerMessage::Command;
	bool admin = false;
	//look to see if userkicking  is admin
	for (int i = 0; i < connectedSockets.size(); i++)
	{
		if (connectedSockets[i].socket == socket)
		{
			if (connectedSockets[i].admin)
				admin = true;
		}

	}
	if(admin){
		std::string newuser_message = "Users Admin: ";

		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (connectedSockets[i].socket == socket)
			{
				connectedSockets[i].admin = false;
				newuser_message = newuser_message + "\n " + connectedSockets[i].playerName + " has been removed form Admin";
			}
			if (connectedSockets[i].admin)
			{
				newuser_message = newuser_message + "\n - " + connectedSockets[i].playerName;

			}
		}

		_packet << newuser_message;
		}
	else
	{
		std::string newuser_message = "You don't have permissions to Remove admin ";
		_packet << newuser_message;
	}

	int ret = sendPacket(_packet, socket);
	if (ret == SOCKET_ERROR)
	{
		reportError("Admin. ERROR");
	}
	}
	else if (command.find("clear chat") != std::string::npos)
	{
	OutputMemoryStream _packet;
	_packet << ServerMessage::Delete;

	std::string newuser_message = "Delete chat ";

	/*for (int i = 0; i < connectedSockets.size(); i++)
	{
		if (connectedSockets[i].socket == socket)
		{
			connectedSockets[i].admin = !connectedSockets[i].admin;
			if (connectedSockets[i].admin)
				newuser_message = newuser_message + " - " + connectedSockets[i].playerName;

		}

	}*/

	_packet << newuser_message;

	int ret = sendPacket(_packet, socket);
	if (ret == SOCKET_ERROR)
	{
		reportError("deleting chat error. ERROR");
	}
	}
	else if (command.find("add_block") != std::string::npos)
	{
		//OutputMemoryStream _packet;
		//_packet << ServerMessage::Command;


		bool found = false;
		bool admin = false;

		

		//look to see if userkicking  is admin
		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (connectedSockets[i].socket == socket)
			{
				if (connectedSockets[i].admin)
					admin = true;
			}

		}
		//look for player to block
		for (int i = 0; i < connectedSockets.size(); i++)
		{
			if (command.find(connectedSockets[i].playerName) != std::string::npos && admin)
			{
				OutputMemoryStream _packet;
				_packet << ServerMessage::Command;
				connectedSockets[i].blocked = true;

				found = true;
			}
			if (command.find(connectedSockets[i].playerName) != std::string::npos && !admin)
			{
				OutputMemoryStream _packet;
				_packet << ServerMessage::Command;

				std::string newuser_message = "Admin priviledge needed";

				_packet << newuser_message;

				int ret = sendPacket(_packet, socket);
				if (ret == SOCKET_ERROR)
				{
					reportError("block without admin. ERROR");
				}
				found = true;
			}
			
		}
		
		if (!found)
		{
			std::string error = "user not found. ERROR";
			reportError("user not found. ERROR"); // server message
			OutputMemoryStream _packet;
			_packet << ServerMessage::Newmessage;
			_packet << error;

			int ret = sendPacket(_packet, socket);
		}

		//print blocked list
		OutputMemoryStream _packet;
		_packet << ServerMessage::Command;


		std::string newuser_message = "Users Blocked: ";

		for (int i = 0; i < connectedSockets.size(); i++)
		{

			if (connectedSockets[i].blocked)
			{
				newuser_message = newuser_message + "\n - " + connectedSockets[i].playerName;

			}
		}

		_packet << newuser_message;

		int ret = sendPacket(_packet, socket);
		if (ret == SOCKET_ERROR)
		{
			reportError("block list. ERROR");
		}
	}
	else if (command.find("remove_block") != std::string::npos)
	{
	//OutputMemoryStream _packet;
	//_packet << ServerMessage::Command;


	bool found = false;
	bool admin = false;



	//look to see if userkicking  is admin
	for (int i = 0; i < connectedSockets.size(); i++)
	{
		if (connectedSockets[i].socket == socket)
		{
			if (connectedSockets[i].admin)
				admin = true;
		}

	}
	//look for player to block
	for (int i = 0; i < connectedSockets.size(); i++)
	{
		if (command.find(connectedSockets[i].playerName) != std::string::npos && admin)
		{
			OutputMemoryStream _packet;
			_packet << ServerMessage::Command;
			connectedSockets[i].blocked = false;

			found = true;
		}
		if (command.find(connectedSockets[i].playerName) != std::string::npos && !admin)
		{
			OutputMemoryStream _packet;
			_packet << ServerMessage::Command;

			std::string newuser_message = "Admin priviledge needed";

			_packet << newuser_message;

			int ret = sendPacket(_packet, socket);
			if (ret == SOCKET_ERROR)
			{
				reportError("block without admin. ERROR");
			}
			found = true;
		}

	}

	if (!found)
	{
		std::string error = "user not found. ERROR";
		reportError("user not found. ERROR"); // server message
		OutputMemoryStream _packet;
		_packet << ServerMessage::Newmessage;
		_packet << error;

		int ret = sendPacket(_packet, socket);
	}

	//print blocked list
	OutputMemoryStream _packet;
	_packet << ServerMessage::Command;


	std::string newuser_message = "Users Blocked: ";

	for (int i = 0; i < connectedSockets.size(); i++)
	{

		if (connectedSockets[i].blocked)
		{
			newuser_message = newuser_message + "\n - " + connectedSockets[i].playerName;

		}
	}

	_packet << newuser_message;

	int ret = sendPacket(_packet, socket);
	if (ret == SOCKET_ERROR)
	{
		reportError("block list. ERROR");
	}
	}
}



