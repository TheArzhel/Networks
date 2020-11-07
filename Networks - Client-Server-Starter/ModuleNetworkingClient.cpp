#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName,std::vector<float> colorName)
{
	int ret = 0;
	playerName = pplayerName;
	vecColor = colorName;

	// TODO(jesus): TCP connection stuff
	// Create the socket
	Socket = socket(AF_INET, SOCK_STREAM, 0);
	//Report Error
	if (Socket == INVALID_SOCKET)
	{
		reportError("Fail to create Client Socket");
	}

	// Destination IP address (to connect to a remote host)
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	//We can use the function ​inet_pton​ to convert aformatted string into the appropriate 
	//library’s IP address internal data format
	//inet_pton​(​AF_INET​,​ remoteAddrStr​,​​&​remoteAddr​.​sin_addr​);
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	// Any process that wants to initiate a new connection has to use the ​connect​ function
	// int​ connect​(​SOCKET s​,​​const​​struct​ sockaddr ​*​ addr​,​​int​ addrlen​);
	ret = connect(Socket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	//Report Error
	if (ret == SOCKET_ERROR)
	{
		reportError("Fail to connect Client to Server");
	}
	else
	{
		// - Add the created socket to the managed list of sockets using addSocket()
		addSocket(Socket);

		// If everything was ok... change the state
		state = ClientState::Start;
	}



	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update() //updated OK
{
	//if (state == ClientState::Start)
	//{
	//	// TODO(jesus): Send the player name to the server

	//	// int​ send​(​SOCKET s​,​​const​​char​​*​ buf​,​​int​ len​,​​int​ flags​);
	//	int ret = send(Socket, playerName.c_str(), playerName.size(), 0);
	//	if (ret == SOCKET_ERROR)
	//	{
	//		reportError("Fail to send Name, client based error. ClientUpdate");
	//	}
	//}

	OutputMemoryStream Package;


	if (state == ClientState::Start)
	{
		// TODO(jesus): Send the player name to the server
		//int ret = send(socketClient, playerName.c_str(), playerName.size(), 0); // Updated OK 

		Package << ClientMessage::Hello;
		Package << playerName;
		Package << vecColor;
		colorText = { vecColor[0], vecColor[1], vecColor[2], vecColor[3] };
		if (sendPacket(Package, Socket))
		{
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}

	if (state == ClientState::Logging)
	{
		if (send)
		{
			if (message[0] == '/')
			{
				Package << ClientMessage::Command;
				DLOG("Command %s", message.c_str());
			}
			else
			{
				Package << ClientMessage::Send;
			}

			Package << message;
			Package << vecColor;

			if (!sendPacket(Package, Socket))
			{
				state = ClientState::Stopped;
				disconnect();
			}

			message.clear();
			send = false;
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);
		//ImGui::SetKeyboardFocusHere();

		ImGui::Text("%s connected to the server...", playerName.c_str());
		if (ImGui::Button("Disconect"))
		{
			onSocketDisconnected(Socket);
			shutdown(Socket, 2);
		}

		ImGui::BeginChild("Chat", ImVec2(375, 400), true);  //updated start OK

		for (int i = 0; i < Messages.size(); ++i) 
		{
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, Messages[i].color);
			ImGui::Text("%s", Messages[i].message.c_str());
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();

		char buff[1024] = "\0";

		if (ImGui::InputText("write", buff, 1024, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			send = true;
			std::string mymessage(buff);
			message = mymessage;
		} //updated finish OK

		
		
		if (state == ClientState::Logging)
		{
			//static ImVec4 color = colorText;
			ImGui::SameLine();
			ImGui::ColorEdit4("MyColor##13", (float*)&colorText, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			
			vecColor = std::vector<float>{ colorText.x,colorText.y,colorText.z,colorText.w };
		}
		

		ImGui::End();
	}

	return true;
}

void ChangeColorChat() {

};

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet)
{
	//state = ClientState::Stopped;
	//updated OK 

	ServerMessage serverMessage;
	packet >> serverMessage;

	//create state machine
	//act depending on the server message
	std::string messageData;
	ImVec4 color;

	Message messageToSend;
	switch (serverMessage)
	{
	case ServerMessage::Welcome:

		packet >> messageData;

		messageToSend.color = { 1.0f,0.0f,0.0f,1.0f };
		messageToSend.message = messageData;
		Messages.push_back(messageToSend);
		break;

	case ServerMessage::Unwelcome:

		packet >> messageData;
		disconnect();
		state = ClientState::Stopped;
		WLOG("%s", messageData.c_str());
		break;

	case ServerMessage::Newuser:

		packet >> messageData;
		messageToSend.color = { 0.0f,0.0f,1.0f,1.0f };
		messageToSend.message = messageData;
		Messages.push_back(messageToSend);
		break;

	case ServerMessage::Newmessage:

		packet >> messageData;
		packet >> vecColor;
		color = { vecColor[0],vecColor[1], vecColor[2], vecColor[3] };
		messageToSend.color = color;
		messageToSend.message = messageData;

		//do not puch message if its blocked
		
		
		Messages.push_back(messageToSend);
		break;

	case ServerMessage::Userdisconnected:

		packet >> messageData;

		messageToSend.color = { 1.0f,1.0f,0.0f,1.0f };
		messageToSend.message = messageData;
		Messages.push_back(messageToSend);
		break;

	case ServerMessage::Command:

		packet >> messageData;

		messageToSend.color = { 1.0f,1.0f,0.0f,1.0f };
		messageToSend.message = messageData;
		Messages.push_back(messageToSend);
		break;

	case ServerMessage::Delete:

		packet >> messageData;

		deleteChat();

		messageToSend.color = { 1.0f,1.0f,0.0f,1.0f };
		messageToSend.message = messageData;
		Messages.push_back(messageToSend);

		break;

	case ServerMessage::ComDisconnect:

		onSocketDisconnected(Socket);
		shutdown(Socket, 2);
		disconnect();
		break;

	case ServerMessage::NewName:

		packet >> messageData;
		playerName = messageData;
		break;

	default:
		break;
	}

}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	//updated OK
	//Delete messages and stop state
	Messages.clear();
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::deleteChat()
{
	//Delete messages and stop state
	Messages.clear();
}