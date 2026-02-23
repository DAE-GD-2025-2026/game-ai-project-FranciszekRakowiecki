#include "SpacePartitioning.h"
#include "DrawDebugHelpers.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
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
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	const float Left = Width / 2.0f;
	const float Top = Height / 2.0f;

	for (int Index_Y = 0; Index_Y < Rows; ++Index_Y)
	{
		for (int Index_X = 0; Index_X < Cols; ++Index_X)
		{
			Cells.emplace_back(Cell(Left, Top, Width, Height));
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	// TODO Add the agent to the correct cell
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	//TODO Check if the agent needs to be moved to another cell.
	//TODO Use the calculated index for oldPos and currentPos for this

	
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	// TODO Register the neighbors for the provided agent
	// TODO Only check the cells that are within the radius of the neighborhood
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// TODO Render the cells with the number of agents inside of it

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

	const float factor = 1.0f / float(NrOfNeighbors);

	for (int index = 0; index < NrOfNeighbors; ++index)
	{
		avgPosition += Neighbors[index]->GetPosition() * factor;
	}

	return avgPosition;
}

FVector2D CellSpace::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

	const float factor = 1.0f / float(NrOfNeighbors);

	for (int index = 0; index < NrOfNeighbors; ++index)
	{
		avgVelocity += Neighbors[index]->GetLinearVelocity() * factor;
	}

	return avgVelocity;
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{

	int x = (Pos.X / CellWidth) * this->NrOfCols;
	int y = (Pos.Y / CellHeight) * this->NrOfRows;
	
	return x + y * this->NrOfRows;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}