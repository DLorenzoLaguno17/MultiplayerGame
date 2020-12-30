#include "ModuleNetworkingClient.h"

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::setServerAddress(const char * pServerAddress, uint16 pServerPort)
{
	serverAddressStr = pServerAddress;
	serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char * pPlayerName, uint8 pSpaceshipType)
{
	playerName = pPlayerName;
	spaceshipType = pSpaceshipType;
}

//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
	if (!createSocket()) return;

	if (!bindSocketToPort(0)) 
	{
		disconnect();
		return;
	}

	// Create remote address
	serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
	if (res == SOCKET_ERROR) 
	{
		reportError("ModuleNetworkingClient::startClient() - inet_pton");
		disconnect();
		return;
	}

	state = ClientState::Connecting;

	inputDataFront = 0;
	inputDataBack = 0;
	secondsSinceLastInputDelivery = 0.0f;

	spawned = false;
}

void ModuleNetworkingClient::onGui()
{
	if (state == ClientState::Stopped) return;

	if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (state == ClientState::Connecting)
		{
			ImGui::Text("Connecting to server...");
		}
		else if (state == ClientState::Connected)
		{
			ImGui::Text("Connected to server");

			ImGui::Separator();

			ImGui::Text("Player info:");
			ImGui::Text(" - Id: %u", playerId);
			ImGui::Text(" - Name: %s", playerName.c_str());

			ImGui::Separator();

			ImGui::Text("Spaceship info:");
			ImGui::Text(" - Type: %u", spaceshipType);
			ImGui::Text(" - Network id: %u", networkId);

			vec2 playerPosition = {};
			GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr)
				playerPosition = playerGameObject->position;
			
			ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

			ImGui::Separator();

			ImGui::Text("Connection checking info:");
			ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
			ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

			ImGui::Separator();

			ImGui::Text("Input:");
			ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);
		}
	}
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	// Reset the "last packet" timer
	secondsSinceLastPacketFromServer = 0.0f;

	uint32 protoId;
	packet >> protoId;
	if (protoId != PROTOCOL_ID) return;

	ServerMessage message;
	packet >> message;

	if (message == ServerMessage::Ping)
		deliveryManager.processAcks(packet);

	if (state == ClientState::Connecting)
	{
		if (message == ServerMessage::Welcome)
		{
			packet >> playerId;
			packet >> networkId;

			LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
			state = ClientState::Connected;
		}
		else if (message == ServerMessage::Unwelcome)
		{
			WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
			disconnect();
		}
	}
	else if (state == ClientState::Connected)
	{
		// Handle replication packages
		if (message == ServerMessage::Replication)
		{
			uint32 currentSequenceNumber = 0;
			packet >> currentSequenceNumber;

			if (deliveryManager.processSequenceNumber(packet))
				replicationManager.read(packet);

			// Server reconciliation
			GameObject* myGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (myGameObject && currentSequenceNumber > inputDataFront)
			{
				InputController inputForServer;
				for (uint32 i = inputDataFront; i < currentSequenceNumber; ++i)
				{
					InputPacketData& inputPacketData = inputData[i % ArrayCount(inputData)];
					inputControllerFromInputPacketData(inputPacketData, inputForServer);

					if (myGameObject->behaviour)
						myGameObject->behaviour->onInput(inputForServer);
				}

				inputDataFront = currentSequenceNumber;
			}
		}
	}
}

