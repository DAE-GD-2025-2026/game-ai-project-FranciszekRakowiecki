
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{}

BlendedSteering::~BlendedSteering()
{
	for (WeightedBehavior& behavior : WeightedBehaviors)
	{
		delete behavior.pBehavior;
		behavior.pBehavior = nullptr;
	}
};

void BlendedSteering::RecalculateWeightMax()
{
	totalWeight = 0.0f;
	for (WeightedBehavior& behavior : WeightedBehaviors)
	{
		totalWeight += behavior.Weight;
	}
}

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	SteeringOutput Steering = {};
	

	for (WeightedBehavior& behavior : WeightedBehaviors)
	{
		Steering = behavior.pBehavior->CalculateSteering(DeltaT, Agent);

		const float normalizedWeight = (behavior.Weight / totalWeight);
		
		BlendedSteering.LinearVelocity += Steering.LinearVelocity * normalizedWeight;
	}

	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

PrioritySteering::~PrioritySteering()
{
	for (ISteeringBehavior* SteeringBehavior : m_PriorityBehaviors)
	{
		delete SteeringBehavior;
		SteeringBehavior = nullptr;
	}
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}