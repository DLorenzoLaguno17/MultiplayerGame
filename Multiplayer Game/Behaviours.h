#pragma once

enum class BehaviourType : uint8
{
	None,
	Spaceship,
	Laser,
};

struct Behaviour
{
	GameObject *gameObject = nullptr;
	bool isServer = false;
	bool isLocalPlayer = false;

	virtual BehaviourType type() const = 0;

	virtual void start() { }
	virtual void update() { }
	virtual void destroy() { }

	virtual void onInput(const InputController &input) { }
	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void write(OutputMemoryStream &packet) { }
	virtual void read(const InputMemoryStream &packet) { }
};

struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;

	BehaviourType type() const override { return BehaviourType::Laser; }

	void start() override;
	void update() override;
};

struct Spaceship : public Behaviour
{
	GameObject *lifebar = nullptr;
	static const uint8 MAX_HIT_POINTS = 5;
	uint8 hitPoints = MAX_HIT_POINTS;

	BehaviourType type() const override { return BehaviourType::Spaceship; }

	void start() override;
	void update() override;
	void destroy() override;

	void onInput(const InputController &input) override;
	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;
	void read(const InputMemoryStream &packet) override;
};