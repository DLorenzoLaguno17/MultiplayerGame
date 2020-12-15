#pragma once

enum class ClientMessage : uint8
{
	Hello,
	Input,
	Ping,
	ReplicationAck
};

enum class ServerMessage : uint8
{
	Welcome,
	Unwelcome,
	Ping,
	Replication,
	Input
};
