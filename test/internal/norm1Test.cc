#include <unordered_set>

#include <gtest.h>
#include <gtest-spi.h>
#include <pprInternal.h>

using namespace std;
using ppr::pprInternal::norm1;
using std::unordered_map;

TEST(norm1, testBothEmpty)
{
  unordered_map<int, double> m1,m2;
  ASSERT_EQ(norm1(m1,m2), 0);
}

TEST(norm1, test1Empty)
{
  unordered_map<int, double> m1,m2;
  int n = 300;
  for(int i = 0; i < 300; i++)
    m1[i] = i;
  ASSERT_EQ(norm1(m1,m2), (300 * 299)/2);
  ASSERT_EQ(norm1(m2,m1), (300 * 299)/2);
}

TEST(norm1, testNormWithSelf)
{
  unordered_map<int, double> m1;
  int n = 300;
  for(int i = 0; i < n; i++)
    m1[i] = i;
  ASSERT_EQ(norm1(m1,m1), 0);
}

TEST(norm1, testNormSameKeysDifferentValues)
{
  unordered_map<int, double> m1,m2;
  int n = 300;
  int totalDiff = 0;
  for(int i = 0; i < n; i++)
  {
    m1[i] = i;
    m2[i] = i * i;
    totalDiff += (i * i) - i;
  }

  ASSERT_EQ(norm1(m1,m2), totalDiff);
  ASSERT_EQ(norm1(m2,m1), totalDiff);
}

TEST(norm1, testNormDifferentKeys)
{
  unordered_map<int, double> m1,m2;
  int n = 300;
  int totalDiff = 0;
  for(int i = 0; i < n; i++)
  {
    m1[i] = i;
    m2[i + n] = i * i;
    totalDiff += (i * i) + i;
  }

  ASSERT_EQ(norm1(m1,m2), totalDiff);
  ASSERT_EQ(norm1(m2,m1), totalDiff);
}

TEST(norm1, testNormDifferentKeys2)
{
  unordered_map<int, double> m1,m2;
  int n = 300;
  int totalDiff = 0;
  for(int i = 0; i < n; i++)
  {
    m1[i] = i;
    m1[-i] = i;
    m2[i + n] = i * i;
    m2[-(i + n)] = i * i;
    totalDiff += ((i * i) + i) * 2;
  }

  ASSERT_EQ(norm1(m1,m2), totalDiff);
  ASSERT_EQ(norm1(m2,m1), totalDiff);
}

TEST(norm1, testNormDifferentAndSharedKeys)
{
  unordered_map<int, double> m1,m2;
  int n = 300;
  int totalDiff = 0;
  //add different keys
  for(int i = 0; i < n; i++)
  {
    m1[i] = i;
    m1[-i] = i;
    m2[i + n] = i * i;
    m2[-(i + n)] = i * i;
    totalDiff += ((i * i) + i) * 2;
  }

  //add shared keys but with same value (totalDiff is not updated)
  for(int i = 0, q = n * 2; i < n; i++)
  {
    m1[q + i] = i;
    m2[q + i] = i;
  }

  //add shared keys but with different values
  for(int i = 0, q = n * 2; i < n; i++)
  {
    m1[q + i] = i;
    m2[q + i] = -i;
    totalDiff += 2 * i;
  }

  ASSERT_EQ(norm1(m1,m2), totalDiff);
  ASSERT_EQ(norm1(m2,m1), totalDiff);
}
