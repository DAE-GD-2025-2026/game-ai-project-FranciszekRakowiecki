#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering;
	Steering.IsValid = false;

	FVector2D avgPosition = pAgent.AvgNeighborLocation;
	FVector2D avgVelocity = pAgent.AvgNeighborVelocity;

	// float distance = FVector2D::Distance(avgPosition, pAgent.GetPosition());

	if (!Flock::IsWithinRadius(pAgent.GetPosition(), avgPosition, 250.0f))
	{
		FVector2D vel = (avgPosition + avgVelocity * pAgent.GetMaxLinearSpeed()) - pAgent.GetPosition();
		vel.Normalize();
		Steering.LinearVelocity = vel;
		Steering.IsValid = true;
	}
	
	return Steering;
}

SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering;

	Steering.IsValid = false;

	FVector2D avgPosition = pAgent.AvgNeighborLocation;
	FVector2D avgVelocity = pAgent.AvgNeighborVelocity;

	if (Flock::IsWithinRadius(pAgent.GetPosition(), avgPosition, 200.0f))
	{
		FVector2D vel = pAgent.GetPosition() - (avgPosition + avgVelocity * pAgent.GetMaxLinearSpeed());
		vel.Normalize();
		Steering.LinearVelocity = vel;
		Steering.IsValid = true;
	}
	
	return Steering;
}

SteeringOutput Match::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};

	FVector forward = pAgent.GetActorForwardVector();
	FVector2D vel = { forward.X, forward.Y };
	vel.Normalize();
	Steering.LinearVelocity = vel;
	
	return Steering;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
