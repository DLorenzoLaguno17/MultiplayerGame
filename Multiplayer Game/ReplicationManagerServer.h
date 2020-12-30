#pragma once

enum class ReplicationAction
{
	None, 
	Create, 
	Update, 
	Destroy
};

struct ReplicationCommand
{
	ReplicationCommand() {}
	~ReplicationCommand() {};

	ReplicationCommand(ReplicationAction action, uint32 networkId) : 
		action(action), networkId(networkId) {}

	ReplicationAction action = ReplicationAction::None;
	uint32 networkId = -1;
	uint32 destroyerTag = 0;
};

class ReplicationManagerServer
{
public:
	void create(uint32 networkId);
	void update(uint32 networkId);
	void destroy(uint32 networkId, uint32 destroyerTag);

	void write(OutputMemoryStream &packet);

	std::vector<ReplicationCommand> replicationCommands;
};

class ReplicationDelegate : public DeliveryDelegate 
{
public:
	ReplicationDelegate(ReplicationManagerServer* replicationManager) :
		replicationManager(replicationManager), replicationCommands(replicationManager->replicationCommands) {}

	ReplicationDelegate() {}
	~ReplicationDelegate() {}

	void onDeliverySuccess(DeliveryManager* deliveryManager) override {}
	void onDeliveryFailure(DeliveryManager* deliveryManager) override;

	ReplicationManagerServer* replicationManager = nullptr;
	std::vector<ReplicationCommand> replicationCommands;
};