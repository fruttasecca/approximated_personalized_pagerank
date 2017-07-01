#include <unordered_set>
#include <random>

#include <gtest.h>
#include <gtest-spi.h>
#include <pprInternal.h>

using namespace std;
using ppr::pprInternal::keepTop;
using std::unordered_map;

TEST(keepTop, testTop0)
{
  unordered_map<int, double> map;
  map[1] = 10;
  map[2] = 4;
  keepTop(0, map);
  ASSERT_EQ(map.size(), 0);
}

TEST(keepTop, testTop1)
{
  unordered_map<int, double> map;
  map[1] = 10;
  map[2] = 4;
  keepTop(1, map);
  ASSERT_EQ(map.size(), 1);
  ASSERT_NE(map.find(1), map.end());
}

TEST(keepTop, testTop2)
{
  unordered_map<int, double> map;
  map[1] = 10;
  map[2] = 4;
  keepTop(2, map);
  ASSERT_EQ(map.size(), 2);
  ASSERT_NE(map.find(1), map.end());
  ASSERT_NE(map.find(2), map.end());
}

TEST(keepTop, testTopN)
{
  unordered_map<int, double> map;

  //make vector of nodes and shuffle it to randomize order of insertion
  int n = 500;
  std::random_device rd;
  std::mt19937 g(rd());
  vector<int> nodes;
  for(int i = 0; i <= n; i++)
    nodes.push_back(i);
  shuffle(nodes.begin(), nodes.end(), g);

  //add to map
  for(int node: nodes)
    map[node] = node;

  for(int i = 0; i <= n + 1; i++)
  {
    unordered_map<int, double> tmp(map);
    keepTop(i, tmp);
    ASSERT_EQ(tmp.size(), i);

    //check that every element that should be in the top is actually there
    for(int u = 0; u < i; u++)
      ASSERT_NE(tmp.find(n - u), tmp.end());
  }
}
