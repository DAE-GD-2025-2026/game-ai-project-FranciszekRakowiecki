#pragma once
#include <vector>

#include "../Steering/SteeringBehaviors.h"

//****************
//BLENDED STEERING
class BlendedSteering final: public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		std::string name;
		ISteeringBehavior* pBehavior = nullptr;
		float Weight = 0.f;

		WeightedBehavior(std::string name, ISteeringBehavior* const pBehavior, float Weight) :
			name(name),
			pBehavior(pBehavior),
			Weight(Weight)
		{};
	};

	BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors);
	~BlendedSteering();

	void AddBehaviour(const WeightedBehavior& WeightedBehavior) { WeightedBehaviors.push_back(WeightedBehavior); }
	void RecalculateWeightMax();
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

	float* GetWeight(ISteeringBehavior* const SteeringBehavior);
	
	// returns a reference to the weighted behaviors, can be used to adjust weighting. Is not intended to alter the behaviors themselves.
	std::vector<WeightedBehavior>& GetWeightedBehaviorsRef() { return WeightedBehaviors; }

private:
	std::vector<WeightedBehavior> WeightedBehaviors = {};

	float totalWeight = 0.0f;

	// using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};

//*****************
//PRIORITY STEERING
class PrioritySteering final: public ISteeringBehavior
{
public:
	PrioritySteering(const std::vector<ISteeringBehavior*>& priorityBehaviors)
		:m_PriorityBehaviors(priorityBehaviors) 
	{}
	virtual ~PrioritySteering();

	void AddBehaviour(ISteeringBehavior* const pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

private:
	std::vector<ISteeringBehavior*> m_PriorityBehaviors = {};

	// using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};