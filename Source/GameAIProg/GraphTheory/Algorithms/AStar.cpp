#include "AStar.h"

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};

	std::vector<NodeRecord> openList{};
	std::vector<NodeRecord> closedList{};
	
	Node* currentNode = pStartNode;
	openList.emplace_back(NodeRecord{ currentNode, nullptr, 0.0f, GetHeuristicCost(currentNode, pGoalNode) });

	bool foundPath = false;

	while (openList.size() > 0)
	{
		auto it = std::min_element(openList.begin(), openList.end());
		NodeRecord currentNodeRecord = *it;
		openList.erase(it);
		currentNode = currentNodeRecord.pNode;

		closedList.emplace_back(currentNodeRecord);

		if (currentNode == pGoalNode)
		{
			foundPath = true;
			break;
		}

		std::vector<Connection*> connections{pGraph->FindConnectionsWith(currentNode->GetId())};
		for (Connection* connection : connections)
		{
			Connection conn(*connection);

			if (conn.GetFromId() != currentNode->GetId())
				conn = conn.GetInverseCopy();
			
			Node* connectedNode{ pGraph->GetNode(conn.GetToId()).get()};
			auto it = std::find(closedList.begin(), closedList.end(), [connectedNode](const NodeRecord& other) { return other.pNode == connectedNode; });
			if (it == closedList.end())
			{
				openList.emplace_back(NodeRecord{ connectedNode, connection, currentNodeRecord.estimatedTotalCost, currentNodeRecord.estimatedTotalCost + GetHeuristicCost(connectedNode, pGoalNode) });

				// perhaps limit the max distance that a search can go to so that there would be another exit condition and so that the search doesn't go on forever
			}
		}
	}

	if (foundPath)
	{
		NodeRecord& record = closedList.back();
		
		while (record.pNode != pStartNode)
		{
			path.push_back(record.pNode);

			if (record.pConnection == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find start node??"));
				return std::vector<Node*>{};
			}

			int idx = record.pConnection->GetFromId() == record.pNode->GetId() ? record.pConnection->GetToId() : record.pConnection->GetFromId();
			
			Node* next = pGraph->GetNode(idx).get();

			record = *std::find(closedList.begin(), closedList.end(), [next](const NodeRecord& other) { return other.pNode == next; });
		}

		std::reverse(path.begin(), path.end());
	}
	
	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}