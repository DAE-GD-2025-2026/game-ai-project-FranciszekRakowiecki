#pragma once
#include <queue>
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		// TODO If the graph is not connected, there can be no Eulerian Trail

		if (!IsConnected())
			return Eulerianity::notEulerian;

		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return Eulerianity::notEulerian;

		// TODO Count nodes with odd degree

		int odd_count = 0;

		for (Node* pNode : Nodes)
		{
			std::vector<Connection*> connections{ m_pGraph->FindConnectionsWith(pNode->GetId()) };

			if ((connections.size() & 1) == 1) // is odd " an odd degree"
			{
				odd_count++;
			}
		}

		if (odd_count > 2) // "A connected graph with more than 2 nodes with an odd degree"
			return Eulerianity::notEulerian;

		if (odd_count == 2 && Nodes.size() > 2)
			return Eulerianity::semiEulerian;

		//  A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian

		//  A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		//  An Euler trail can be made, but only starting and ending in these 2 nodes

		//  A connected graph with no odd nodes is Eulerian
		
		return Eulerianity::eulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		
		if (eulerianity == Eulerianity::notEulerian)
			return std::vector<Node*>();
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };

		if (eulerianity == Eulerianity::semiEulerian)
		{
			
		}
		
		// TODO Start algorithm loop
		std::stack<int> nodeStack;

		

		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	// The tip says to use a loop but then if its an index from the node vector then why not just access it directly, unless
	// the start index is actually the id then well too bad this is going to use the index from the vector not the id
	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex ) const
	{
		// TODO Mark the visited node

		std::queue<Node*> toCheck{}; // DFS -> so that the child nodes get checked before child's child connections get checked

		toCheck.push(Nodes[startIndex]);

		while (!toCheck.empty())
		{
			Node* node = toCheck.front();
			toCheck.pop();

			visited[node->GetId()] = true;

			std::vector<Connection*> connections = m_pGraph->FindConnectionsFrom(node->GetId());

			for (Connection* connection : connections)
			{
				if (visited[connection->GetToId()])
					continue;

				toCheck.push(m_pGraph->GetNode(connection->GetToId()).get());
			}
		}

		// TODO Ask the graph for the connections from that node
		// TODO recursively visit any valid connected nodes that were not visited before
		// TODO Tip: use an index-based for-loop to find the correct index
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		std::vector<bool> visited{};
		visited.resize(Nodes.size());

		std::queue<Node*> toCheck{};

		// Find node with at least one connection.

		for (Node* node : Nodes)
		{
			if (m_pGraph->FindConnectionsFrom(node->GetId()).size() > 0)
			{
				toCheck.push(node);
				break;
			}
		}

		while (!toCheck.empty())
		{
			Node* node = toCheck.front();
			toCheck.pop();

			visited[node->GetId()] = true;

			std::vector<Connection*> connections = m_pGraph->FindConnectionsFrom(node->GetId());

			for (Connection* connection : connections)
			{
				if (visited[connection->GetToId()])
					continue;

				toCheck.push(m_pGraph->GetNode(connection->GetToId()).get());
			}
		}

		for (bool isVisited : visited)
		{
			if (!isVisited) // if even one node isn't visited then the graph is not connected.
				return false;
		}

		return true;
	}
}