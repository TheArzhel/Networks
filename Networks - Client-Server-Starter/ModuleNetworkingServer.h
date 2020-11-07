#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingServer : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer public methods
	//////////////////////////////////////////////////////////////////////

	bool start(int port);

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

	bool isListenSocket(SOCKET socket) const override;

	void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) override;

	//void onSocketReceivedData(SOCKET socket, byte * data) override;
	void onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet) override; // updated OK


	void onSocketDisconnected(SOCKET socket) override;



	//////////////////////////////////////////////////////////////////////
	// State
	//////////////////////////////////////////////////////////////////////

	enum class ServerState
	{
		Stopped,
		Listening
	};

	ServerState state = ServerState::Stopped;

	SOCKET listenSocket;

	struct ConnectedSocket
	{
		sockaddr_in address;
		SOCKET socket;
		std::string playerName;
		//ImVec4 playerColor;
		bool admin = false;
	};

	//////////////////////////////////////////////////////////////////////
	// Server methods UPDATED OK
	//////////////////////////////////////////////////////////////////////
	
	// server added fucntions
	// for username check and commands execution 
	bool CheckName(std::string name);
	void CommandToExecute(std::string command, SOCKET socket);

	std::vector<ConnectedSocket> connectedSockets;

	
};

