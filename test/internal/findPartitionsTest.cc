#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <gtest.h>
#include <gtest-spi.h>
#include <pprInternal.h>

using namespace std;
using ppr::pprInternal::findPartitions;

TEST(findPartitions, emptyGraph)
{
  unordered_map<int, vector<int>> graph;
  auto ps = findPartitions(graph);
  ASSERT_EQ(ps.first.size(), 0);
  ASSERT_EQ(ps.second.size(), 0);
}

TEST(findPartitions, noEdges)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  //add nodes
  for(int i = 0; i < n; i++)
    graph[i];

  auto ps = findPartitions(graph);
  ASSERT_EQ(ps.first.size(), n);
  ASSERT_EQ(ps.second.size(), 0);
}

TEST(findPartitions, twoPartitions1Node)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  //add edges from 0 to other nodes
  for(int i = 0; i < n; i++)
  {
    graph[0].push_back(i);
    //add node to graph
    graph[i];
  }

  auto ps = findPartitions(graph);
  ASSERT_TRUE(
      (ps.first.size() == 1 && ps.second.size() == (n - 1)) ||
      (ps.first.size() == (n - 1) && ps.second.size() == 1));
}

TEST(findPartitions, twoPartitionsPairedNodes)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  for(int i = 0; i < n; i++)
  {
    graph[i].push_back(i + n);
    graph[i + n].push_back(i);
  }

  auto ps = findPartitions(graph);
  ASSERT_EQ(ps.first.size(), n);
  ASSERT_EQ(ps.second.size(), n);
}

TEST(findPartitions, twoPartitionsCompleteBetweenpartitions)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  for(int i = 0; i < n; i++)
  {
    for(int u = 0; u < n; u++)
    {
      graph[i].push_back(u + n);
      graph[u + n].push_back(i);
    }
  }

  auto ps = findPartitions(graph);
  ASSERT_EQ(ps.first.size(), n);
  ASSERT_EQ(ps.second.size(), n);
}

TEST(findPartitions, twoPartitionsCompletegraph)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  //complete graph
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);

  auto ps = findPartitions(graph);
  ASSERT_TRUE(
      (ps.first.size() == 1 && ps.second.size() == (n - 1)) ||
      (ps.first.size() == (n - 1) && ps.second.size() == 1));
}
