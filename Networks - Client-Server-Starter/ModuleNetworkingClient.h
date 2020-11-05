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
	void onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet) override; //updated

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

	//std::string playerName;

	SOCKET Socket;  //updated start

	struct Message
	{
		ImVec4 color;
		std::string message;
	};

	std::string playerName;
	std::vector<Message> Messages;
	std::string message;

	bool send = false; //update finish
};

