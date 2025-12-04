/*
Anusheel Nand
TSP Implementation
EEC 289Q, Fall 2025
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <list>
#include <limits>
#include <chrono>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <cstdlib>
#include <iterator>

std::vector<std::vector<double>> parseGraph(std::string fileName);
void printGraph(std::vector<std::vector<double>>& graph);
bool checkTime(std::chrono::high_resolution_clock::time_point startTime);
void writeBestCycle (std::vector<int> cycle);
void checkCycle(std::vector<int> bestCycle, std::vector<std::vector<double>>& graph);
void bestGreedyCycle(std::vector<std::vector<double>>& graph);
void shuffleNodes(std::vector<std::vector<double>>& graph);
void checkCycleCost(std::vector<int> cycle, std::vector<std::vector<double>>& graph);

int numNodes = 0;
double timeLimit = 57.0; // Time limit, ~60 seconds with breathing room

std::vector<int> bestCycle;
double minCost = std::numeric_limits<double>::max();
int cycleCount = 0;
std::chrono::high_resolution_clock::time_point startTime;

int main(int argc, char* argv[])
{
    // Start clock
    startTime = std::chrono::high_resolution_clock::now();

    if (argc != 2)
    {
        std::cout << "Input atmost one graph at a time" << std::endl;
        exit(1);
    }

    // Create adjacency matrix from file in argument
    std::vector<std::vector<double>> graph = parseGraph(argv[1]);
    // printGraph(graph);

    bestGreedyCycle(graph);
    shuffleNodes(graph);
    writeBestCycle(bestCycle);
    std::cout << "Min cost: " << minCost << std::endl;
    checkCycle(bestCycle, graph);
}

void bestGreedyCycle(std::vector<std::vector<double>>& graph)
{
    // Populate unvisited starting nodes
    std::vector<int> unvisitedStartNodes;
    for (int i = 1; i <= numNodes; i++)
    {
        unvisitedStartNodes.emplace_back(i);
    }

    // Randomizer from https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
    std::random_device random;
    std::mt19937 generator(random());

    while(checkTime(startTime) == false && !unvisitedStartNodes.empty())
    {
        std::vector<int> cycle;
        cycleCount++;
        double cost = 0.0;
        
        // Get a random node
        std::uniform_int_distribution<> start(0, unvisitedStartNodes.size()-1);
        size_t rand_idx = start(generator);
        int startNode = unvisitedStartNodes[rand_idx];

        // Don't start with this node again
        std::swap(unvisitedStartNodes[rand_idx], unvisitedStartNodes.back());
        unvisitedStartNodes.pop_back();

        // Populate unvisited set
        std::unordered_set<int> unvisitedNodes;
        for (int i = 1; i <= numNodes; i++)
        {
            unvisitedNodes.insert(i);
        }
        unvisitedNodes.erase(startNode);

        cycle.emplace_back(startNode);
        int nextNode = startNode;
        while(!unvisitedNodes.empty() && cost < minCost)
        {
            int bestNeighbor;
            double nextNodeCost = std::numeric_limits<double>::max();
            for(int i : unvisitedNodes)
            {
                if(graph[nextNode][i] < nextNodeCost)
                {
                    nextNodeCost = graph[nextNode][i];
                    bestNeighbor = i;
                }
            }
            int prevNode = nextNode;
            nextNode = bestNeighbor;
            cycle.emplace_back(nextNode);
            cost += graph[prevNode][nextNode];
            unvisitedNodes.erase(nextNode);
        }
        cost += graph[nextNode][startNode];
        cycle.emplace_back(startNode);
        if(cost < minCost)
        {
            minCost = cost;
            bestCycle = cycle;
        }

    }
}

void shuffleNodes(std::vector<std::vector<double>>& graph)
{
    // Shuffling 3 end nodes requires atleast 7 nodes in bestCycle
    if(numNodes < 7)
    {
        return;
    }

    // Randomizer from https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
    std::random_device dev;
    std::mt19937 generator(dev());
    std::uniform_int_distribution<> randNodes(0, bestCycle.size()-3); // Avoid shuffling last node (return to starting node)

    while (checkTime(startTime) == false)
    {

        size_t randNode1 = randNodes(generator);
        size_t randNode2 = randNodes(generator);
        size_t randNode3 = randNodes(generator);

        // // Ensure no shared nextNode
        if((randNode1 == randNode2) || (randNode1 == randNode3) || (randNode2 == randNode3))
        {
            continue;
        }

        // Allocating ahead of time increases total # of cycles explored!
        std::vector<int> cycle1 = bestCycle;
        std::vector<int> cycle2 = bestCycle;
        std::vector<int> cycle3 = bestCycle;
        std::vector<int> cycle4 = bestCycle;
        std::vector<int> cycle5 = bestCycle;

        // Permutation 1 (2,3,1)
        int tmp = cycle1[randNode1+1];
        cycle1[randNode1+1] = cycle1[randNode2+1];
        cycle1[randNode2+1] = cycle1[randNode3+1];
        cycle1[randNode3+1] = tmp;
        checkCycleCost(cycle1, graph);

        // Permutation 2 (2,1,3)
        tmp = cycle2[randNode1+1];
        cycle2[randNode1+1] = cycle2[randNode2+1];
        cycle2[randNode2+1] = tmp;
        checkCycleCost(cycle2, graph);

        // Permutation 3 (3,1,2)
        tmp = cycle3[randNode1+1];
        cycle3[randNode1+1] = cycle3[randNode3+1];
        cycle3[randNode3+1] = cycle3[randNode2+1];
        cycle3[randNode2+1] = tmp;
        checkCycleCost(cycle3, graph);

        // Permutation 4 (3,2,1)
        tmp = cycle4[randNode1+1];
        cycle4[randNode1+1] = cycle4[randNode3+1];
        cycle4[randNode3+1] = tmp;
        checkCycleCost(cycle4, graph);

        // Permutation 5 (1,3,2)
        tmp = cycle5[randNode2+1];
        cycle5[randNode2+1] = cycle5[randNode3+1];
        cycle5[randNode3+1] = tmp;
        checkCycleCost(cycle5, graph);
    }
}

std::vector<std::vector<double>> parseGraph(std::string fileName)
{
    std::ifstream txt(fileName);
    if (txt.fail())
    {
        std::cout << "Invalid file" << std::endl;
        exit(1);
    }

    std::string txtHeader;

    txt >> numNodes;
    std::getline(txt >> std::ws, txtHeader);
    // std::cout << "# Nodes = " << numNodes << std::endl;
    // std::cout << "File header: " << txtHeader << std::endl;

    // Allocate numNodes + 1 so we don't have to deal with 0-indexed matrix vs 1-indexed node issues
    std::vector<std::vector<double>> graph(numNodes+1, std::vector<double>(numNodes+1));

    int i, j;
    double cost;
    while (txt >> i >> j >> cost)
    {
        graph[i][j] = cost;
        graph[j][i] = cost;
    }
    txt.close();
    return graph;
}

void printGraph(std::vector<std::vector<double>>& graph)
{
    for (int i = 1; i <= numNodes; i++)
    {
        std::cout << "[";
        for (int j = 1; j <= numNodes; j++)
        {
            std::cout << graph[i][j] << " ";
        }
        std::cout << "]" << std::endl;
    }

}

void writeBestCycle (std::vector<int> cycle)
{
    std::cout << "Cycles evaluated: " << std::scientific << std::setprecision(0) << static_cast<double>(cycleCount) << std::endl;
    std::cout << std::fixed << std::setprecision(2); // Set to 2-decimal format for minCost
    std::ofstream cycleTxt ("solution_922092536.txt");
    if(cycleTxt.fail())
    {
        std::cout << "Invalid output file" << std::endl;
        exit(1);
    }
    for (auto node = cycle.begin(); node != cycle.end(); node++)
    {
        if(std::next(node) == cycle.end())
        {
            cycleTxt << *node << std::endl;
        }
        else
        {
            cycleTxt << *node << ", ";
        }
    }
    cycleTxt.close();
}

void checkCycle (std::vector<int> cycle, std::vector<std::vector<double>>& graph)
{
    std::cout << std::endl;
    std::cout << "Check if a valid TSP tour was found" << std::endl;
    std::vector<int> cycleToCheck = cycle;
    if(cycleToCheck.front() == cycleToCheck.back())
    {
        std::cout << "Complete cycle!" << std::endl;
    }

    double costCheck = 0;
    int lastNode = cycleToCheck.front();
    for (int node : cycleToCheck)
    {
        costCheck += graph[lastNode][node];
        lastNode = node;
        // std::cout << " -> " << lastNode;
    }
    std::cout << "costCheck: " << costCheck << " ";
    if(costCheck == minCost)
    {
        std::cout << "matches!" << std::endl;
    }
    else
    {
        std::cout << "doesn't match!" << std::endl;
    }

    cycleToCheck.pop_back(); // Make sorting and checking easier
    std::sort(cycleToCheck.begin(), cycleToCheck.end());
    std::vector<int> validTSP;
    for (int i = 1; i <= numNodes; i++)
    {
        validTSP.emplace_back(i);
    }
    if(cycleToCheck == validTSP)
    {
        std::cout << "Valid TSP tour!" << std::endl;
    }
    else
    {
        std::cout << "Invalid TSP tour!" << std::endl;
    }

}

void checkCycleCost(std::vector<int> cycle, std::vector<std::vector<double>>& graph)
{
    cycleCount++;
    int lastNode = cycle.front();
    double costCheck = 0.0;
    for (size_t i = 1; i < cycle.size(); i++)
    {
        costCheck += graph[lastNode][cycle[i]];
        lastNode = cycle[i];
        // std::cout << " -> " << lastNode;
    }
    if(costCheck < minCost)
    {
        minCost = costCheck;
        bestCycle = cycle;
    }
}

bool checkTime(std::chrono::high_resolution_clock::time_point startTime)
{
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = now - startTime;
    return duration.count() >= timeLimit;
}