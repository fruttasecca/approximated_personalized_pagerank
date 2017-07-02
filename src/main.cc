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
#include <pprSingleSource.h>

using namespace std;
using ppr::grank;
using ppr::pprInternal::pprSingleSource;

template<class Key>
void printMap(unordered_map<Key, unordered_map<Key, double>> map)
{
  for(auto vk1: map)
  {
    cout << vk1.first << " ppr: " << endl;
    for(auto vk2: map[vk1.first])
    {
      cout << "\t" << vk2.first  << ":\t" << vk2.second << endl;
    }
  }
}

//checkare per lati ripetuti
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


int main()
{
  unordered_map<int, vector<int>> graph = importGraph("gnutella30.csv");
  unordered_map<int, double> testing;
  auto begin = std::chrono::steady_clock::now();
  auto map = grank<int>(graph, 50, 100, 30, 0.85, 0.0001);
  int done = 0;
  for(const auto& keyVal: graph)
  {
    pprSingleSource(graph, 30, 0.85, 0.0001, keyVal.first);
    done++;
  }
  cout << done << endl;
  auto end= std::chrono::steady_clock::now();
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() <<std::endl;


  return 0;
}
