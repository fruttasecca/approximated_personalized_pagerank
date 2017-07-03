#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdlib.h>//exit

#include <gtest.h>
#include <gtest-spi.h>
#include <benchmarkAlgorithm.h>
#include <pprSingleSource.h>
#include <grank.h>

using namespace std;
using ppr::pprInternal::pprSingleSource;
using ppr::grank;
using ppr::benchmarkAlgorithm;

extern random_device rd;
extern default_random_engine eng;
extern uniform_int_distribution<unsigned long long> dis;

TEST(benchmarkAlgorithm, badParameters)
{
  unordered_map<int, vector<int>> graph;
  auto gr = grank(graph, 1, 3, 42, 0.5, 0.0001);
  ASSERT_EXIT(benchmarkAlgorithm(gr, graph, 0, false), ::testing::ExitedWithCode(EXIT_FAILURE), "testNodes must be positive");

  //gr containing nodes that aren't in graph
  gr[5];
  ASSERT_EXIT(benchmarkAlgorithm(gr, graph, 10, false), ::testing::ExitedWithCode(EXIT_FAILURE),
    "node 5 in the provided map is not part of the provided graph");
}

TEST(benchmarkAlgorithm, emptyInputMap)
{
  unordered_map<int, vector<int>> graph;
  unordered_map<int, unordered_map<int, double>> gr;
  auto res = benchmarkAlgorithm(gr, graph, 400, false);

  for(auto& keyVal: res)
    ASSERT_EQ(keyVal.second, -1);
}

TEST(benchmarkAlgorithm, strictLeadsTo0TestedNodes)
{
  unordered_map<int, vector<int>> graph;
  graph[0];
  graph[1];
  auto gr = grank(graph, 1, 3, 42, 0.5, 0.0001);
  auto res = benchmarkAlgorithm(gr, graph, 50, true);

  for(auto& keyVal: res)
    ASSERT_EQ(keyVal.second, -1);
}

TEST(benchmarkAlgorithm, testNoEdges)
{
  unordered_map<int, vector<int>> graph;
  for(int i = 0; i < 100; i++)
    graph[i];
  auto gr = grank(graph, 1, 3, 42, 0.5, 0.0001);
  auto res = benchmarkAlgorithm(gr, graph, 50, false);
  for(auto& keyVal: res)
    ASSERT_EQ(keyVal.second, 1.0);
}

TEST(benchmarkAlgorithm, compareSameAlgorithm)
{
  unordered_map<int, vector<int>> graph;
  unordered_map<int, unordered_map<int, double>> pr;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);
  for(int i = 0; i < n ; i++)
    pr[i] = pprSingleSource(graph, 100, 0.85, 0.0001,  i);
  auto res = benchmarkAlgorithm(pr, graph, 50, false);
  ASSERT_NEAR(res["jaccard average"], 1.0, 10e-5);
  ASSERT_NEAR(res["jaccard min"], 1.0, 10e-5);
  ASSERT_NEAR(res["kendall average"], 1.0, 10e-5);
  ASSERT_NEAR(res["kendall min"], 1.0, 10e-5);
  ASSERT_NEAR(res["average map size"], 100, 10e-5);
}

TEST(benchmarkAlgorithm, compareSameAlgorithmRandomGraph)
{
  unordered_map<int, vector<int>> graph;
  unordered_map<int, unordered_map<int, double>> pr;
  int n = 100;
  int edges = 4000;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < edges; u++)
      graph[dis(eng)%n].push_back(dis(eng)%n);
  for(int i = 0; i < n ; i++)
    pr[i] = pprSingleSource(graph, 100, 0.85, 0.0001,  i);
  auto res = benchmarkAlgorithm(pr, graph, 50, false);
  ASSERT_NEAR(res["jaccard average"], 1.0, 10e-5);
  ASSERT_NEAR(res["jaccard min"], 1.0, 10e-5);
  ASSERT_NEAR(res["kendall average"], 1.0, 10e-5);
  ASSERT_NEAR(res["kendall min"], 1.0, 10e-5);
}

TEST(benchmarkAlgorithm, compareNodesWithTotallyDifferentTop)
{
  unordered_map<int, vector<int>> graph;
  unordered_map<int, unordered_map<int, double>> pr;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);
  for(int i = 0; i < n ; i++)
    pr[i][i+1] = -1.0;
  auto res = benchmarkAlgorithm(pr, graph, 50, false);
  ASSERT_NEAR(res["jaccard average"], 0, 10e-5);
  ASSERT_NEAR(res["jaccard min"], 0, 10e-5);
}

TEST(benchmarkAlgorithm, compareSameAlgorithmReveresedOrder)
{
  unordered_map<int, vector<int>> graph;
  unordered_map<int, unordered_map<int, double>> pr;
  int n = 100;
  int edges = 4000;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < edges; u++)
      graph[dis(eng)%n].push_back(dis(eng)%n);
  for(int i = 0; i < n ; i++)
  {
    pr[i] = pprSingleSource(graph, 100, 0.85, 0.0001,  i);
    for(auto& keyVal: pr[i])
      keyVal.second *= -1;
  }
  auto res = benchmarkAlgorithm(pr, graph, 50, false);
  ASSERT_NEAR(res["jaccard average"], 1.0, 10e-5);
  ASSERT_NEAR(res["jaccard min"], 1.0, 10e-5);
  ASSERT_NEAR(res["kendall average"], -1.0, 10e-5);
  ASSERT_NEAR(res["kendall min"], -1.0, 10e-5);
}

TEST(benchmarkAlgorithm, compareHalfJaccard)
{
  unordered_map<int, vector<int>> graph;
  unordered_map<int, unordered_map<int, double>> pr;
  int n = 100;
  for(int i = 0; i < n; i++)
    for(int u = 0; u < n; u++)
      graph[i].push_back(u);
  for(int i = 0; i < n ; i++)
  {
    pr[i] = pprSingleSource(graph, 100, 0.85, 0.0001,  i);
    double minus = -1;
    for(int u = 1; u <= n; u++)
    {
      graph[-u];
      pr[i][-u] = 1.0;
    }
  }
  auto res = benchmarkAlgorithm(pr, graph, 50, false);
  ASSERT_NEAR(res["jaccard average"], 0.5, 10e-5);
  ASSERT_NEAR(res["jaccard min"], 0.5, 10e-5);
}
