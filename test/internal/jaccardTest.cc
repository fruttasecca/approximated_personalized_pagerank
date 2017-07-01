#include <iostream>
#include <unordered_set>

#include <gtest.h>
#include <gtest-spi.h>
#include <pprInternal.h>

using namespace std;
using ppr::pprInternal::jaccard;
using std::unordered_set;

TEST(jaccard, testEmpty)
{
  unordered_set<int> s1, s2;
  ASSERT_EQ(jaccard(s1,s2), 1.0);
}

TEST(jaccard, testOneElement1)
{
  unordered_set<int> s1, s2;
  s1.insert(2);
  ASSERT_EQ(jaccard(s1,s2), 0.0);
}

TEST(jaccard, testOneElement1_1)
{
  unordered_set<int> s1, s2;
  s2.insert(2);
  ASSERT_EQ(jaccard(s1,s2), 0.0);
}

TEST(jaccard, testOneElement2)
{
  unordered_set<int> s1, s2;
  s1.insert(2);
  s2.insert(-2);
  ASSERT_EQ(jaccard(s1,s2), 0.0);
}

TEST(jaccard, testOneElement3)
{
  unordered_set<int> s1, s2;
  s1.insert(-2);
  s2.insert(-2);
  ASSERT_EQ(jaccard(s1,s2), 1.0);
}

TEST(jaccard, testIdenticallyPopulatedSets)
{
  unordered_set<int> s1, s2;
  for(int i = 0; i < 100; i++)
  {
    s1.insert(i);
    s2.insert(i);
  }
  ASSERT_EQ(jaccard(s1,s2), 1.0);
}

TEST(jaccard, testIdenticalSets)
{
  unordered_set<int> s1, s2;
  for(int i = 0; i < 100; i++)
  {
    s1.insert(i);
    s2.insert(i);
  }
  ASSERT_EQ(jaccard(s1,s2), 1.0);
}

TEST(jaccard, testTotallyDifferentSets)
{
  unordered_set<int> s1, s2;
  for(int i = 1; i < 100; i++)
  {
    s1.insert(i);
    s2.insert(-i);
  }
  ASSERT_EQ(jaccard(s1,s2), 0.0);
}

TEST(jaccard, testHalfJaccard)
{
  unordered_set<int> s1, s2;
  for(int i = 0; i < 100; i++)
  {
    s1.insert(i);
    s2.insert(i + 100);
  }
  s2.insert(s1.begin(), s2.end());
  ASSERT_EQ(jaccard(s1,s2), 0.5);
}

TEST(jaccard, testPercentJaccard)
{
  unordered_set<int> s1, s2;
  //init a set with ints from 0 to 99
  for(int i = 0; i < 100; i++)
    s1.insert(i);
  //grow s2 10% at a time
  for(int round = 1; round <= 10; round++)
  {
      for(int i = (round - 1) * 10 ; i < round * 10; i++)
          s2.insert(i);
      ASSERT_EQ(jaccard(s1,s2), round/10.0);
  }
}
