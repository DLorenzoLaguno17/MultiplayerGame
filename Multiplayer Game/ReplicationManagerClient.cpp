#include "Networks.h"

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	// Read the packet until it is empty
	while (packet.RemainingByteCount() > 0)
	{
		ReplicationAction newAction = ReplicationAction::None;
		uint32 networkId = -1;
		uint32 destroyerTag = 0;

		packet >> newAction;
		packet >> networkId;
		packet >> destroyerTag;

		if (newAction == ReplicationAction::Destroy)
		{
			GameObject* destroyed_object = App->modLinkingContext->getNetworkGameObject(networkId);
			if (destroyed_object)
			{
				App->modLinkingContext->unregisterNetworkGameObject(destroyed_object);
				Destroy(destroyed_object);
				
				if (App->modNetClient->clientGameObject && App->modNetClient->clientGameObject->tag == destroyerTag)
					App->modNetClient->playerKills++;
			}
		}
		else if (newAction == ReplicationAction::Create)
		{
			GameObject* newObject = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(newObject, networkId);
			newObject->readCreationPacket(packet);
		}
		else if (newAction == ReplicationAction::Update)
		{
			GameObject* updated_object = App->modLinkingContext->getNetworkGameObject(networkId);
			if (updated_object)
			{
				updated_object->readUpdatePacket(packet);
				updated_object->behaviour->read(packet);
			}
		}
	}
}