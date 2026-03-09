#pragma once

#include <memory>
#include <string>
#include <vector>
#include <Movement/SteeringBehaviors/SteeringHelpers.h>

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// Your own SteeringBehaviors should follow here...

class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

/**
* 		Wander,
		Flee,
		Arrive,
		Evade,
		Pursuit,

		// @ End
		Count*/

class Wander : public ISteeringBehavior
{
public:
	Wander() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Arrive : public ISteeringBehavior
{
public:
	float TargetRadius = 50.0f;
	Arrive() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	void SetTargetRadius(float radius);
};

class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};