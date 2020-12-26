#include "Networks.h"

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet, DeliveryDelegate* delegate)
{
	packet << nextOutgoingSequenceNumber;

	Delivery* newDelivery = new Delivery();
	newDelivery->sequenceNumber = nextOutgoingSequenceNumber++;
	newDelivery->dispatchTime = Time.time;
	newDelivery->delegate = delegate;

	pendingDeliveries.push_back(newDelivery);
	return newDelivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 sequenceNumber = 0;
	packet >> sequenceNumber;

	// Check if it was the expected sequence number
	if (sequenceNumber > nextExpectedSequenceNumber)
		nextExpectedSequenceNumber = sequenceNumber;

	if (sequenceNumber == nextExpectedSequenceNumber)
	{
		sequenceNumbersPendingAck.push_back(sequenceNumber);
		nextExpectedSequenceNumber++;
		return true;
	}

	return false;
}

bool DeliveryManager::hasPendingAcks() const
{
	return sequenceNumbersPendingAck.size() > 0;
}

void DeliveryManager::writePendingAcks(OutputMemoryStream& packet)
{
	for (int i = 0; i < sequenceNumbersPendingAck.size(); ++i)
		packet << sequenceNumbersPendingAck[i];

	sequenceNumbersPendingAck.clear();
}

void DeliveryManager::processAcks(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > 0)
	{
		uint32 currentSequenceNumber = 0;
		packet >> currentSequenceNumber;		

		for (auto currentDelivery : pendingDeliveries)
		{
			// Packet acknowledged
			if (currentDelivery->sequenceNumber == currentSequenceNumber)
			{
				if (currentDelivery->delegate != nullptr)
				{
					currentDelivery->delegate->onDeliverySuccess(this);
					currentDelivery->CleanUp();
				}

				pendingDeliveries.erase(std::find(pendingDeliveries.begin(), pendingDeliveries.end(), currentDelivery));
				break;
			}
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
	for (auto currentDelivery : pendingDeliveries)
	{
		// Packet acknowledged
		if (Time.time - currentDelivery->dispatchTime > PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			if (currentDelivery->delegate != nullptr)
			{
				currentDelivery->delegate->onDeliveryFailure(this);
				currentDelivery->CleanUp();
			}

			pendingDeliveries.erase(std::find(pendingDeliveries.begin(), pendingDeliveries.end(), currentDelivery));
		}
	}
}

void DeliveryManager::clear() 
{
	for (int i = 0; i < pendingDeliveries.size(); ++i) 
		pendingDeliveries[i]->CleanUp();

	pendingDeliveries.clear();
	sequenceNumbersPendingAck.clear();
	nextOutgoingSequenceNumber = 0;
	nextExpectedSequenceNumber = 0;
}

void Delivery::CleanUp()
{
	if (delegate != nullptr) 
	{
		delete delegate;
		delegate = nullptr;
	}
}