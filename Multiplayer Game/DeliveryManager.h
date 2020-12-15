#pragma once

class DeliveryManager;

class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	Delivery() {};
	~Delivery() {}

	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;

	void CleanUp();

};

class DeliveryManager
{
public:
	// For senders to write a new sequence numbers into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream &packet, DeliveryDelegate* delegate);

	// For receivers to process the sequence number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream &packet);

	// For receivers to write acknowledged sequence numbers into a packet
	bool hasPendingAcks() const;
	void writePendingAcks(OutputMemoryStream &packet);

	// For senders to process acknowledged sequence numbers from a packet
	void processAcks(const InputMemoryStream &packet);
	void processTimedOutPackets();

	void clear();

private:
	// Sender side
	uint32 nextOutgoingSequenceNumber = 0; 			// The next outgoing sequence number
	std::vector<Delivery*> pendingDeliveries;		// List of pending deliveries

	// Receiver side
	uint32 nextExpectedSequenceNumber = 0;			// The next expected sequence number
	std::vector<uint32> sequenceNumbersPendingAck;	// A list of sequence numbers pending ack
};