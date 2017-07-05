#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdlib.h>//exit
#include <random>

#include <gtest.h>
#include <gtest-spi.h>
#include <../header-only/mccompletepathv2.h>
#include <pprSingleSource.h>

using namespace std;
using ppr::mccompletepathv2;
using ppr::pprInternal::pprSingleSource;

extern random_device rd;
extern default_random_engine eng;
extern uniform_int_distribution<unsigned long long> dis;

TEST(mccompletepathv2HeaderOnly, badParameters)
{
  unordered_map<int, vector<int>> graph;
  ASSERT_EXIT(mccompletepathv2(graph, 0, 3, 42, 0.5), ::testing::ExitedWithCode(EXIT_FAILURE), "K must be positive");
  ASSERT_EXIT(mccompletepathv2(graph, 2, 0, 32, 0.85), ::testing::ExitedWithCode(EXIT_FAILURE), "L must be positive");
  ASSERT_EXIT(mccompletepathv2(graph, 2, 1, 10, 0.5), ::testing::ExitedWithCode(EXIT_FAILURE), "K must be <= L");
  ASSERT_EXIT(mccompletepathv2(graph, 2, 2, 0, 0.5), ::testing::ExitedWithCode(EXIT_FAILURE), "iterations must be positive");
  ASSERT_EXIT(mccompletepathv2(graph, 2, 2, 10, 1.5), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
  ASSERT_EXIT(mccompletepathv2(graph, 2, 2, 10, -1.5), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
}

TEST(mccompletepathv2HeaderOnly, emptyGraph)
{
  unordered_map<int, vector<int>> graph;
  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), 0);
}

TEST(mccompletepathv2HeaderOnly, testNoEdges)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 10; i++)
    graph[i];
  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ(res[i].size(), 1);
    ASSERT_NEAR(res[i][i], 1.0, 10e-5);
  }
}

TEST(mccompletepathv2HeaderOnly, testTopL)
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
    auto res =  mccompletepathv2(graph, i, i, 100, 0.85);
    ASSERT_EQ(res.size(), graph.size());
    for(int u = 0; u < 30; u++)
      ASSERT_LE(res[u].size(), i);
  }
}

TEST(mccompletepathv2HeaderOnly, singleNode)
{
  unordered_map<int, vector<int>> graph;
  graph[0];
  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);

  graph[0].push_back(0);
  res =  mccompletepathv2(graph, 10, 30, 1000, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_GE(res[0][0], 1.0);
}

TEST(mccompletepathv2HeaderOnly, twoNodes)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];
  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), 1);
  ASSERT_EQ(res[1].size(), 1);
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);
  ASSERT_NEAR(res[1][1], 1.0, 10e-5);

  graph[0].push_back(1);
  graph[1].push_back(0);
  res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_EQ(res[1].size(), graph.size());
  ASSERT_GE(res[0][0], res[0][1]);
  ASSERT_GE(res[1][1], res[1][0]);
}

TEST(mccompletepathv2HeaderOnly, lineGraph)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 0; i < 6; i++)
    graph[i].push_back((i+1)%6);

  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), graph.size());
      for(int u = 0; u < 5; u++)
        ASSERT_GE(res[i][(i + u)%6], res[i][(i + u + 1)%6]);
  }

  ASSERT_EQ(res.size(), graph.size());
  res =  mccompletepathv2(graph, 6, 6, 1000, 0.85);

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), 6);
      for(int u = 0; u < 2; u++)
      {
        ASSERT_GE(res[i][(i + u)%6],res[i][(i + u + 1)%6]);
      }
  }

  ASSERT_EQ(res.size(), graph.size());
  res =  mccompletepathv2(graph, 6, 6, 100, 0.85);

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), 6);
      for(int u = 0; u < 2; u++)
      {
        ASSERT_GE(res[i][(i + u)%6],res[i][(i + u + 1)%6]);
      }
  }
}

TEST(mccompletepathv2HeaderOnly, starGraph)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 1; i < 6; i++)
    graph[i].push_back(0);

  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), 1);
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);

  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 2);
    ASSERT_NEAR(res[i][0], 0.85, 10e-5);
  }

  //connect the center to itself
  graph[0].push_back(0);
  res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 2);
    ASSERT_GE(res[i][0], 1.0);
  }
}

TEST(mccompletepathv2HeaderOnly, starGraphReversed)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 1; i < 6; i++)
    graph[0].push_back(i);

  auto res =  mccompletepathv2(graph, 10, 30, 100, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);

  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 1);
    //0.85/5 because res[i][i] is equal 1.0 and will be multiplied by 0.85
    //when combined by 0, then divided by the number of successors of 0
    ASSERT_NEAR(res[0][1], 0.85/5, 10e-5);
    ASSERT_NEAR(res[i][0], 0, 10e-5);
  }

  for(int i = 1; i < 6; i++)
    graph[i].push_back(i);
  res =  mccompletepathv2(graph, 10, 30, 1000, 0.85);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);
  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 1);
    ASSERT_NEAR(res[i][0], 0, 10e-5);
    ASSERT_GE(res[0][i], 1.0);
  }
}

TEST(mccompletepathv2HeaderOnly, testNodesGreaterThanK)
{
  const size_t K = 10;
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto res =  mccompletepathv2(graph, K, K, 100, 0.85);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), K);
      for(int u = 0, uEnd = K - 1; u < uEnd; u++)
      {
        ASSERT_GE(res[i][(i + u + 1)%100], 0);
        ASSERT_GE(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }

  //same as before but with L > K
  res =  mccompletepathv2(graph, K, K * 2, 100, 0.85);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), K);
      for(int u = 0, uEnd = K - 1; u < uEnd; u++)
      {
        ASSERT_GE(res[i][(i + u + 1)%100], 0);
        ASSERT_GE(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }

  //same as before but with L  = graph.size()
  res =  mccompletepathv2(graph, K, graph.size(), 100, 0.85);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      ASSERT_EQ(res[i].size(), K);
      for(int u = 0, uEnd = K - 1; u < uEnd; u++)
      {
        ASSERT_GE(res[i][(i + u + 1)%100], 0);
        ASSERT_GE(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }

}

TEST(mccompletepathv2HeaderOnly, testNodesEqualK)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  //-1 tolerance means no tolerance/ignored
  auto res =  mccompletepathv2(graph, graph.size(), graph.size(), 1000, 0.85);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      for(int u = 0, uEnd = graph.size() - 1; u < uEnd; u++)
      {
        ASSERT_GE(res[i][(i + u + 1)%100], 0);
        ASSERT_GE(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }
}

TEST(mccompletepathv2HeaderOnly, testNodesLowerThanK)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto res =  mccompletepathv2(graph, graph.size() * 2, graph.size() * 2, 1000, 0.85);

  //for each node check that the PPR of a node after that is always lower,
  for(int i = 0; i < 100; i++)
  {
      for(int u = 0, uEnd = graph.size() - 1; u < uEnd; u++)
      {
        ASSERT_GE(res[i][(i + u + 1)%100], 0);
        //GE and not GT because the smallest values are so low that its hard to compare them
        ASSERT_GE(res[i][(i + u)%100], res[i][(i + u + 1)%100]);
      }
  }
}
