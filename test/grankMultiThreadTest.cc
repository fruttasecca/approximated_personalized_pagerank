#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdlib.h>//exit
#include <random>

#include <gtest.h>
#include <gtest-spi.h>
#include <grank.h>
#include <../header-only/grankMulti.h>
#include <pprSingleSource.h>

using namespace std;
using ppr::grankMulti;
using ppr::grank;
using ppr::pprInternal::pprSingleSource;

extern std::random_device rd;
extern std::default_random_engine eng;
extern std::uniform_int_distribution<unsigned long long> dis;

TEST(grankMultiThread, badParameters)
{
  unordered_map<int, vector<int>> graph;
  ASSERT_EXIT(grankMulti(graph, 0, 3, 42, 0.5, 0.0001, 4), ::testing::ExitedWithCode(EXIT_FAILURE), "K must be positive");
  ASSERT_EXIT(grankMulti(graph, 2, 0, 32, 0.85, 0.0001, 4), ::testing::ExitedWithCode(EXIT_FAILURE), "L must be positive");
  ASSERT_EXIT(grankMulti(graph, 2, 1, 10, 0.5, 0.0001, 4), ::testing::ExitedWithCode(EXIT_FAILURE), "K must be <= L");
  ASSERT_EXIT(grankMulti(graph, 2, 2, 0, 0.5, 0.0001, 4), ::testing::ExitedWithCode(EXIT_FAILURE), "iterations must be positive");
  ASSERT_EXIT(grankMulti(graph, 2, 2, 10, 1.5, 0.0001, 4), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
  ASSERT_EXIT(grankMulti(graph, 2, 2, 10, -1.5, 0.0001, 4), ::testing::ExitedWithCode(EXIT_FAILURE), "damping must be \\[0,1]");
  ASSERT_EXIT(grankMulti(graph, 2, 2, 10, 0.6, 0.0001, 0), ::testing::ExitedWithCode(EXIT_FAILURE), "nThreads must be positive");
}

TEST(grankMultiThread, emptyGraph)
{
  unordered_map<int, vector<int>> graph;
  auto res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), 0);
}

TEST(grankMultiThread, testNoEdges)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 10; i++)
    graph[i];
  auto res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), graph.size());
  for(int i = 0; i < 10; i++)
  {
    ASSERT_EQ(res[i].size(), 1);
    ASSERT_NEAR(res[i][i], 0.15, 10e-5);
  }
}

TEST(grankMultiThread, testTopL)
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
    auto res =  grankMulti(graph, i, i, 100, 0.85, 0.0001, 4);
    ASSERT_EQ(res.size(), graph.size());
    for(int u = 0; u < 30; u++)
      ASSERT_LE(res[u].size(), i);
  }
}

TEST(grankMultiThread, singleNode)
{
  unordered_map<int, vector<int>> graph;
  graph[0];
  auto res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 0.15, 10e-5);

  graph[0].push_back(0);
  res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_NEAR(res[0][0], 1.0, 10e-5);
}

TEST(grankMultiThread, twoNodes)
{
  unordered_map<int, vector<int>> graph;
  graph[0];graph[1];
  auto res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), 1);
  ASSERT_EQ(res[1].size(), 1);
  ASSERT_NEAR(res[0][0], 0.15, 10e-5);
  ASSERT_NEAR(res[1][1], 0.15, 10e-5);

  graph[0].push_back(1);
  graph[1].push_back(0);
  res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), graph.size());
  ASSERT_EQ(res[0].size(), graph.size());
  ASSERT_EQ(res[1].size(), graph.size());
  ASSERT_GT(res[0][0], res[0][1]);
  ASSERT_GT(res[1][1], res[1][0]);
}

TEST(grankMultiThread, lineGraph)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 0; i < 6; i++)
    graph[i].push_back((i+1)%6);

  auto res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  ASSERT_EQ(res.size(), graph.size());

  //for each node check that the PPR of a node after is always lower
  for(int i = 0; i < 6; i++)
  {
      ASSERT_EQ(res[i].size(), graph.size());
      for(int u = 0; u < 5; u++)
        ASSERT_GT(res[i][(i + u)%6], res[i][(i + u + 1)%6]);
  }

  ASSERT_EQ(res.size(), graph.size());
  res =  grankMulti(graph, 3, 4, 100, 0.85, 0.0001, 4);

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
  res =  grankMulti(graph, 3, 3, 100, 0.85, 0.0001, 4);

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