void ModuleNetworkingClient::onUpdate()
{
	if (state == ClientState::Stopped) return;

	if (state == ClientState::Connecting)
	{
		sendHelloPacket();
	}
	else if (state == ClientState::Connected)
	{
		secondsSinceLastPacketFromServer += Time.deltaTime;
		secondsSinceLastPingToServer += Time.deltaTime;
		secondsSinceLastInputDelivery += Time.deltaTime;

		// Client prediction
		GameObject* myGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		if (myGameObject != nullptr && myGameObject->behaviour != nullptr)
			myGameObject->behaviour->onInput(Input);

		// Disconnect the client if the time since the last received packet is greater than 5 seconds
		if (secondsSinceLastPacketFromServer > DISCONNECT_TIMEOUT_SECONDS)
			disconnect();

		// Send a Ping to the server every 0.5 seconds
		if (secondsSinceLastPingToServer > PING_INTERVAL_SECONDS)
		{
			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Ping;
			sendPacket(packet, serverAddress);
			secondsSinceLastPingToServer = 0.0f;
		}		

		// Check pending acknowledgements
		if (deliveryManager.hasPendingAcks())
		{
			OutputMemoryStream replicationPacket;
			replicationPacket << PROTOCOL_ID;
			replicationPacket << ClientMessage::ReplicationAck;
			deliveryManager.writePendingAcks(replicationPacket);
			sendPacket(replicationPacket, serverAddress);
		}

		// Process more inputs if there's space
		if (inputDataBack - inputDataFront < ArrayCount(inputData))
		{
			// Pack current input
			uint32 currentInputData = inputDataBack++;
			InputPacketData& inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
			inputPacketData.sequenceNumber = currentInputData;
			inputPacketData.horizontalAxis = Input.horizontalAxis;
			inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);

			// Input delivery interval timed out: create a new input packet
			if (secondsSinceLastInputDelivery > inputDeliveryIntervalSeconds)
			{
				OutputMemoryStream packet;
				packet << PROTOCOL_ID;
				packet << ClientMessage::Input;

				for (uint32 i = inputDataFront; i < inputDataBack; ++i)
				{
					InputPacketData& inputPacketData = inputData[i % ArrayCount(inputData)];
					packet << inputPacketData.sequenceNumber;
					packet << inputPacketData.horizontalAxis;
					packet << inputPacketData.verticalAxis;
					packet << inputPacketData.buttonBits;
				}
				sendPacket(packet, serverAddress);
				secondsSinceLastInputDelivery = 0.0f;
			}
		}

		// Update camera for player
		GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		if (playerGameObject != nullptr)
		{
			App->modRender->cameraPosition = playerGameObject->position;
			spawned = true;
		}
		else if (spawned && playerGameObject == nullptr)
		{
			
			GameObject* gameObject = Instantiate();
			gameObject->position = { App->modRender->cameraPosition.x + 200, App->modRender->cameraPosition.y + 200 };
			//gameObject->size = { 100, 100 };

			// Create sprite
			gameObject->sprite = App->modRender->addSprite(gameObject);
			gameObject->sprite->order = 5;
			gameObject->sprite->texture = App->modResources->popUp;

			// Create collider
			gameObject->collider = App->modCollision->addCollider(ColliderType::PopUp, gameObject);
			gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events


			
			//disconnect();
		}
	}
}

void ModuleNetworkingClient::sendHelloPacket()
{
	OutputMemoryStream packet;
	packet << PROTOCOL_ID;
	packet << ClientMessage::Hello;

	LoginDelegate* delegate = new LoginDelegate();
	deliveryManager.writeSequenceNumber(packet, delegate);

	packet << playerName;
	packet << spaceshipType;
	sendPacket(packet, serverAddress);
}

void ModuleNetworkingClient::onDisconnect()
{
	state = ClientState::Stopped;

	GameObject* networkGameObjects[MAX_NETWORK_OBJECTS] = {};
	uint16 networkGameObjectsCount;
	App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
	App->modLinkingContext->clear();

	for (uint32 i = 0; i < networkGameObjectsCount; ++i)
	{
		Destroy(networkGameObjects[i]);
	}

	deliveryManager.clear();
	App->modRender->cameraPosition = {};
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in & fromAddress)
{
	disconnect();
}

void LoginDelegate::onDeliverySuccess(DeliveryManager* deliveryManager) 
{

}

void LoginDelegate::onDeliveryFailure(DeliveryManager* deliveryManager) 
{ 
	App->modNetClient->sendHelloPacket(); 
}