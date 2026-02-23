#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	
	return Steering;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	const float rotation = Agent.GetRotation();

	const float randomRotation = FMath::RandRange(0.0f, 1.0f) * 3.14f * 2.0f;

	FVector2D randomDirection = FVector2D(cosf(randomRotation), sinf(randomRotation)) * 400.0f;
	FVector2D direction = FVector2D(cosf(rotation), sinf(rotation)) * 300.0f;

	FVector2D circlePosition = randomDirection + Agent.GetPosition() + direction;

	DrawDebugCircleArc(Agent.GetWorld(), FVector{circlePosition.X, circlePosition.Y, 0.0f }, 300.0f, FVector{}, 1.0f, 10, { 255, 0, 0} );

	Steering.LinearVelocity = Agent.GetPosition() - circlePosition;
	
	return Steering;
}
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Agent.GetPosition() - Target.Position;
	
	return Steering;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	const float SlowRadius = 100.0f;
	const float TargetRadius = 300.0f;

	const float Speed = 500.0f;

	const float distance = FVector2D::Distance(Target.Position, Agent.GetPosition());

	const float scale = fminf(fmaxf(distance - SlowRadius, 0.0f) / TargetRadius, 1.0f);

	Agent.SetMaxLinearSpeed(Speed * scale);

	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	
	return Steering;
}

SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	Agent.SetMaxLinearSpeed(0.0f);

	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	
	return Steering;
}

SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.IsValid = FVector2D::Distance(Target.Position, Agent.GetPosition()) < 300.0f;

	if (Steering.IsValid)
	{
		Agent.SetMaxLinearSpeed(500.0f);

		Agent.PredictedTargetLocation = Target.Position + Target.LinearVelocity * (FVector2D::Distance(Target.Position, Agent.GetPosition()) / Agent.GetMaxLinearSpeed());

		Steering.LinearVelocity = Agent.PredictedTargetLocation - Agent.GetPosition();

		Steering.LinearVelocity *= -1.0f;
	}
	
	return Steering;
}

SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Agent.SetMaxLinearSpeed(300.0f);

	Agent.PredictedTargetLocation = Target.Position + Target.LinearVelocity * (FVector2D::Distance(Target.Position, Agent.GetPosition()) / Agent.GetMaxLinearSpeed());

	Steering.LinearVelocity = Agent.PredictedTargetLocation - Agent.GetPosition();
	
	return Steering;
}
