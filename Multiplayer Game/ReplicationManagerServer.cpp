#include "Networks.h"

void ReplicationManagerServer::create(uint32 networkId)
{
	ReplicationCommand newCommand(ReplicationAction::Create, networkId);
	replicationCommands.push_back(newCommand);
}

void ReplicationManagerServer::update(uint32 networkId)
{
	ReplicationCommand newCommand(ReplicationAction::Update, networkId);
	replicationCommands.push_back(newCommand);
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	ReplicationCommand newCommand(ReplicationAction::Destroy, networkId);
	replicationCommands.insert(replicationCommands.begin(), newCommand);
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	for (int i = 0; i < replicationCommands.size(); ++i)
	{
		packet << replicationCommands[i].action;
		packet << replicationCommands[i].networkId;

		// Serialize the values oht the object 
		if (replicationCommands[i].action == ReplicationAction::Create)
		{
			GameObject* created_object = App->modLinkingContext->getNetworkGameObject(replicationCommands[i].networkId);
			created_object->writeCreationPacket(packet);
		}
		else if (replicationCommands[i].action == ReplicationAction::Update)
		{
			GameObject* updated_object = App->modLinkingContext->getNetworkGameObject(replicationCommands[i].networkId);
			if (updated_object)
			{
				updated_object->writeUpdatePacket(packet);
				if (updated_object->behaviour)
					updated_object->behaviour->write(packet);
			}
		}
	}

	// Clear the list of actions
	replicationCommands.clear();
}

void ReplicationDelegate::onDeliveryFailure(DeliveryManager* deliveryManager)
{
	// Resend packet
	/*for (int i = 0; i < replicationCommands.size(); ++i)
	{
		if (replicationCommands[i].action == ReplicationAction::Create 
			&& App->modLinkingContext->getNetworkGameObject(replicationCommands[i].networkId) == nullptr)
			replicationManager->create(replicationCommands[i].networkId);

		else if (replicationCommands[i].action == ReplicationAction::Update 
			&& App->modLinkingContext->getNetworkGameObject(replicationCommands[i].networkId) != nullptr)
			replicationManager->update(replicationCommands[i].networkId);

		else if (replicationCommands[i].action == ReplicationAction::Destroy 
			&& App->modLinkingContext->getNetworkGameObject(replicationCommands[i].networkId) == nullptr)
			replicationManager->destroy(replicationCommands[i].networkId);
	}

	replicationCommands.clear();*/
}