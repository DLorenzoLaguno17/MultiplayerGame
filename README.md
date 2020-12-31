
# DOS Attack

DOS Attack is a simple multiplayer game developed by 4th grade students Daniel Lorenzo and Jacobo Galofre. In it you will be controlling a computer system that surfs through a network and battleing with up to 4 other players to see which one can get more kills. The objective is to perform a Denial-of-Service (or DOS) attack to the other computer systems that are connected to the web until you shut them down by overflowing their request systems. To do so, you will be able to move your computer system through the network and send (shoot) ping requests to the other systems.

## Project features

### Gameplay

With keyboard:
* A - Rotate left.
* D - Rotate right.
* Left arrow - Shoot.
* Down arrow - Move forwards.

### Network testing options

* Simulate latency or jitter.
* Simulate packet drops.

Only as a Client:
* Activate/deactivate client side prediction.
* Activate/deactivate entity interpolation.
* Change input delivery interval.

## Feature implementation

### _Implemented by Dani Lorenzo:_
### Delivery Manager

* Completeness: Totally achieved.
* Bugs during implementation: When _Latency / Jitter_ or _Packet Drops_ were activated and then deactivated and some seconds passed, the game crashed since the replication packet memory streams ended up being too big.
* Bug resolution: What happened was that activating and deactivating those checkboxes provoked the sequence number of the packets to augment more than what was expected, and the `DeliveryManager` was only programmed to handle packets with a sequence number **equal or lower** than expected. That resulted in packets never being acknowledged and being resent over and over.

### Client Prediction & Server Reconciliation

* Completeness: Totally achieved.
* Bugs during implementation: The first time we implemented the **Client** prediction the player had a weird flicker, which increased proportionaly with _Latency / Jitter_.
* Bug resolution: That flicker was actually due to a bad implementation. There was a mess because the **Client** was trying to do both the predction and the old replication procedure at the same time, resulting in that strange behaviour.

### Entity Interpolation

* Completeness: Totally achieved.
* Bugs during implementation: There were some problems with the interpolation because we did not quite know how to calculate the interpolation time and the movement of the spaceships were either erratic and abrupt or non-existent. There were bugs on objects creation as well, since they did not appear where they should and did not replace until they received an update.
* Bug resolution: The first problems were solved by creating a ratio variable which handled the time and ensuring it only affected extern **Clients**. The second bug just required the proper initialization of the interpolation values from each `GameObject`.

### _Implemented by Jacobo Galofre:_
### World State Replication

* Completeness: Totally achieved.
* Bugs during implementation: At the beginning, when we replicated the creation of a `GameObject` it appeared twice in the client, and the size was not the appropiate. Furthermore, I could not replicate the hp of the spaceship and the game crashed when a spaceship died. 
* Bug resolution: The first two bugs were really easy to fix. For the hp, I realized I needed to call the `write() / read()` methods of the behaviours in the `ReplicationManager`. To avoid the crashes, I programmed that, everytime there was **Destroy** action in the replication commands, it was inserted at the beggining of the list. 

### Reskin of the game

* Completeness: Partially achieved.
* Bugs during implementation: When changing some audio files the project crashed because it didn't accept the sample rate from the `.wav` files we tried to use.
* Bug resolution: Modify the `.wav` files using audacity so it fitted the sample rate.

### Handle players joining & leaving

* Completeness: Almost totally achieved with bug pending.
* Bugs during implementation: At the beginning if someone tried to disconnect and after that tried to reconnect, the player computer (character) was still inside the list of `proxys` of the **Server**. Furthermore every time the same **Client** tried to reconnect the server took longer to process the `packets`.
* Bug resolution: For the first bug, we made sure that on the `onDisconnect()` function we deleted what we needed to and for the second bug we found it was not working because we missed a `clear()` and the reset of the `DeliveryManager` when disconnecting the **Client**.
* Known bugs: When we disconnect the **Server** while it has connected clients everything disconnects properly. But then, when we try to create the **Server** again it crashes.

