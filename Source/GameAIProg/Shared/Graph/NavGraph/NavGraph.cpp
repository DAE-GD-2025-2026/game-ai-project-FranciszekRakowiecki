#include "NavGraph.h"

#include "NavGraphNode.h"
#include "GraphTheory/Algorithms/Heuristics.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigation mesh and create nodes
			// Create node here

	int EdgeIdx = 0;
	for (TriPolygon::Edge const & Edge : pNavPoly->GetEdges())
	{
		FVector position = (Edge.GetP1(*pNavPoly) + Edge.GetP2(*pNavPoly)) * 0.5;
		std::unique_ptr<Node> pNode = std::make_unique<NavGraphNode>(FVector2D{ position.X, position.Y }, EdgeIdx);
		AddNode(std::move(pNode));
		EdgeIdx++;
	}

	std::vector<int> neighbors{};
	
	for (TriPolygon::Triangle const & Triangle : pNavPoly->GetTriangles())
	{
		neighbors = Triangle.GetNeighbors(*pNavPoly);

		std::array<TriPolygon::Edge, 3> edges = Triangle.GetEdges();

		std::vector<int> validTriangleNodes{};

		for (int tIdx : neighbors)
		{
			TriPolygon::Triangle const& neighbor{ pNavPoly->GetTriangle(tIdx) };

			std::array<TriPolygon::Edge, 3> neighborEdges = neighbor.GetEdges();

			std::vector<TriPolygon::Edge> sharedEdges {};
			
			std::set_intersection(edges.begin(), edges.end(), neighborEdges.begin(), neighborEdges.end(), std::back_inserter(sharedEdges));

			if (sharedEdges.size() > 0)
			{
				TriPolygon::Edge sharedEdge {sharedEdges[0]};

				std::optional<int> opt = pNavPoly->FindEdgeIndex(sharedEdge);
				if (!opt.has_value())
					continue;

				int nodeidx = GetNodeIdFromEdgeIndex(opt.value());

				if (nodeidx == Graphs::InvalidNodeId)
				{
					FVector position = (sharedEdge.GetP1(*pNavPoly) + sharedEdge.GetP2(*pNavPoly)) * 0.5;
					std::unique_ptr<Node> pNode = std::make_unique<NavGraphNode>(FVector2D{ position.X, position.Y }, opt.value());
					nodeidx = AddNode(std::move(pNode));
				}

				validTriangleNodes.push_back(nodeidx);
			}
		}

		if (validTriangleNodes.size() == 2)
		{
			AddConnectionWithWeight(validTriangleNodes[0], validTriangleNodes[1]);
		}
		else if (validTriangleNodes.size() == 3)
		{
			AddConnectionWithWeight(validTriangleNodes[0], validTriangleNodes[1]);
			AddConnectionWithWeight(validTriangleNodes[1], validTriangleNodes[2]);
			AddConnectionWithWeight(validTriangleNodes[2], validTriangleNodes[0]);
		}
	}
}

void GameAI::NavGraph::AddConnectionWithWeight(int NodeId1, int NodeId2)
{
	std::unique_ptr<Connection> connection{std::make_unique<Connection>(NodeId1, NodeId2)};
	
	Node* node1 = GetNode(NodeId1).get();
	Node* node2 = GetNode(NodeId2).get();

	FVector2D difference = node1->GetPosition() - node2->GetPosition();
			
	connection->SetWeight(HeuristicFunctions::Euclidean(difference.X, difference.Y));

	AddConnection(std::move(connection));
}
