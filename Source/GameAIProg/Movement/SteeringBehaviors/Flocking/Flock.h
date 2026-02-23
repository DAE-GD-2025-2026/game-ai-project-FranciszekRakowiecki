#pragma once

// Toggle this define to enable/disable spatial partitioning
// #define GAMEAI_USE_SPACE_PARTITIONING

#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/SteeringHelpers.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include <memory>
#include "imgui.h"
#ifdef GAMEAI_USE_SPACE_PARTITIONING
#include "../SpacePartitioning/SpacePartitioning.h"
#endif

class CellSpace;

class Flock final
{
public:
	Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize = 10, 
	float WorldSize = 100.f, 
	ASteeringAgent* const pAgentToEvade = nullptr, 
	bool bTrimWorld = false);

	~Flock();

	void Tick(float DeltaTime);
	void RenderDebug();
	void ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize);

	ASteeringAgent* MakeMeAnAgent() const;

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	//const TArray<ASteeringAgent*>& GetNeighbors() const { return pPartitionedSpace->GetNeighbors(); }
	//int GetNrOfNeighbors() const { return pPartitionedSpace->GetNrOfNeighbors(); }
#else // No space partitioning
	void RegisterNeighbors(ASteeringAgent* const Agent);
	int GetNrOfNeighbors() const { return NrOfNeighbors; }
	const TArray<ASteeringAgent*>& GetNeighbors() const { return Neighbors; }
#endif // USE_SPACE_PARTITIONING

	FVector2D GetAverageNeighborPos() const;
	FVector2D GetAverageNeighborVelocity() const;

	static bool IsWithinRadius(ASteeringAgent* const Agent, ASteeringAgent* const OtherAgent);
	static bool IsWithinRadius(FVector2D const& one, FVector2D const& two, float radius);

	void SetTarget_Seek(FSteeringParams const & Target);

	void UpdateTarget(ASteeringAgent* const Agent);

private:
	// For debug rendering purposes
	UWorld* pWorld{nullptr};
	
	int FlockSize{0};
	TArray<ASteeringAgent*> Agents{};
	TSubclassOf<ASteeringAgent> AgentClass;
	std::unique_ptr<CellSpace> pPartitionedSpace{};
	int NrOfCellsX{ 10 };
	TArray<FVector2D> OldPositions{};

	TArray<ASteeringAgent*> Neighbors{};
	
	float NeighborhoodRadius{200.f};
	int NrOfNeighbors{0};
	ASteeringAgent* pLastNeighborCalc{nullptr};

	ASteeringAgent* pAgentToEvade{nullptr};
	
	//Steering Behaviors
	std::unique_ptr<Separation> pSeparationBehavior{};
	std::unique_ptr<Cohesion> pCohesionBehavior{};
	std::unique_ptr<Match> pVelMatchBehavior{};
	std::unique_ptr<Seek> pSeekBehavior{};
	std::unique_ptr<Wander> pWanderBehavior{};
	std::unique_ptr<Evade> pEvadeBehavior{};

	std::unique_ptr<BlendedSteering> pBlendedSteering{};
	std::unique_ptr<PrioritySteering> pPrioritySteering{};
	// UI and rendering
	bool DebugRenderSteering{false};
	bool DebugRenderNeighborhood{true};
	bool DebugRenderPartitions{true};

	void RenderNeighborhood();
};
