#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//Create the path to return
	std::vector<FVector2D> finalPath{};

	FVector2D outTrigStart{};
	TriPolygon::Triangle const* trigStart = pNavGraph->GetNavPolygon()->GetClosestTriangleToPosition(startPos, outTrigStart);

	FVector2D outTrigEnd{};
	TriPolygon::Triangle const* trigEnd = pNavGraph->GetNavPolygon()->GetClosestTriangleToPosition(startPos, outTrigEnd);

	if (trigStart == trigEnd)
		return std::vector<FVector2D>();

	//We have valid start/end triangles and they are not the same
	//=> Start looking for a path
	//Copy the graph

	int startNodeIdx = Graphs::InvalidNodeId, endNodeIdx = Graphs::InvalidNodeId;

	std::unique_ptr<NavGraph> graph = pNavGraph->Clone();

	for (TriPolygon::Edge& edge : trigStart->GetEdges())
	{
		std::optional<int> hasEdge = pNavGraph->GetNavPolygon()->FindEdgeIndex(edge);
		if (hasEdge.has_value())
		{
			int edgeNodeIdx = pNavGraph->GetNodeIdFromEdgeIndex(hasEdge.value());

			std::unique_ptr<NavGraphNode> node = std::make_unique<NavGraphNode>(startPos, Graphs::InvalidNodeId);
			int nodeIdx = startNodeIdx = graph->AddNode(std::move(node));

			graph->AddConnection(nodeIdx, edgeNodeIdx);
			break;
		}
	}

	for (TriPolygon::Edge& edge : trigEnd->GetEdges())
	{
		std::optional<int> hasEdge = pNavGraph->GetNavPolygon()->FindEdgeIndex(edge);
		if (hasEdge.has_value())
		{
			int edgeNodeIdx = pNavGraph->GetNodeIdFromEdgeIndex(hasEdge.value());

			std::unique_ptr<NavGraphNode> node = std::make_unique<NavGraphNode>(endPos, Graphs::InvalidNodeId);
			int nodeIdx = endNodeIdx = graph->AddNode(std::move(node));

			graph->AddConnection(edgeNodeIdx, nodeIdx);
			break;
		}
	}

	if (startNodeIdx == Graphs::InvalidNodeId || endNodeIdx == Graphs::InvalidNodeId)
		return std::vector<FVector2D>();

	//Run A star on new graph

	AStar aStar(graph.get(), HeuristicFunctions::SqEuclidean);

	std::vector<Node*> path = aStar.FindPath(graph->GetNode(startNodeIdx).get(), graph->GetNode(endNodeIdx).get());

	for (Node* node : path)
	{
		finalPath.emplace_back(node->GetPosition());
	}

	//Debug Visualisation

	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	// debugPortals = SSFA::FindPortals(nodes, *pNavGraph->GetNavPolygon());
	// finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}