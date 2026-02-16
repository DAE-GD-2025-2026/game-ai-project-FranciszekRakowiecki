#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}, AgentClass{AgentClass}
{
	Agents.SetNum(FlockSize);

 // TODO: initialize the flock and the memory pool

	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>({ new Evade(), new Wander() }));
	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>());
	// {new Cohesion(this), 0.5f}, {new Separation(this), 0.5f}, {new Match(this), 0.5f}
	pBlendedSteering->AddBehaviour({ new Cohesion(this), 0.5f });
	pBlendedSteering->AddBehaviour({ new Separation(this), 0.5f });
	pBlendedSteering->AddBehaviour({ new Match(this), 0.5f });

	Neighbors.SetNum(8);

	for (int index = 0; index < Agents.Num(); ++index)
	{
		ASteeringAgent* SteeringAgent;
		Agents[index] = SteeringAgent = MakeMeAnAgent();

		SteeringAgent->SetSteeringBehavior(pBlendedSteering.get());
	}
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data

	 for (int index = 0; index < this->Agents.Num(); ++index)
	 {
	 	this->pWorld->DestroyActor(Agents[index]);
	 }
}

void Flock::Tick(float DeltaTime)
{
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world

	// Registering neighbors can be done when calculating steering combining two loops into one
	// just preventing recalculating the neighbors for the agent when another behavior already checked for those around it

	// And resetting it every tick so that if the last and first agent in the loop check perhaps because of sorting, means that the agent will still have its neighbors updated correctly
	pLastNeighborCalc = nullptr;
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

ASteeringAgent* Flock::MakeMeAnAgent() const
{
	return this->pWorld->SpawnActor<ASteeringAgent>(this->AgentClass, FVector{0,0,90}, FRotator::ZeroRotator);
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	if (pAgent == pLastNeighborCalc)
		return;
 // TODO: Implement
	NrOfNeighbors = 0;

	for (ASteeringAgent* pAgentToEvade : Agents)
	{
		if (NrOfNeighbors >= Neighbors.Num())
			break;
		if (pAgentToEvade != pAgent && IsWithinRadius(pAgentToEvade, pAgent))
		{
			Neighbors[NrOfNeighbors] = pAgent;
			NrOfNeighbors++;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

	const float factor = 1.0f / float(NrOfNeighbors);

	for (int index = 0; index < NrOfNeighbors; ++index)
	{
		avgPosition += Neighbors[index]->GetPosition() * factor;
	}
	
 // TODO: Implement
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

 // TODO: Implement

	const float factor = 1.0f / float(NrOfNeighbors);

	for (int index = 0; index < NrOfNeighbors; ++index)
	{
		avgVelocity += Neighbors[index]->GetLinearVelocity() * factor;
	}

	return avgVelocity;
}

bool Flock::IsWithinRadius(ASteeringAgent* const Agent, ASteeringAgent* const OtherAgent)
{
	if (Agent == OtherAgent)
		return true;
	const FVector2D AgentPos = Agent->GetPosition();
	const FVector2D OtherPos = OtherAgent->GetPosition();

	const float Radius = 300.0f;

	return IsWithinRadius(AgentPos, OtherPos, Radius);
}

bool Flock::IsWithinRadius(FVector2D const& one, FVector2D const& two, float radius)
{
	bool x = FMath::Abs(one.X - two.X) > radius * 2.0f;
	bool y = FMath::Abs(one.Y - two.Y) > radius * 2.0f;

	if (x && y)
		return false;

	return FVector2D::Distance(one, two) < radius * 2.0f;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
}

