#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering;
	pFlock->RegisterNeighbors(&pAgent);

	FVector2D avgPosition = pFlock->GetAverageNeighborPos();
	FVector2D avgVelocity = pFlock->GetAverageNeighborVelocity();

	if (!Flock::IsWithinRadius(pAgent.GetPosition(), avgPosition, 200.0f))
		Steering.LinearVelocity = (avgPosition + avgVelocity) - pAgent.GetPosition();
	
	return Steering;
}

SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	pFlock->RegisterNeighbors(&pAgent);
	
	SteeringOutput Steering;
	pFlock->RegisterNeighbors(&pAgent);

	FVector2D avgPosition = pFlock->GetAverageNeighborPos();
	FVector2D avgVelocity = pFlock->GetAverageNeighborVelocity();

	if (Flock::IsWithinRadius(pAgent.GetPosition(), avgPosition, 200.0f))
		Steering.LinearVelocity = pAgent.GetPosition() - (avgPosition + avgVelocity);
	
	return Steering;
}

SteeringOutput Match::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	pFlock->RegisterNeighbors(&pAgent);

	SteeringOutput Steering{};
	
	Steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	
	return Steering;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
