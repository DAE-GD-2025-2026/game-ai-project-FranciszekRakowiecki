#include "SpacePartitioning.h"

#include <iostream>
#include <__msvc_ostream.hpp>

#include "DrawDebugHelpers.h"

// --- Cell ---
// ------------
Cell::Cell(int index, float Left, float Bottom, float Width, float Height) : index(index)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}, MaxNeighbors(MaxEntities)
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	for (int row = 0, Index = 0; row < Rows; ++row)
	{
		for (int col = 0; col < Cols; ++col, Index++)
		{
			Cells.emplace_back(Cell(Index, col * CellWidth - Width / 2.0f, row * CellHeight - Height / 2.0f, CellWidth, CellHeight));
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent* Agent)
{
	UpdateAgentCell(Agent, {}, false);
}

void CellSpace::UpdateAgentCell(ASteeringAgent* Agent, const FVector2D& OldPos, bool hasOldPosition)
{
	if (hasOldPosition)
	{
		const int index = PositionToIndex(OldPos);
		const int new_index = PositionToIndex(Agent->GetPosition());

		if (index != new_index)
		{
			auto it = std::ranges::find(Cells[index].Agents, Agent);
			if (it != Cells[index].Agents.end())
				Cells[index].Agents.erase(it);

			UE_LOG(LogTemp, Warning, TEXT("From %d to %d"), index, new_index);
		}
		else
			return;
	}

	const int new_index = PositionToIndex(Agent->GetPosition());

	// UE_LOG(LogTemp, Warning, TEXT("The index is silly %d"), new_index);
	
	Cells[new_index].Agents.push_back(Agent);
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors = 0;
	
	const FVector2D position = Agent.GetPosition();
	const FRect agent_rect ( {position.X - QueryRadius, position.Y - QueryRadius}, {QueryRadius + position.X, QueryRadius + position.Y} );
	
	for (Cell& cell : Cells)
	{
		if (NrOfNeighbors >= MaxNeighbors)
			return;
		if (!DoRectsOverlap(cell.BoundingBox, agent_rect))
			continue;
		for (ASteeringAgent* agent : cell.Agents)
		{
			if (NrOfNeighbors >= MaxNeighbors)
				return;
			
			Neighbors[NrOfNeighbors] = agent;
			NrOfNeighbors++;
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	for (const Cell& cell : Cells)
	{
		const FVector2D extent = cell.BoundingBox.Max - cell.BoundingBox.Min;
		const FVector2D center = (cell.BoundingBox.Min + cell.BoundingBox.Max) / 2.0f;
		DrawDebugBox(pWorld, FVector(center.X, center.Y, 0.0f), FVector(extent.X, extent.Y, 0.0f), FColor(50, 255, 50));
	}
}

FVector2D CellSpace::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

	for (int index = 0; index < NrOfNeighbors; ++index)
	{
		avgPosition += Neighbors[index]->GetPosition();
	}

	return avgPosition / float(NrOfNeighbors);
}

FVector2D CellSpace::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

	for (int index = 0; index < NrOfNeighbors; ++index)
	{
		avgVelocity += Neighbors[index]->GetLinearVelocity();
	}

	return avgVelocity / float(NrOfNeighbors);
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	int x = static_cast<int>((Pos.X + SpaceWidth / 2.0f) / CellWidth);
	int y = static_cast<int>((Pos.Y + SpaceHeight / 2.0f) / CellHeight);

	x = FMath::Clamp(x, 0, this->NrOfRows - 1);
	y = FMath::Clamp(y, 0, this->NrOfCols - 1);
	
	return x + y * this->NrOfCols;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}