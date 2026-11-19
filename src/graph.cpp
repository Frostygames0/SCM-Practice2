#include "graph.hpp"

#include <format>
#include <queue>
#include <stack>
#include <unordered_set>

namespace PackageMan
{
DependencyGraph buildGraph(const Repository& repo, const std::string& root)
{
    DependencyGraph graph{.root = root};
    std::queue<std::string> queue;
    std::unordered_set<std::string> visited{root};
    queue.push(root);

    while (!queue.empty())
    {
        auto current = std::move(queue.front());
        queue.pop();
        graph.order.push_back(current);

        auto deps = repo.getDependencies(current);
        if (!deps)
        {
            graph.missing.push_back(current);
            continue;
        }

        for (const auto& dep : *deps)
        {
            if (visited.insert(dep).second)
            {
                queue.push(dep);
            }
        }
        graph.edges[current] = std::move(*deps);
    }

    return graph;
}

std::vector<std::string> findCyclicNodes(const DependencyGraph& graph)
{
    // Reverse Kahn: repeatedly remove nodes with no outgoing edges (leaves)
    std::unordered_map<std::string, std::size_t> outDegree;
    std::unordered_map<std::string, std::vector<std::string>> dependents;
    for (const auto& node : graph.order)
    {
        outDegree[node] = 0;
    }
    for (const auto& [node, deps] : graph.edges)
    {
        outDegree[node] = deps.size();
        for (const auto& dep : deps)
        {
            dependents[dep].push_back(node);
        }
    }

    std::queue<std::string> queue;
    for (const auto& [node, degree] : outDegree)
    {
        if (degree == 0)
        {
            queue.push(node);
        }
    }

    while (!queue.empty())
    {
        auto node = std::move(queue.front());
        queue.pop();
        for (const auto& parent : dependents[node])
        {
            if (--outDegree[parent] == 0)
            {
                queue.push(parent);
            }
        }
    }

    std::vector<std::string> cyclic;
    for (const auto& node : graph.order)
    {
        if (outDegree[node] != 0)
        {
            cyclic.push_back(node);
        }
    }
    return cyclic;
}

std::vector<std::string> loadOrder(const DependencyGraph& graph)
{
    static const std::vector<std::string> s_noDeps;

    auto depsOf = [&](const std::string& node) -> const std::vector<std::string>& {
        auto it = graph.edges.find(node);
        return it != graph.edges.end() ? it->second : s_noDeps;
    };

    std::vector<std::string> order;
    std::unordered_set<std::string> visited{graph.root};
    std::stack<std::pair<std::string, std::size_t>> stack; // node, next dep index
    stack.emplace(graph.root, 0);

    while (!stack.empty())
    {
        auto& [node, idx] = stack.top();
        const auto& deps = depsOf(node);
        if (idx < deps.size())
        {
            const auto& dep = deps[idx++];
            if (visited.insert(dep).second)
            {
                stack.emplace(dep, 0);
            }
            continue;
        }
        order.push_back(node);
        stack.pop();
    }

    return order;
}

std::string toMermaid(const DependencyGraph& graph)
{
    // Node ids must be plain identifiers; package names may contain '-', '.', '+', ':'
    std::unordered_map<std::string, std::string> ids;
    for (std::size_t i = 0; i < graph.order.size(); ++i)
    {
        ids.emplace(graph.order[i], std::format("n{}", i));
    }

    std::string out = "graph TD\n";
    for (const auto& node : graph.order)
    {
        out += std::format("    {}[\"{}\"]\n", ids.at(node), node);
    }
    for (const auto& node : graph.order)
    {
        if (auto it = graph.edges.find(node); it != graph.edges.end())
        {
            for (const auto& dep : it->second)
            {
                out += std::format("    {} --> {}\n", ids.at(node), ids.at(dep));
            }
        }
    }

    auto cyclic = findCyclicNodes(graph);
    if (!cyclic.empty())
    {
        out += "    classDef cyclic fill:#f99,stroke:#c00\n";
        for (const auto& node : cyclic)
        {
            out += std::format("    class {} cyclic\n", ids.at(node));
        }
    }
    for (const auto& node : graph.missing)
    {
        out += std::format("    style {} stroke-dasharray: 5 5\n", ids.at(node));
    }

    return out;
}
} // namespace PackageMan
