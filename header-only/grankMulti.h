#ifndef GRANKMULTI_H
#define GRANKMULTI_H

#include <unordered_set>
#include <vector>
#include <stdlib.h>//exit
#include <utility>//make pair
#include <algorithm>//max
#include <thread>
#include <chrono>

#include <pprInternal.h>

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::cerr; using std::endl;
using std::make_pair;
using std::max;
using std::swap;
using std::thread;

using ppr::pprInternal::keepTop;
using ppr::pprInternal::norm1;
using ppr::pprInternal::findPartitions;

namespace ppr
{

namespace grankMultiInternal
{

  template<typename Key, typename It>
  inline void combineMaps(It begin, It end, const unordered_map<Key, vector<Key>>& graph,
    const unordered_map<Key, unordered_map<Key, double>>& scores,
    unordered_map<Key, unordered_map<Key, double>>& nextScores,
    double& maxDiff, const size_t L, const double damping)
  {
    //get nextScores map for current vertex, clear it and obtain results by combining
    //maps from the successors
    for(auto it = begin; it != end; it++)
    {
      const Key& v = *it;
      unordered_map<Key, double> currentMap;
      currentMap.reserve(nextScores[v].size());
      currentMap.insert(make_pair(v, 1.0 - damping));

      //get successors
      const vector<Key>& successors = graph.find(v)->second;
      double factor = damping / successors.size();

      for(const Key& successor: successors)
      {
        /**
         * for each value of personalized pagerank (max L values) saved
         * in the map  of a successor increment the personalized pagerank of v
         * for that key of a fraction of it.
         */
         for(const auto& keyValue: scores.find(successor)->second)
           currentMap[keyValue.first] += keyValue.second * factor;
      }

      //keep the top L values only
      keepTop(L, currentMap);

      //check difference between new and old map for this now and eventually
      //updated the maxDiff
      maxDiff = max(maxDiff, norm1(currentMap, scores.find(v)->second));

      currentMap.swap(nextScores[v]);
    }
  }
}

/**
 * Approximated Personalized Pagerank for all nodes in the graph. The graph
 * is an unordered_map where each key is a node, and is mapped to a vector of
 * nodes for which an edge exists between the key node and the nodes in the vector.
 * Nodes which have no edges must still be part of the map, and are mapped to an
 * empty vector.
 * @param graph      Graph for which to calculate ppr for all sources.
 * @param K          Number of entries (nodes) for each source, the ppr top-K scoring nodes for the source node.
 * @param L          Number of entries (nodes) for each source to store during computation.
 * @param iterations Max number of iterations of the algorithm (it might be stopped by the tolerance).
 * @param damping    Damping factor, a la Pagerank.
 * @param tolerance  Stopping tolerance based on the norm-1 between old and new top-L, a negative
 * tolerance can be used to have no tolerance at all, making it so that the
 * algorithm stops only once the max number of iterations are done.
 * @param nThreads Number of threads to use (one at least).
 * @return Maps of each node, storing theirs personalized pagerank top-K basket.
 */
template<typename Key>
unordered_map<Key, unordered_map<Key, double>> grankMulti(const unordered_map<Key, vector<Key>>& graph, //the graph
size_t K,//small top, K <= L
size_t L,//large top
size_t iterations,//max number of iterations
double damping,//damping factor
double tolerance,//tolerance
size_t nThreads)//number of threads, at least 1
{
  //checking parameters
  if(K == 0){cerr << "K must be positive" << endl; exit(EXIT_FAILURE);}
  if(L == 0){cerr << "L must be positive" << endl; exit(EXIT_FAILURE);}
  if(K > L){cerr << "K must be <= L" << endl; exit(EXIT_FAILURE);}
  if(iterations == 0){cerr << "iterations must be positive" << endl; exit(EXIT_FAILURE);}
  if(damping < 0 || damping > 1){cerr << "damping must be [0,1]" << endl; exit(EXIT_FAILURE);}
  if(nThreads == 0){cerr << "nThreads must be positive" << endl; exit(EXIT_FAILURE);}
  //note: no checks on tolerance to allow having no tolerance at all by setting
  //it to a negative number

  //get a vector of all keys (used for multi threaded initialization and clean up
  //of the maps, with clean up i mean the keepTop(K, map) at the end of this method
  vector<Key> allKeys; allKeys.reserve(graph.size());

  //allocate scores maps and init each map so that threads using scores and
  //nextScores don't cause any change in scores and nextScores due to
  //rehashing or whatever
  unordered_map<Key, unordered_map<Key, double>> scores; scores.reserve(graph.size());
  unordered_map<Key, unordered_map<Key, double>> nextScores; nextScores.reserve(graph.size());
  for(const auto& keyVal: graph)
  {
    allKeys.push_back(keyVal.first);
    scores[keyVal.first];
    nextScores[keyVal.first];
  }

  //multi threaded initialization
  vector<thread> threads;
  size_t chunk = allKeys.size()/nThreads;
  for(size_t t = 0; t < nThreads; t++)
  {
    auto begin = allKeys.begin() + (chunk * t);
    auto end = allKeys.begin() + (chunk * (t + 1));
    if(t == nThreads - 1)
      end = allKeys.end();

    //threads.emplace([captured stuff](arguments){body of the lambda}, arguments ...}
    threads.emplace_back([L, damping, &scores, &graph](typename vector<Key>::iterator begin, typename vector<Key>::iterator end)
      {
        for(auto it = begin; it != end; it++)
        {
          const Key& node = *it;

          //get successors of node
          const vector<Key>& successors = graph.find(node)->second;
          double factor = damping / successors.size();

          //assign to itself a score of 1 - damping
          //scores[node] retrieves the map of scores for source node node
          //scores[node][node] is operating on the map of scores of source node "node"
          scores[node][node] = 1.0 - damping;

          for(const Key& successor: successors)
            scores[node][successor] += factor;

          keepTop(L, scores[node]);
        }
      }
      , begin, end);
  }
  for(auto& t: threads)
    t.join();
  threads.clear();

  pair<unordered_set<Key>, unordered_set<Key>> partitions = findPartitions<Key>(graph);
  pair<vector<Key>, vector<Key>> partitionsV(std::make_pair(vector<Key>(), vector<Key>()));
  std::copy(partitions.first.begin(), partitions.first.end(), std::back_inserter(partitionsV.first));
  std::copy(partitions.second.begin(), partitions.second.end(), std::back_inserter(partitionsV.second));

  //max difference between old and new map between iterations, a variable for each
  //partition is needed to avoid some edge cases where a very simple partitition (i.e. no edges etc.)
  //might make the algorithm converge during the first iteration, before the
  //other partition is considered
  double maxDiff[2] = {tolerance, tolerance};

  for(size_t i = 0; i < iterations && max(maxDiff[0], maxDiff[1]) >= tolerance; i++)
  {
    maxDiff[0] = 0;

    //multi threaded combination of direct successors maps for every node
    vector<double> maxDiffs(nThreads, 0);//used for the maxDiff of every thread
    size_t chunk = partitionsV.first.size()/nThreads;
    for(size_t t = 0; t < nThreads; t++)
    {
      auto begin = partitionsV.first.begin() + (chunk * t);
      auto end = partitionsV.first.begin() + (chunk * (t + 1));
      //cout << chunk * t <<  " - " << ((t == nThreads - 1)? partitionsV.first.size() :chunk * (t + 1)) << endl;
      if(t == nThreads - 1)
        end = partitionsV.first.end();

      //threads.emplace(function, arguments...)
      threads.emplace_back(ppr::grankMultiInternal::combineMaps<Key, typename vector<Key>::iterator>,
        begin, end, std::ref(graph),
        std::ref(scores), std::ref(nextScores), std::ref(maxDiffs[t]), L, damping);
    }

    for(auto& t: threads)
      t.join();
    threads.clear();

    //swap partitions
    partitionsV.first.swap(partitionsV.second);

    //carry on results for the partition that wasn't elaborated
    //during this iteration to the next iteration
    for(const Key& v: partitionsV.first)
      nextScores[v].swap(scores[v]);

    for(double m: maxDiffs)
      maxDiff[0] = max(maxDiff[0], m);

    //swap scores (results from this iteration are the new current results)
    scores.swap(nextScores);

    //swap diffs
    swap(maxDiff[0], maxDiff[1]);
  }

  //keep K top entries for all maps, multi threaded
  for(size_t t = 0; t < nThreads; t++)
  {
    auto begin = allKeys.begin() + (chunk * t);
    auto end = allKeys.begin() + (chunk * (t + 1));
    if(t == nThreads - 1)
      end = allKeys.end();

    //threads.emplace([captured stuff](arguments){body of the lambda}, arguments ...}
    threads.emplace_back([K, &scores](typename vector<Key>::iterator begin, typename vector<Key>::iterator end)
      {
        for(auto it = begin; it != end; it++)
          keepTop(K, scores[(*it)]) ;
      }
      , begin, end);
  }
  for(auto& t: threads)
    t.join();

  return scores;
}

}
#endif
