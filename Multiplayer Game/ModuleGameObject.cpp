#include "Networks.h"
#include "math.h"

bool ModuleGameObject::init()
{
	return true;
}

bool ModuleGameObject::preUpdate()
{
	BEGIN_TIMED_BLOCK(GOPreUpdate);

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::STARTING,     // After INSTANTIATE
		GameObject::UPDATING,     // After STARTING
		GameObject::UPDATING,     // After UPDATING
		GameObject::DESTROYING,   // After DESTROY
		GameObject::NON_EXISTING  // After DESTROYING
	};

	for (GameObject &gameObject : gameObjects)
	{
		gameObject.state = gNextState[gameObject.state];
	}

	END_TIMED_BLOCK(GOPreUpdate);

	return true;
}

bool ModuleGameObject::update()
{	
	// Delayed destructions
	for (DelayedDestroyEntry &destroyEntry : gameObjectsWithDelayedDestruction)
	{
		if (destroyEntry.object != nullptr)
		{
			destroyEntry.delaySeconds -= Time.deltaTime;
			if (destroyEntry.delaySeconds <= 0.0f)
			{
				Destroy(destroyEntry.object);
				destroyEntry.object = nullptr;
			}
		}
	}

	return true;
}

bool ModuleGameObject::postUpdate()
{
	return true;
}

bool ModuleGameObject::cleanUp()
{
	return true;
}

GameObject * ModuleGameObject::Instantiate()
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		GameObject &gameObject = App->modGameObject->gameObjects[i];

		if (gameObject.state == GameObject::NON_EXISTING)
		{
			gameObject = GameObject();
			gameObject.id = i;
			gameObject.state = GameObject::INSTANTIATE;
			return &gameObject;
		}
	}

	ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
	return nullptr;
}

void ModuleGameObject::Destroy(GameObject * gameObject)
{
	ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::DESTROY,      // After INSTANTIATE
		GameObject::DESTROY,      // After STARTING
		GameObject::DESTROY,      // After UPDATING
		GameObject::DESTROY,      // After DESTROY
		GameObject::DESTROYING    // After DESTROYING
	};

	ASSERT(gameObject->state < GameObject::STATE_COUNT);
	gameObject->state = gNextState[gameObject->state];
}

void ModuleGameObject::Destroy(GameObject * gameObject, float delaySeconds)
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (App->modGameObject->gameObjectsWithDelayedDestruction[i].object == nullptr)
		{
			App->modGameObject->gameObjectsWithDelayedDestruction[i].object = gameObject;
			App->modGameObject->gameObjectsWithDelayedDestruction[i].delaySeconds = delaySeconds;
			break;
		}
	}
}

GameObject * Instantiate()
{
	GameObject *result = ModuleGameObject::Instantiate();
	return result;
}

void Destroy(GameObject * gameObject)
{
	ModuleGameObject::Destroy(gameObject);
}

void Destroy(GameObject * gameObject, float delaySeconds)
{
	ModuleGameObject::Destroy(gameObject, delaySeconds);
}

void GameObject::readCreationPacket(const InputMemoryStream& packet)
{
	// Values
	packet >> position.x;
	packet >> position.y;
	packet >> size.x;
	packet >> size.y;
	packet >> angle;

	initial_angle = angle;
	initial_position = position;
	final_angle = angle;
	final_position = position;

	// Sprite
	sprite = App->modRender->addSprite(this);
	packet >> sprite->pivot.x;
	packet >> sprite->pivot.y;
		   
	packet >> sprite->color.r;
	packet >> sprite->color.g;
	packet >> sprite->color.b;
	packet >> sprite->color.a;

	packet >> sprite->order;
	std::string filename;
	packet >> filename;

	if (filename == "spacecraft1.png")
		sprite->texture = App->modResources->spacecraft1;
	else if (filename == "spacecraft2.png")
		sprite->texture = App->modResources->spacecraft2;
	else if (filename == "spacecraft3.png")
		sprite->texture = App->modResources->spacecraft3;
	else if (filename == "laser.png")
		sprite->texture = App->modResources->laser;
	else if (filename == "explosion1.png")
		sprite->texture = App->modResources->explosion1;
	else if (filename == "asteroid1.png")
		sprite->texture = App->modResources->asteroid1;
	else if (filename == "asteroid2.png")
		sprite->texture = App->modResources->asteroid2;

	// Collider
	ColliderType col_type = ColliderType::None;
	bool hasCollider = false;
	packet >> hasCollider;
	
	// Only spaceships have collider at the time of their creation
	if (hasCollider)
	{
		packet >> col_type;
		collider = App->modCollision->addCollider(col_type, this);
		packet >> collider->isTrigger;
	}
	else
	{
		bool hasAnimation = false;
		packet >> hasAnimation;

		// If it has animation it's an explosion
		if (hasAnimation)
		{
			animation = App->modRender->addAnimation(this);
			animation->clip = App->modResources->explosionClip;
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
		// If it doesn't have animation it's a laser
		else
			col_type = ColliderType::Laser;
	}

	// Create behaviour
	if (col_type == ColliderType::Player)
	{
		Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(this);
		behaviour = spaceshipBehaviour;
		behaviour->isServer = false;
	}
	else if (col_type == ColliderType::Laser)
	{
		Laser* laserBehaviour = App->modBehaviour->addLaser(this);
		behaviour = laserBehaviour;
		behaviour->isServer = false;
	}
}

void GameObject::writeCreationPacket(OutputMemoryStream& packet)
{
	// Values
	packet << position.x;
	packet << position.y;
	packet << size.x;
	packet << size.y;
	packet << angle;

	// Sprite
	packet << sprite->pivot.x;
	packet << sprite->pivot.y;

	packet << sprite->color.r;
	packet << sprite->color.g;
	packet << sprite->color.b;
	packet << sprite->color.a;

	packet << sprite->order;
	std::string filename = sprite->texture->filename;
	packet << filename;

	// Animation
	bool hasAnimation = false;
	if (animation != nullptr)
		hasAnimation = true;

	// Collider
	if (collider != nullptr)
	{
		packet << true;
		packet << collider->type;
		packet << collider->isTrigger;
	}
	else
	{
		packet << false;
		packet << hasAnimation;
	}
}

void GameObject::readUpdatePacket(const InputMemoryStream& packet)
{
	// Interpolation
	initial_position = position;
	initial_angle = angle;

	packet >> final_position.x;
	packet >> final_position.y;
	packet >> size.x;
	packet >> size.y;
	packet >> final_angle;

	if (App->modNetClient->getMyNetworkId() != networkId)
		seconds_elapsed = 0.0f;	
}

void GameObject::writeUpdatePacket(OutputMemoryStream& packet)
{
	// Values
	packet << position.x;
	packet << position.y;
	packet << size.x;
	packet << size.y;
	packet << angle;
}

void GameObject::interpolate(float ratio)
{
	position = initial_position + ratio * (final_position - initial_position);
	angle = initial_angle + ratio * (final_angle - initial_angle);
}