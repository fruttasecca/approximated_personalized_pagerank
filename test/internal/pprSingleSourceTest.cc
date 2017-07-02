#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdlib.h>//exit

#include <gtest.h>
#include <gtest-spi.h>
#include <pprSingleSource.h>

using namespace std;
using ppr::pprInternal::pprSingleSource;

TEST(pprSingleSource, badParameters)
{
  unordered_map<int, vector<int>> graph;
  ASSERT_EXIT(pprSingleSource(graph, 0, 0.85, 0.001,  0), ::testing::ExitedWithCode(EXIT_FAILURE), "iterations must be positive");
  ASSERT_EXIT(pprSingleSource(graph, 1, 1.85, 0.001,  0), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");//last argument is a posix regex
  ASSERT_EXIT(pprSingleSource(graph, 1, -1.85, 0.001,  0), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
  ASSERT_EXIT(pprSingleSource(graph, 1, 0.85, 0.001,  0), ::testing::ExitedWithCode(EXIT_FAILURE), "source node not part of the graph");
}

TEST(pprSingleSource, testSingleNode)
{
  unordered_map<int, vector<int>> graph;
  graph[0];

  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  0);
  ASSERT_NEAR(res[0], 0.15, 0.00001);
}

TEST(pprSingleSource, testNoEdges)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 5; i++)
    graph[i];

  for(int i = 0; i < 5; i++)
  {
    auto res = pprSingleSource(graph, 100, 0.85, 0.001,  i);
    ASSERT_NEAR(res[i], 0.15, 0.00001);
  }
}

TEST(pprSingleSource, testEdgesIsolated)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 1; i < 5; i++)
    for(int u = 1; u < 5; u++)
      graph[i].push_back(u);
  graph[0];

  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  0);
  ASSERT_NEAR(res[0], 0.15, 0.00001);
}

TEST(pprSingleSource, testOriginHighestScore1)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];graph[2];

  graph[0].push_back(1);
  graph[0].push_back(2);
  graph[1].push_back(0);
  graph[1].push_back(2);
  graph[2].push_back(0);
  graph[2].push_back(1);


  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  0);
  ASSERT_EQ(res.size(), 3);
  ASSERT_TRUE(res[0] > res[1] && res[0] > res[2]);
}

TEST(pprSingleSource, testOriginHighestScore2)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];graph[2];

  graph[0].push_back(1);
  graph[0].push_back(2);
  graph[1].push_back(2);
  graph[2].push_back(1);


  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  1);
  ASSERT_EQ(res.size(), 2);
  ASSERT_TRUE(res[1] > res[2]);
}

TEST(pprSingleSource, testOriginHighestScore3)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];graph[2];graph[3];graph[4];graph[5];


  graph[0].push_back(1);
  graph[1].push_back(2);
  graph[2].push_back(3);
  graph[3].push_back(4);
  graph[4].push_back(5);
  graph[5].push_back(0);

  for(int i = 0; i < 6; i ++)
  {
    auto res = pprSingleSource(graph, 100, 0.85, 0.001,  i);
    ASSERT_EQ(res.size(), graph.size());
    for(int u = 0; u < 6; u++)
      ASSERT_TRUE(res[i] > res[u] || i == u);
  }
}

TEST(pprSingleSource, testCorrectlyOrderedScore1)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];graph[2];graph[3];

  graph[0].push_back(1);
  graph[0].push_back(2);
  graph[1].push_back(3);
  graph[2].push_back(3);
  graph[3].push_back(0);

  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  0);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_TRUE(res[0] > res[1] && res[0] > res[2] && res[0] > res[3]);
  ASSERT_TRUE(res[3] > res[2] && res[3] > res[1]);
  ASSERT_NEAR(res[1], res[2], 10e-5);
}

TEST(pprSingleSource, testCorrectlyOrderedScore2)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 8; i++)
    graph[i];
  for(int i = 1; i < 7; i++)
    graph[0].push_back(i);
  for(int i = 1; i < 7; i++)
    graph[i].push_back(7);
  graph[7].push_back(0);

  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  0);
  ASSERT_TRUE(res[0] > res[7]);
  ASSERT_EQ(res.size(), graph.size());
  for(int i = 1; i < 7; i++)
  {
    ASSERT_TRUE(res[0] > res[i]);
    ASSERT_TRUE(res[7] > res[i]);
  }

  res = pprSingleSource(graph, 100, 0.85, 0.001,  7);
  ASSERT_TRUE(res[7] > res[0]);
  ASSERT_EQ(res.size(), graph.size());
  for(int i = 1; i < 7; i++)
  {
    ASSERT_TRUE(res[0] > res[i]);
    ASSERT_TRUE(res[7] > res[i]);
  }
}

TEST(pprSingleSource, testCorrectlyOrderedScore3)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];
  graph[0].push_back(1);
  graph[0].push_back(2);
  graph[0].push_back(4);

  graph[1].push_back(3);
  graph[2].push_back(3);
  graph[3].push_back(5);
  graph[4].push_back(5);
  graph[5].push_back(0);

  auto res = pprSingleSource(graph, 100, 0.85, 0.001,  0);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_TRUE(res[0] > res[5]);
  for(int i = 1; i < 5; i++)
  {
    ASSERT_TRUE(res[0] > res[i]);
    ASSERT_TRUE(res[5] > res[i]);
    ASSERT_TRUE(res[3] > res[i] || i == 3);
  }

  res = pprSingleSource(graph, 100, 0.85, 0.001,  4);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_TRUE(res[5] > res[0]);
  for(int i = 1; i < 5; i++)
  {
    ASSERT_TRUE(res[0] > res[i]);
    ASSERT_TRUE(res[5] > res[i]);
  }
}
