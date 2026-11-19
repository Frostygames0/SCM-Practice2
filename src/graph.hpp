#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "repository.hpp"

namespace PackageMan
{
struct DependencyGraph
{
    std::string root;
    std::unordered_map<std::string, std::vector<std::string>> edges; // package -> direct deps
    std::vector<std::string> order;                                  // BFS visit order
    std::vector<std::string> missing;                                // not found in repository
};

DependencyGraph buildGraph(const Repository& repo, const std::string& root);
std::vector<std::string> findCyclicNodes(const DependencyGraph& graph);
std::vector<std::string> loadOrder(const DependencyGraph& graph);

std::string toMermaid(const DependencyGraph& graph);
} // namespace PackageMan
