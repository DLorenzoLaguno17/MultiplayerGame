
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

### Dani Lorenzo
* **World State Replication**

Completeness: Totally achieved.

Bugs during implementation: At the beginning, when we replicated the creation of a GameObject it appeared twice in the client, and the size was not the appropiate. Furthermore, I could not replicate the hp of the spaceship and the game crashed when a spaceship died. 

Bug resolution: The first two bugs were really easy to fix. For the hp, I realized I needed to call the write()/read() methods of the behaviours in the ReplicationManager. To avoid the crashes, I programmed that, everytime there was _Destroy_ action in the replication commands, it was inserted at the beggining of the list. 


* **Delivery Manager**

Completeness: Totally achieved.

Bugs during implementation: When Latency/Jitter or Packet Loss were activated and then deactivated and some seconds passed, the game crashed since the replication packet memory streams ended up being too big.

Bug resolution: What happened was that activating and deactivating those checkboxes provoked the sequence number of the packets to augment more than what was expected, and the DeliveryManager was only programmed to handle packets with a sequence number _equal or lower_ than expected. That resulted in packets never being acknowledged and being resent over and over.

* **Client Prediction & Server Reconciliation**
Completeness: Totally achieved.
Bugs during implementation: weird flicker
Bug resolution: imrpove replication

* **Entity Interpolation**
Completeness: Totally achieved.
Bugs during implementation:
Bug resolution:

### Jacobo Galofre