TEST(grankMultiThread, starGraph)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 6; i++)
    graph[i];

  for(int i = 1; i < 6; i++)
    graph[i].push_back(0);

  auto res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
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
  res =  grankMulti(graph, 10, 30, 100, 0.85, 0.0001, 4);
  for(int i = 1; i < 6; i++)
  {
    ASSERT_EQ(res[i].size(), 2);
    ASSERT_NEAR(res[i][0], 0.85, 10e-5);
  }
}

TEST(grankMultiThread, testNodesGreaterThanK)
{
  const size_t K = 10;
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto res =  grankMulti(graph, K, K, 100, 0.85, 0.0001, 4);

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
  res =  grankMulti(graph, K, K * 2, 100, 0.85, 0.0001, 4);

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
  res =  grankMulti(graph, K, graph.size(), 100, 0.85, 0.0001, 4);

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

TEST(grankMultiThread, testNodesEqualK)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  //-1 tolerance means no tolerance/ignored
  auto res =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);

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

TEST(grankMultiThread, testNodesLowerThanK)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  //-1 tolerance means no tolerance/ignored
  auto res =  grankMulti(graph, graph.size() * 2, graph.size() * 2, 100, 0.85, -1, 4);

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

TEST(grankMultiThread, sameAsPagerank1)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto gr =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsPagerank2)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(0);

  auto gr =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }

  graph[0].push_back(0);
  gr =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }

  for(int i = 0; i < 99; i++)
    graph[0].push_back(i);
  gr =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < 100; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsPagerank3)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  int edges = 5000;
  for(int i = 0; i < n; i++)
    graph[i];
  for(int i = 0; i < edges; i++)
    graph[dis(eng)%n].push_back(dis(eng)%n);

  auto gr =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < n; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < n; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsPagerank4)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);

  auto gr =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < n; i++)
  {
    auto ppr = pprSingleSource(graph, 100, 0.85, -1,  i);
    ASSERT_EQ(gr[i].size(), ppr.size());
    for(int u = 0; u < n; u++)
      ASSERT_NEAR(gr[i][u], ppr[u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsGrank1)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsGrank2)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(0);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.01);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.01, 4);

  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }

  graph[0].push_back(0);
  gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.0005);
  grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.0005, 4);
  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }

  for(int i = 0; i < 99; i++)
    graph[0].push_back(i);
  gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.00001);
  grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.00001, 4);
  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsGrank3)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  int edges = 5000;
  for(int i = 0; i < n; i++)
    graph[i];
  for(int i = 0; i < edges; i++)
    graph[dis(eng)%n].push_back(dis(eng)%n);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 4);
  for(int i = 0; i < n; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, sameAsGrank4)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.001);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.001, 4);
  for(int i = 0; i < n; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, singleThreadSameAsGrank1)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(i + 1);
  graph[99].push_back(0);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 1);
  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, singleThreadSameAsGrank2)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  for(int i = 0; i < 99; i++)
    graph[i].push_back(0);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.01);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.01, 1);

  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }

  graph[0].push_back(0);
  gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.0005);
  grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.0005, 1);
  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }

  for(int i = 0; i < 99; i++)
    graph[0].push_back(i);
  gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.00001);
  grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.00001, 1);
  for(int i = 0; i < 100; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, singleThreadSameAsGrank3)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  int edges = 5000;
  for(int i = 0; i < n; i++)
    graph[i];
  for(int i = 0; i < edges; i++)
    graph[dis(eng)%n].push_back(dis(eng)%n);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, -1);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, -1, 1);
  for(int i = 0; i < n; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}

TEST(grankMultiThread, singleThreadSameAsGrank4)
{
  unordered_map<int, vector<int>> graph;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);

  auto gr =  grank(graph, graph.size(), graph.size(), 100, 0.85, 0.001);
  auto grM =  grankMulti(graph, graph.size(), graph.size(), 100, 0.85, 0.001, 1);
  for(int i = 0; i < n; i++)
  {
    ASSERT_EQ(gr[i].size(), grM[i].size());
    for(int u = 0; u < 100; u++)
      ASSERT_NEAR(gr[i][u], grM[i][u], 10e-5);
  }
}
