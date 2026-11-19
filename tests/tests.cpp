#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "graph.hpp"
#include "repository.hpp"
#include "utils/string.hpp"

namespace
{
using namespace PackageMan;
using Names = std::vector<std::string>;

int g_failed = 0;

void check(bool ok, const std::string& what)
{
    if (!ok)
    {
        ++g_failed;
        std::cerr << "  FAIL: " << what << '\n';
    }
}

void checkEq(const Names& actual, const Names& expected, const std::string& what)
{
    check(actual == expected,
          what + ": expected [" + StringUtils::joinStrings(expected, " ") + "], got [" +
              StringUtils::joinStrings(actual, " ") + "]");
}

TestRepository load(const std::string& file)
{
    auto repo = TestRepository::loadFromFile(std::string{TEST_DATA_DIR} + "/" + file);
    if (!repo)
    {
        std::cerr << "  cannot load " << file << ": " << repo.error() << '\n';
        std::exit(1);
    }
    return *std::move(repo);
}

void diamond()
{
    auto graph = buildGraph(load("diamond.txt"), "A");
    checkEq(graph.order, {"A", "B", "C", "D"}, "BFS order");
    checkEq(graph.edges.at("B"), {"D"}, "B edges");
    checkEq(graph.edges.at("C"), {"D"}, "C edges");
    checkEq(graph.missing, {}, "missing");
    checkEq(findCyclicNodes(graph), {}, "cyclic");
    checkEq(loadOrder(graph), {"D", "B", "C", "A"}, "load order");
}

void subgraph()
{
    // Only nodes reachable from the root are part of the graph
    auto graph = buildGraph(load("diamond.txt"), "C");
    checkEq(graph.order, {"C", "D"}, "BFS order");
    checkEq(loadOrder(graph), {"D", "C"}, "load order");
}

void cycle()
{
    auto graph = buildGraph(load("cycle.txt"), "A");
    checkEq(graph.order, {"A", "B", "C"}, "BFS order");
    checkEq(graph.edges.at("C"), {"A"}, "back edge kept");
    checkEq(findCyclicNodes(graph), {"A", "B", "C"}, "cyclic");
    checkEq(loadOrder(graph), {"C", "B", "A"}, "load order");
}

void selfLoop()
{
    auto graph = buildGraph(load("self.txt"), "A");
    checkEq(graph.order, {"A"}, "BFS order");
    checkEq(findCyclicNodes(graph), {"A"}, "cyclic");
    checkEq(loadOrder(graph), {"A"}, "load order");
}

void tailCycle()
{
    auto graph = buildGraph(load("tail_cycle.txt"), "A");
    checkEq(graph.order, {"A", "B", "D", "C"}, "BFS order");
    checkEq(findCyclicNodes(graph), {"A", "B", "C"}, "cyclic");
    checkEq(loadOrder(graph), {"C", "B", "D", "A"}, "load order");
}

void missing()
{
    auto graph = buildGraph(load("missing.txt"), "A");
    checkEq(graph.order, {"A", "B", "Z"}, "BFS order");
    checkEq(graph.missing, {"Z"}, "missing");
    checkEq(findCyclicNodes(graph), {}, "cyclic");
    checkEq(loadOrder(graph), {"Z", "B", "A"}, "load order");
}

void missingRoot()
{
    auto graph = buildGraph(load("diamond.txt"), "Q");
    checkEq(graph.order, {"Q"}, "BFS order");
    checkEq(graph.missing, {"Q"}, "missing");
}

void chain()
{
    auto graph = buildGraph(load("chain.txt"), "A");
    check(graph.order.size() == 26, "all 26 packages visited");
    check(loadOrder(graph).front() == "Z" && loadOrder(graph).back() == "A", "Z first, A last");
}

void mermaid()
{
    auto text = toMermaid(buildGraph(load("cycle.txt"), "A"));
    check(text.starts_with("graph TD\n"), "header");
    check(text.contains("n0[\"A\"]"), "node label");
    check(text.contains("n0 --> n1"), "edge A -> B");
    check(text.contains("n2 --> n0"), "back edge C -> A");
    check(text.contains("class n0 cyclic"), "cycle highlighted");

    auto missingText = toMermaid(buildGraph(load("missing.txt"), "A"));
    check(missingText.contains("style n2 stroke-dasharray"), "missing node dashed");
    check(!missingText.contains("cyclic"), "no cycle class");
}

void loadErrors()
{
    check(!TestRepository::loadFromFile(std::string{TEST_DATA_DIR} + "/nope.txt"), "missing file");
    check(!TestRepository::loadFromFile(std::string{TEST_DATA_DIR} + "/broken.txt"),
          "line without ':'");
}
} // namespace

int main()
{
    const std::vector<std::pair<const char*, std::function<void()>>> tests{
        {"diamond", diamond},
        {"subgraph", subgraph},
        {"cycle", cycle},
        {"self loop", selfLoop},
        {"tail cycle", tailCycle},
        {"missing", missing},
        {"missing root", missingRoot},
        {"chain", chain},
        {"mermaid", mermaid},
        {"load errors", loadErrors},
    };

    for (const auto& [name, test] : tests)
    {
        int before = g_failed;
        test();
        std::cout << (g_failed == before ? "[ OK ] " : "[FAIL] ") << name << '\n';
    }

    std::cout << (g_failed == 0 ? "All tests passed\n" : "Some tests failed\n");
    return g_failed == 0 ? 0 : 1;
}
