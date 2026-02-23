#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering;

	FVector2D avgPosition = pAgent.AvgNeighborLocation;
	FVector2D avgVelocity = pAgent.AvgNeighborVelocity;

	if (!Flock::IsWithinRadius(pAgent.GetPosition(), avgPosition, 200.0f))
		Steering.LinearVelocity = (avgPosition + avgVelocity) - pAgent.GetPosition();
	
	return Steering;
}

SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering;

	FVector2D avgPosition = pAgent.AvgNeighborLocation;
	FVector2D avgVelocity = pAgent.AvgNeighborVelocity;

	if (Flock::IsWithinRadius(pAgent.GetPosition(), avgPosition, 200.0f))
		Steering.LinearVelocity = pAgent.GetPosition() - (avgPosition + avgVelocity);
	
	return Steering;
}

SteeringOutput Match::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	
	Steering.LinearVelocity = pAgent.AvgNeighborVelocity;
	
	return Steering;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
