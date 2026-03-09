#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"
#include "Shared/ImGuiHelpers.h"
#include "Shared/Level_Base.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass, AWorldTrimVolume* trimWorld,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, AgentClass{AgentClass},pAgentToEvade{pAgentToEvade}, trimWorld(trimWorld)
{
	Agents.SetNum(FlockSize);

	pEvadeBehavior = std::make_unique<Evade>();
	pWanderBehavior = std::make_unique<Wander>();
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pSeparationBehavior = std::make_unique<Separation>(this);
	pVelMatchBehavior = std::make_unique<Match>(this);
	pSeekBehavior = std::make_unique<Seek>();

	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>({ pEvadeBehavior.get(), pWanderBehavior.get() }));
	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>());
	pBlendedSteering->AddBehaviour({ "Cohesion", pCohesionBehavior.get(), 0.5f });
	pBlendedSteering->AddBehaviour({ "Separation", pSeparationBehavior.get(), 0.5f });
	pBlendedSteering->AddBehaviour({ "Match", pVelMatchBehavior.get(), 0.5f });

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, 2000.0f, 2000.0f, NrOfCellsX, NrOfCellsX, MaxNrOfNeighbors);
#endif

	Neighbors.SetNum(MaxNrOfNeighbors);

	for (int index = 0; index < Agents.Num(); ++index)
	{
		ASteeringAgent* SteeringAgent;
		Agents[index] = SteeringAgent = MakeMeAnAgent();
		SteeringAgent->OldLocation = SteeringAgent->GetPosition();
		SteeringAgent->SetMaxLinearSpeed(1000.0f);

		#ifdef GAMEAI_USE_SPACE_PARTITIONING
				pPartitionedSpace->AddAgent(SteeringAgent);
		#endif

		// SteeringAgent->SetSteeringBehavior(pBlendedSteering.get());
	}
}
void Flock::Tick(float DeltaTime)
{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
	for (int index = 0; index < Agents.Num(); ++index)
	{
		ASteeringAgent* SteeringAgent = Agents[index];
		
		pPartitionedSpace->RegisterNeighbors(*SteeringAgent, 150.0f);

		SteeringAgent->AvgNeighborVelocity = pPartitionedSpace->GetAverageNeighborVelocity();
		SteeringAgent->AvgNeighborLocation = pPartitionedSpace->GetAverageNeighborPos();

		SteeringAgent->Tick(DeltaTime);

		pPartitionedSpace->UpdateAgentCell(SteeringAgent, SteeringAgent->OldLocation);

		SteeringAgent->OldLocation = SteeringAgent->GetPosition();
	}
#else
	for (ASteeringAgent* SteeringAgent : Agents)
	{
		RegisterNeighbors(SteeringAgent);

		SteeringAgent->AvgNeighborLocation = GetAverageNeighborPos();
		SteeringAgent->AvgNeighborVelocity = GetAverageNeighborVelocity();

		SteeringAgent->Tick(DeltaTime);
	}
#endif
	
	pLastNeighborCalc = nullptr;
}

void Flock::RenderDebug()
{
	RenderNeighborhood();

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	pPartitionedSpace->RenderCells();
#endif
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

		ImGui::Checkbox("TrimWorld", &trimWorld->bShouldTrimWorld);
		float size = trimWorld->GetTrimWorldSize();
		if (ImGui::DragFloat("TrimSize", &size, 0.01f, 0.0f, 5000.0f))
		{
			trimWorld->SetTrimWorldSize(size);
		}

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

		for (BlendedSteering::WeightedBehavior& Behavior : pBlendedSteering->GetWeightedBehaviorsRef())
		{
			ImGui::DragFloat(Behavior.name.c_str(), &Behavior.Weight, 0.01f, 0.0f, 1.0f);
		}
		
		ImGui::End();
	}
#pragma endregion
#endif
}

ASteeringAgent* Flock::MakeMeAnAgent() const
{
	FActorSpawnParameters params{};
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASteeringAgent* agent{nullptr};

	while (!agent)
	{
		agent = this->pWorld->SpawnActor<ASteeringAgent>(this->AgentClass, FVector{FMath::RandRange(-1000.0f, 1000.0f),FMath::RandRange(-1000.0f, 1000.0f),90}, FRotator::ZeroRotator, params);
	}

	agent->SetSteeringBehavior(pBlendedSteering.get());

	agent->SetActorTickEnabled(false);

	return agent;
}

void Flock::RenderNeighborhood()
{
	ASteeringAgent* SteeringAgent{Agents[0]};

	DrawDebugSphere(pWorld, FVector(SteeringAgent->GetPosition(), 0.0f), 50.0f, 12, FColor(255, 0, 0));
	pPartitionedSpace->RegisterNeighbors(*SteeringAgent, 150.0f);
	for (int index = 0; index < pPartitionedSpace->GetNrOfNeighbors(); ++index)
	{
		ASteeringAgent* neighbor = pPartitionedSpace->GetNeighbors()[index];
		DrawDebugSphere(pWorld, FVector(neighbor->GetPosition(), 0.0f), 50.0f, 12, FColor(0, 255, 0));
	}
	DrawDebugSphere(pWorld, FVector(SteeringAgent->AvgNeighborLocation, 0.0f), 25.0f, 12, FColor(0, 0, 255));
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	if (pAgent == pLastNeighborCalc)
		return;
	NrOfNeighbors = 0;

	pLastNeighborCalc = pAgent;

	for (ASteeringAgent* AgentToEvade : Agents)
	{
		if (NrOfNeighbors >= Neighbors.Num())
			break;
		if (AgentToEvade != pAgent && IsWithinRadius(AgentToEvade, pAgent))
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

	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

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
	pBlendedSteering->SetTarget(Target);
	pPrioritySteering->SetTarget(Target);
	pEvadeBehavior->SetTarget(Target);
	pCohesionBehavior->SetTarget(Target);
	pSeekBehavior->SetTarget(Target);
	pSeparationBehavior->SetTarget(Target);
	pVelMatchBehavior->SetTarget(Target);
	pWanderBehavior->SetTarget(Target);
}