#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdlib.h>//exit
#include <random>

#include <gtest.h>
#include <gtest-spi.h>
#include <grank.h>
#include <pprSingleSource.h>

using namespace std;
using ppr::grank;
using ppr::pprInternal::pprSingleSource;

std::random_device rd;
std::default_random_engine eng(rd());
std::uniform_int_distribution<unsigned long long> dis;

TEST(grank, badParameters)
{
  unordered_map<int, vector<int>> graph;
  ASSERT_EXIT(grank(graph, 0, 3, 42, 0.5, 0.0001), ::testing::ExitedWithCode(EXIT_FAILURE), "K must be positive");
  ASSERT_EXIT(grank(graph, 2, 0, 32, 0.85, 0.0001), ::testing::ExitedWithCode(EXIT_FAILURE), "L must be positive");
  ASSERT_EXIT(grank(graph, 2, 1, 10, 0.5, 0.0001), ::testing::ExitedWithCode(EXIT_FAILURE), "K must be <= L");
  ASSERT_EXIT(grank(graph, 2, 2, 0, 0.5, 0.0001), ::testing::ExitedWithCode(EXIT_FAILURE), "iterations must be positive");
  ASSERT_EXIT(grank(graph, 2, 2, 10, 1.5, 0.0001), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
  ASSERT_EXIT(grank(graph, 2, 2, 10, -1.5, 0.0001), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
}

TEST(grank, emptyGraph)
{
  unordered_map<int, vector<int>> graph;
  auto res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), 0);
}

TEST(grank, testNoEdges)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 10; i++)
    graph[i];
  auto res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ(res[i].size(), 1);
    ASSERT_NEAR(res[i][i], 0.15, 10e-5);
  }
}

TEST(grank, testTopL)
{
  unordered_map<int, vector<int>> graph;
  //adds 30 nodes and <= 30 random edges, then checks if the returned top
  //for each node is always <= topL
  for(int i = 0; i < 30; i++)
    graph[i];
  for(int i = 0; i < 30; i++)
    graph[dis(eng)%30].push_back(dis(eng)%30);
  for(int i = 1; i < 30; i++)
  {
    auto res =  grank(graph, i, i, 100, 0.85, 0.0001);
    ASSERT_EQ(res.size(), graph.size());
    for(int u = 0; u < 30; u++)
      ASSERT_LE(res[u].size(), i);
  }
}

TEST(grank, singleNode)
{
  unordered_map<int, vector<int>> graph;
  graph[0];
  auto res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 0.15, 10e-5);

  graph[0].push_back(0);
  res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);
}

TEST(grank, twoNodes)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];
  auto res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), 1);
  ASSERT_EQ(res[1].size(), 1);
  ASSERT_NEAR(res[0][0], 0.15, 10e-5);
  ASSERT_NEAR(res[1][1], 0.15, 10e-5);

  graph[0].push_back(1);
  graph[1].push_back(0);
  res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_EQ(res[1].size(), graph.size());
  ASSERT_GT(res[0][0], res[0][1]);
  ASSERT_GT(res[1][1], res[1][0]);
}

TEST(grank, lineGraph)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 0; i < 6; i++)
    graph[i].push_back((i+1)%6);

  auto res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), graph.size());
      for(int u = 0; u < 5; u++)
        ASSERT_GT(res[i][(i + u)%6], res[i][(i + u + 1)%6]);
  }

  ASSERT_EQ(res.size(), graph.size());
  res =  grank(graph, 3, 4, 100, 0.85, 0.0001);

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), 3);
      for(int u = 0; u < 2; u++)
      {
        ASSERT_GT(res[i][(i + u)%6],res[i][(i + u + 1)%6]);
      }
  }

  ASSERT_EQ(res.size(), graph.size());
  res =  grank(graph, 3, 3, 100, 0.85, 0.0001);

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), 3);
      for(int u = 0; u < 2; u++)
      {
        ASSERT_GT(res[i][(i + u)%6],res[i][(i + u + 1)%6]);
      }
  }
}

TEST(grank, starGraph)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 1; i < 6; i++)
    graph[i].push_back(0);

  auto res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), 1);
  ASSERT_NEAR(res[0][0], 0.15, 10e-5);

  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 2);
    ASSERT_NEAR(res[i][0], 0.15 * 0.85, 10e-5);
  }

  //connect the center to itself
  graph[0].push_back(0);
  res =  grank(graph, 10, 30, 100, 0.85, 0.0001);
  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 2);
    ASSERT_NEAR(res[i][0], 0.85, 10e-5);
  }
}

TEST(grank, testNodesGreaterThanK)
{
  const size_t K = 10;
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto res =  grank(graph, K, K, 100, 0.85, 0.0001);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), K);
      for(int u = 0, uEnd = K - 1; u < uEnd; u++)
      {
        ASSERT_GT(res[i][(i + u + 1)%100], 0);
        ASSERT_GT(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }

  //same as before but with L > K
  res =  grank(graph, K, K * 2, 100, 0.85, 0.0001);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), K);
      for(int u = 0, uEnd = K - 1; u < uEnd; u++)
      {
        ASSERT_GT(res[i][(i + u + 1)%100], 0);
        ASSERT_GT(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }

  //same as before but with L  = graph.size()
  res =  grank(graph, K, graph.size(), 100, 0.85, 0.0001);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), K);
      for(int u = 0, uEnd = K - 1; u < uEnd; u++)
      {
        ASSERT_GT(res[i][(i + u + 1)%100], 0);
        ASSERT_GT(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }

}

TEST(grank, testNodesEqualK)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  //-1 tolerance means no tolerance/ignored
  auto res =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), graph.size());
      for(int u = 0, uEnd = graph.size() - 1; u < uEnd; u++)
      {
        ASSERT_GT(res[i][(i + u + 1)%100], 0);
        ASSERT_GT(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }
}

TEST(grank, testNodesLowerThanK)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  //-1 tolerance means no tolerance/ignored
  auto res =  grank(graph, graph.size() * 2, graph.size() * 2, 100, 0.85, -1);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), graph.size());
      for(int u = 0, uEnd = graph.size() - 1; u < uEnd; u++)
      {
        ASSERT_GT(res[i][(i + u + 1)%100], 0);
        ASSERT_GT(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }
}

TEST(grank, sameAsPagerank1)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grank, sameAsPagerank2)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(0);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }

  graph[0].push_back(0);
  gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }

  for(int i = 0; i < 99; i++)
    graph[0].push_back(i);
  gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grank, sameAsPagerank3)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  int edges = 5000;
  for(int i = 0; i < n; i++)
    graph[i];
  for(int i = 0; i < edges; i++)
    graph[dis(eng)%n].push_back(dis(eng)%n);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  for(int i = 0; i < n; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < n; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grank, sameAsPagerank4)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  for(int i = 0; i < n; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < n; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}
