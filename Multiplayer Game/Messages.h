#pragma once

enum class ClientMessage : uint8
{
	Hello = 0,
	Input,
	Ping,
	ReplicationAck
};

enum class ServerMessage : uint8
{
	Welcome = 0,
	Unwelcome,
	Ping,
	Replication
};
