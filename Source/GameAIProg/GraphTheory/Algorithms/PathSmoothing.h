#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
	{
		if (Path.size() < 2)
			return {};
		
		//Container
		std::vector<NavLine> Portals = {};

		for (int index = 1; index < Path.size(); ++index)
		{
			NavGraphNode* last = (NavGraphNode*)Path[index - 1];
			NavGraphNode* current = (NavGraphNode*)Path[index];

			FVector2D direction = current->GetPosition() - last->GetPosition();
			
			int edgeIdx = current->GetEdgeIdx();

			if (edgeIdx < 0)
				continue;
			
			TriPolygon::Edge edge = NavPoly.GetEdges()[edgeIdx];

			FVector _fvp1 = edge.GetP1(NavPoly);
			FVector _fvp2 = edge.GetP2(NavPoly);
			FVector2D p1 = { _fvp1.X, _fvp1.Y };
			FVector2D p2 = { _fvp2.X, _fvp2.Y };

			double compare_p1{FVector2D::CrossProduct(direction, p1 - last->GetPosition())};

			// should never be 0.0, otherwise that means the edge has length 0.0

			if (compare_p1 > 0.0f)
			{
				Portals.emplace_back(NavLine{ p2, p1 });
			}
			else
			{
				Portals.emplace_back(NavLine{ p1, p2 });
			}
		}
		
		//For each node received, get it's corresponding line
		
			//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point

			//Store portal

		//Add degenerate portal to force end evaluation

		return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		if (Portals.empty())
			return {};
		
		std::vector<FVector2D> Path{};

		auto crossCheck = [](const FVector2D& a, const FVector2D& b, const FVector2D& c) -> double
		{
			return FVector2D::CrossProduct(b - a, c - a);
		};

		auto nearEq = [](const FVector2D& a, const FVector2D& b, double eps = 0.001) -> bool
		{
			return (a - b).SizeSquared() <= eps * eps;
		};

		FVector2D portalApex = (Portals[0].P1 + Portals[0].P2) * 0.5f;
		FVector2D portalLeft = Portals[0].P2;
		FVector2D portalRight = Portals[0].P1;
		
		int leftIndex = 0;
		int rightIndex = 0;

		Path.push_back(portalApex);

		for (int i = 1; i < Portals.size(); ++i)
		{
			const FVector2D left = Portals[i].P2;
			const FVector2D right = Portals[i].P1;
			
			if (crossCheck(portalApex, portalRight, right) <= 0.0)
			{
				if (nearEq(portalApex, portalRight) || crossCheck(portalApex, portalLeft, right) > 0.0)
				{
					portalRight = right;
				}
				else
				{
					Path.push_back(portalRight);
					portalApex = portalLeft;
					portalLeft = portalApex;
					portalRight = portalApex;
				}
			}

			if (crossCheck(portalApex, portalLeft, left) >= 0.0)
			{
				if (nearEq(portalApex, portalLeft) || crossCheck(portalApex, portalRight, left) < 0.0)
				{
					portalLeft = left;
				}
				else
				{
					Path.push_back(portalLeft);
					portalApex = portalRight;
					portalLeft = portalApex;
					portalRight = portalApex;
				}
			}
		}

		const FVector2D endPoint = (Portals.back().P1 + Portals.back().P2) * 0.5f;

		if (Path.empty() || !nearEq(Path.back(), endPoint))
			Path.push_back(endPoint);
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			
				//2. See if new line degenerates a line segment - RIGHT
				
					//Leftleg becomes new apex point

					//Calculate new legs (if not the end)


			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT

				//2. See if new line degenerates a line segment - LEFT

					//Rightleg becomes new apex point

					//Calculate new legs (if not the end)


		// Add last path point

		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
