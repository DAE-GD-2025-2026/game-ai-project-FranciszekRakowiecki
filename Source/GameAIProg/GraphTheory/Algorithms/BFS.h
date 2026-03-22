#pragma once
#include <vector>

namespace GameAI
{
	class Graph;
	class Node;

	class BFS
	{
	public:
		BFS(Graph* const pGraph);

		std::vector<Node*> FindPath(Node* const pStartNode, Node* const pDestinationNode) const;

	private:
		Graph* pGraph;
		struct BFS_Jump;

		std::vector<Node*> BackTrack(std::vector<BFS_Jump>& PastNodes, Node* start, Node* end) const;
	};
}
