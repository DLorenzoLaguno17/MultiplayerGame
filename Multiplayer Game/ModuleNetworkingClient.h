#pragma once
#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	void setServerAddress(const char *serverAddress, uint16 serverPort);
	void setPlayerInfo(const char *playerName, uint8 spaceshipType);
	void sendHelloPacket();

	uint32 getMyNetworkId() { return networkId; }
	bool isClient() const override { return true; }

private:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onStart() override;
	void onGui() override;
	void onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress) override;

	void onUpdate() override;
	void onDisconnect() override;
	void onConnectionReset(const sockaddr_in &fromAddress) override;

	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Connecting,
		Connected
	};

	ClientState state = ClientState::Stopped;

	std::string serverAddressStr;
	uint16 serverPort = 0;

	sockaddr_in serverAddress = {};
	std::string playerName = "player";
	uint8 spaceshipType = 0;

	uint32 playerId = 0;
	uint32 networkId = 0;

	// Input
	static const int MAX_INPUT_DATA_SIMULTANEOUS_PACKETS = 64;
	InputPacketData inputData[MAX_INPUT_DATA_SIMULTANEOUS_PACKETS];
	uint32 inputDataFront = 0;
	uint32 inputDataBack = 0;

	// Latency management
	float inputDeliveryIntervalSeconds = 0.05f;
	float secondsSinceLastInputDelivery = 0.0f;

	// Virtual connection
	float secondsSinceLastPingToServer = 0.0f;
	float secondsSinceLastPacketFromServer = 0.0f;

	// Replication
	ReplicationManagerClient replicationManager;

	// Delivery manager
	DeliveryManager deliveryManager;
};

// Delivery delegates
class LoginDelegate : public DeliveryDelegate 
{
	void onDeliverySuccess(DeliveryManager* deliveryManager) override;
	void onDeliveryFailure(DeliveryManager* deliveryManager) override;
};