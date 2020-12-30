
# Multiplayer Game

__ is a simple multiplayer game which consists in shooting to death all the other players on the room!

## Project features

### Gameplay

With keyboard:
* A - Rotate left.
* D - Rotate right.
* Left arrow - Move forwards.
* Down arrow - Shoot.

### Network testing options

* Simulate latency or jitter.
* Simulate packet drops.

Only as a Client:
* Activcate/deactivate client side prediction.
* Activcate/deactivate entity interpolation.
* Change input delivery interval.

## Feature implementation

### World State Replication

* Completeness: Totally achieved.
* Bugs during implementation: At the beginning, when we replicated the creation of a `GameObject` it appeared twice in the client, and the size was not the appropiate. Furthermore, I could not replicate the hp of the spaceship and the game crashed when a spaceship died. 
* Bug resolution: The first two bugs were really easy to fix. For the hp, I realized I needed to call the **write() / read()** methods of the behaviours in the _ReplicationManager_. To avoid the crashes, I programmed that, everytime there was **Destroy** action in the replication commands, it was inserted at the beggining of the list. 

### Delivery Manager

* Completeness: Totally achieved.
* Bugs during implementation: When _Latency / Jitter_ or _Packet Loss_ were activated and then deactivated and some seconds passed, the game crashed since the replication packet memory streams ended up being too big.
* Bug resolution: What happened was that activating and deactivating those checkboxes provoked the sequence number of the packets to augment more than what was expected, and the _DeliveryManager_ was only programmed to handle packets with a sequence number **equal or lower** than expected. That resulted in packets never being acknowledged and being resent over and over.

### Client Prediction & Server Reconciliation
* Completeness: Totally achieved.
* Bugs during implementation: The first time we implemented the **Client** prediction the player had a weird flicker, which increased proportionaly with _Latency / Jitter_.
* Bug resolution: That flicker was actually due to a bad implementation. There was a mess because the **Client** was trying to do both the predction and the old replication procedure at the same time, resulting in that strange behaviour.

### Entity Interpolation
* Completeness: Totally achieved.
* Bugs during implementation: 
* Bug resolution:
