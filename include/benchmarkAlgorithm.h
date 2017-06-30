#ifndef BENCHMARKALGORITHM_H
#define BENCHMARKALGORITHM_H

#include <algorithm>//min
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>//make pair
#include <stdlib.h>//exit
#include <string>
#include <random>
#include <iostream>

#include <pprSingleSource.h>
#include <pprInternal.h>
#include <kendall.h>

using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::cout; using std::endl;

namespace ppr
{
  /**
   * Given a map mapping source nodes to a basket containing top-K personalized
   * Pagerank scores confront the provided results with those provided by
   * classic Pagerank for a given number of source nodes, chosen randomly among
   * the keys of the provided map.
   * Pagerank parameters: 100 max iterations, 0.85 damping, 0.0001 tolerance.
   * @param ppr       Map of source nodes and their personalized pagerank baskets.
   * @param graph     Graph on which the provided pagerank scores are based on.
   * @param testNodes Number of sample nodes, equals to the number of times
   * pagerank will be run.
   * @param strict    If true nodes with outdegree equal 0 will be skipped while
   * randomly picking sample nodes. This is to avoid having those nodes inflating
   * the jaccard and kendall values.
   * @return Returns a map mapping names of statistics to their value: the jaccard
   * average and min, the kendall average and min, the average size of maps from
   * the "ppr" parameter. This last statistic might be useful because while experimenting
   * with the parameters using a very low number of iterations might make the
   * number of reached nodes (and thus the map containing the scoring nodes) lower
   * than intended (i.e. having a K = 50 but max iterations set to 2 might lead
   * to an average map size of less than 50).
   */
  template<typename Key>
  unordered_map<string, double> benchmarkAlgorithm(const unordered_map<Key, unordered_map<Key, double>>& ppr,
    const unordered_map<Key, vector<Key>>& graph, size_t testNodes, bool strict)
  {
    unordered_map<string, double> result;

    // shuffle nodes to benchmark the algorithm on a number of random nodes equal to
    // testNodes parameter
    std::random_device rd;
    std::mt19937 g(rd());
    vector<Key> nodes;
    for(const auto& keyVal: ppr)
    {
      if(graph.find(keyVal.first) == graph.cend())
      {
        cout << "node " << keyVal.first << " in the provided map is not part of the provided graph" << endl;
        exit(EXIT_FAILURE);
      }

      if(strict)
      {
        if(graph.find(keyVal.first)->second.size() != 0)
          nodes.push_back(keyVal.first);
      }
      else
          nodes.push_back(keyVal.first);
    }
    shuffle(nodes.begin(), nodes.end(), g);

    double jaccardAverage = 0;
    double jaccardMin = 1.0;
    double kendallAverage = 0;
    double kendallMin = 1.0;
    double averageMapSize = 0;
    for(size_t i = 0; i < nodes.size(); i++)
      {
        const Key& node = nodes[i];
        const unordered_map<Key, double>& otherAlgo = ppr.find(node)->second;

        unordered_map<Key, double> pagerank = pprInternal::pprSingleSource<Key>(graph, 100, 0.85, 0.0001, node);
        //needed for kendall
        unordered_map<Key, double> bkup(pagerank);
        //have the top-K be the same of the size of the top-K from the approximation algorithm, needed for jaccard
        pprInternal::keepTop<Key>(otherAlgo.size(), pagerank);

        //jaccard stuff
        //create sets
        unordered_set<Key> setOther;
        setOther.reserve(otherAlgo.size());
        unordered_set<Key> setPagerank;
        setPagerank.reserve(otherAlgo.size());
        for(const auto& keyVal: otherAlgo)
          setOther.insert(keyVal.first);
        for(const auto& keyVal: pagerank)
          setPagerank.insert(keyVal.first);

        double jaccard = pprInternal::jaccard<Key>(setOther, setPagerank);
        jaccardAverage += jaccard;
        jaccardMin = std::min(jaccardMin, jaccard);

        //kendall stuff
        //push pairs of scores (seen as two vectors) of the nodes of the top-K of
        //the benchmarked algorithm, a pair is formed by the score given by
        //the benchmarked algorithm and by classic pagerank
        vector<double> otherScores;
        otherScores.reserve(otherAlgo.size());
        vector<double> pagerankScores;
        pagerankScores.reserve(otherAlgo.size());
        for(const auto& keyVal: otherAlgo)
        {
          otherScores.push_back(keyVal.second);
          pagerankScores.push_back(bkup[keyVal.first]);
        }

        double kendall = kendallCorrelation(otherScores, pagerankScores);
        kendallAverage += kendall;
        kendallMin = std::min(kendallMin, kendall);

        averageMapSize += otherAlgo.size();
      }

    jaccardAverage /= nodes.size();
    kendallAverage /= nodes.size();
    averageMapSize /= nodes.size();
    result["jaccard average"] = jaccardAverage;
    result["jaccard min"] = jaccardMin;
    result["kendall average"] = kendallAverage;
    result["kendall min"] = kendallMin;
    result["average map size"] = averageMapSize;
    return result;
  }
}
#endif
