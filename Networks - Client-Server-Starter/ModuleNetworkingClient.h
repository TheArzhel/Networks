#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	//void onSocketReceivedData(SOCKET socket, byte * data) override;
	void onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet) override; //updated OK

	void onSocketDisconnected(SOCKET socket) override;



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging
	};

	ClientState state = ClientState::Stopped;

	sockaddr_in serverAddress = {};
	//SOCKET Socket = INVALID_SOCKET;

	SOCKET Socket;  //updated start OK
	
	//Message variables for chat
	struct Message
	{
		std::string message;
		ImVec4 color;
	};
	//name
	std::string playerName;
	//grroup of messages
	std::vector<Message> Messages;
	//single message
	std::string message;
	// control variable
	//ImVec4 playerColor;
	bool send = false; //update finish OK
};

