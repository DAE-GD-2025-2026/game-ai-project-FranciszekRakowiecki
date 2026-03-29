#include "BFS.h"

#include <map>
#include <queue>
#include <stack>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

struct BFS::BFS_Jump
{
	Node* Self;
	Node* Previous;
};

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

// TODO Breath First Search Algorithm searches for a path from the startNode to the destinationNode
std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	if (pStartNode == pDestinationNode)
		return std::vector<Node*>();
	std::queue<Node*> OpenList;
	std::vector<BFS_Jump> ClosedList;

	OpenList.push(pStartNode);
	ClosedList.emplace_back(BFS_Jump{ pStartNode, nullptr });

	while (!OpenList.empty())
	{
		Node* pCurrentNode = OpenList.front();
		OpenList.pop();

		if (pCurrentNode == pDestinationNode)
			return BackTrack(ClosedList, pCurrentNode, pStartNode);
		std::vector<Connection*> connections = pGraph->FindConnectionsFrom(pCurrentNode->GetId());

		for (Connection* conn : connections)
		{
			Node* child = pGraph->GetNode(conn->GetToId()).get();
			auto isNodeAlreadyChecked = [child](const BFS_Jump& j)
			{
				return j.Self == child;
			};
			if (std::ranges::find_if(ClosedList.begin(), ClosedList.end(), isNodeAlreadyChecked) != ClosedList.end())
			{
				ClosedList.emplace_back(BFS_Jump{ child, pCurrentNode });
				OpenList.emplace(child);
			}
		}
	}
	
	return std::vector<Node*>();
}

std::vector<Node*> BFS::BackTrack(std::vector<BFS_Jump>& PastNodes, Node* start, Node* end) const
{
	if (start == end)
		return std::vector<Node*>();
	std::vector<Node*> path;

	Node* pCurrentNode = start;

	while (pCurrentNode != end)
	{
		auto it = std::ranges::find_if(PastNodes.begin(), PastNodes.end(), [pCurrentNode](const BFS_Jump& j) { return j.Self == pCurrentNode; });
		if (it == PastNodes.end())
		{
			UE_LOG(LogTemp, Error, TEXT("There is no path to the begin node."));
			return std::vector<Node*>();
		}

		path.emplace_back(pCurrentNode);
		pCurrentNode = (*it).Previous;
	}

	path.emplace_back(end);

	std::ranges::reverse(path); // because we are going backwards
	
	return path;
}
