#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Send,
	Disconnect,
	Command
};

enum class ServerMessage
{
	Welcome,
	Unwelcome,
	Newuser,
	Newmessage,
	Userdisconnected,
	Command,
	ComDisconnect,
	NewName,
	Delete
};

