#include <iostream>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

#include <kendall.h>
#include <grank.h>
#include <grankMulti.h>
#include <mccompletepathv2.h>
#include <pprSingleSource.h>
#include <benchmarkAlgorithm.h>

using namespace std;
using ppr::grank;
using ppr::grankMulti;
using ppr::mccompletepathv2;
using ppr::pprInternal::pprSingleSource;
using ppr::benchmarkAlgorithm;


/**
 * Imports a direct graph from a csv, every line is an edge in the form of:
 * node1, node2
 */
unordered_map<int, vector<int>> importGraph(string fname);
unordered_map<string, vector<string>> importGraph2(string fname);

int main()
{
  unordered_map<int, vector<int>> graph = importGraph("gnutella30.csv");
  //unordered_map<string, vector<string>> graph = importGraph2("gnutella30.csv");

  //grank multi
  {
    auto begin = std::chrono::steady_clock::now();
    auto map = grankMulti(graph, 50, 100, 30, 0.85, 0.0001, 4);
    auto end= std::chrono::steady_clock::now();
    std::cout << "grank run-time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << endl;

    auto bench = benchmarkAlgorithm(map, graph, 200, true);
    cout << "-------" << endl;
    for(auto& keyVal: bench)
      cout << keyVal.first << "     " << keyVal.second << endl;
    cout << "-------" << endl;
  }

  //grank
  {
    auto begin = std::chrono::steady_clock::now();
    auto map = grank(graph, 50, 100, 30, 0.85, 0.0001);
    auto end= std::chrono::steady_clock::now();
    std::cout << "grank run-time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << endl;

    auto bench = benchmarkAlgorithm(map, graph, 200, true);
    cout << "-------" << endl;
    for(auto& keyVal: bench)
      cout << keyVal.first << "     " << keyVal.second << endl;
    cout << "-------" << endl;
  }

  //mc
  {
    auto begin2 = std::chrono::steady_clock::now();
    auto map2 = mccompletepathv2<int>(graph, 50, 200, 1000, 0.85);
    auto end2 = std::chrono::steady_clock::now();
    std::cout << "mc run-time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin2).count() << " ms" << endl;

    auto bench2 = benchmarkAlgorithm(map2, graph, 200, true);
    cout << "-------" << endl;
    for(auto& keyVal: bench2)
      cout << keyVal.first << "     " << keyVal.second << endl;
    cout << "-------" << endl;
  }

  return 0;
}

unordered_map<int, vector<int>> importGraph(string fname)
{
  size_t edgeCounter = 0;
  ifstream inputFile(fname, ifstream::in);
  unordered_map<int, vector<int>> graph;

  //needed for repeating edges
  unordered_map<int, unordered_map<int, bool>> edges;

  string tmp = "";
  string delimiter = ",";
  while(getline(inputFile, tmp))
  {
    size_t pos = tmp.find(delimiter);
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '\r'), tmp.end());
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '\n'), tmp.end());
    int n1 = stoi(tmp.substr(0, pos));
    int n2 = stoi(tmp.substr(pos + 1));

    //make sure that the second node is added to the map
    //in case there are no edges starting from this node
    graph[n2];

    //checking to avoid repeated edges in the csv file
    if(!edges[n1][n2])
    {
      edges[n1][n2] = true;
      graph[n1].push_back(n2);
      edgeCounter++;
    }
  }
  cout << "nodes: " << graph.size() << " edges: " << edgeCounter << endl;

  return graph;
}

unordered_map<string, vector<string>> importGraph2(string fname)
{
  size_t edgeCounter = 0;
  ifstream inputFile(fname, ifstream::in);
  unordered_map<string, vector<string>> graph;

  //needed for repeating edges
  unordered_map<string, unordered_map<string, bool>> edges;

  string tmp = "";
  string delimiter = ",";
  while(getline(inputFile, tmp))
  {
    size_t pos = tmp.find(delimiter);
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '\r'), tmp.end());
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '\n'), tmp.end());
    string n1 = tmp.substr(0, pos);
    string n2 = tmp.substr(pos + 1);


    //make sure that the second node is added to the map
    //in case there are no edges starting from this node
    graph[n2];

    //checking to avoid repeated edges in the csv file
    if(!edges[n1][n2])
    {
      edges[n1][n2] = true;
      graph[n1].push_back(n2);
      edgeCounter++;
    }
  }
  cout << "nodes: " << graph.size() << " edges: " << edgeCounter << endl;

  return graph;
}
