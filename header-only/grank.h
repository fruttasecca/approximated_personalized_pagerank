#ifndef GRANK_H
#define GRANK_H

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdlib.h>//exit
#include <utility>//make pair
#include <algorithm>//max
#include <queue>

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::cerr; using std::endl;
using std::make_pair;
using std::max;
using std::swap;
using std::queue;
using std::pair;


namespace ppr
{
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
   * @return Map of maps of each source node, mapping the node to a basket of top-K highest scoring
   * nodes.
   */
  template<typename Key>
  unordered_map<Key, unordered_map<Key, double>> grank(const unordered_map<Key, vector<Key>>& graph, //the graph
  size_t K,//small top, K <= L
  size_t L,//large top
  size_t iterations,//max number of iterations
  double damping,//damping factor
  double tolerance);//tolerance


  /*****************************************************************************
  The definition of grank starts at around line 217 in this file, but you should
  probably read the algorithm from the more readable file "include/grank.h"
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  ******************************************************************************
  *****************************************************************************/

  namespace grankInternal
  {
    /**
     * Given a graph which is an unordered_map which maps nodes to
     * a list of their direct successors get two partitions.
     * @param graph The graph for which to find two partitions.
     */
    template<typename Key>
    inline pair<unordered_set<Key>, unordered_set<Key>> findPartitions(const unordered_map<Key, vector<Key>>& graph)
    {
      //get list of predecessors for each node
      unordered_map<Key, vector<Key>> predecessors;
      for(const auto& keyVal: graph)
      {
        const Key& node = keyVal.first;
        //init predecessors[v] to an empty vector, in case the node has no successors
        predecessors[node];
        const vector<Key>& successors = keyVal.second;

        for(const Key& v: successors)
          predecessors[v].push_back(node);
      }

      pair<unordered_set<Key>, unordered_set<Key>> partitions(std::make_pair(unordered_set<Key>(), unordered_set<Key>()));

      //queue for going breadth first
      queue<Key> que;

      //keeps track of visited nodes
      unordered_set<Key> visited;
      visited.reserve(graph.size());

      for(const auto& keyVal: graph)
      {
        //if it has not been already visited add it to the visited nodes, to partition
        //one and to the queue
        if(visited.find(keyVal.first) == visited.cend())
          {
            visited.insert(keyVal.first);
            que.push(keyVal.first);

            partitions.first.insert(keyVal.first);
          }

        //extract from queue and assign its non-visited successors to the complimentary
        //partition
        while(!que.empty())
        {
          Key next = que.front();
          que.pop();
          /* check if it belongs to the first partition, all direct successors
          that are not visited will be put in the other partition*/
          unordered_set<Key>& currentPartition =
            (partitions.first.find(next) == partitions.first.cend())? partitions.first : partitions.second;
          for(const Key& successor: graph.find(next)->second)
          {
            if(visited.find(successor) == visited.cend())
            {
              visited.insert(successor);
              currentPartition.insert(successor);
              que.push(successor);
            }
          }

          for(const Key& predecessor: predecessors.find(next)->second)
          {
            if(visited.find(predecessor) == visited.cend())
            {
              visited.insert(predecessor);
              currentPartition.insert(predecessor);
              que.push(predecessor);
            }
          }
        }
      }

      return partitions;
    }


    /**
     * Keep the top-L scoring elements (key-val pairs), a pair scores better
     * than another if it's value is greater than the value of the other.
     * If L is greater than the size of the map the function call has no effect.
     * @param L Number of elements to retain.
     * @param m Unordered_map for which to keep the top-L elements.
     */
    template<typename Key>
    inline void keepTop(size_t L, unordered_map<Key, double>& m)
    {
      if(m.size() > L)
      {
        //make vectors of pairs and partially sort it
        vector<pair<Key, double>> data(m.cbegin(), m.cend());

        std::nth_element(data.begin(), data.begin() + L, data.end(),
          [](const pair<Key, double>& p1, const pair<Key, double>& p2)
          { return p1.second > p2.second;});

        //if elements to remove are less than L just remove them
        if(m.size() - L < L)
        {
          for(auto it = data.cbegin() + L, end = data.cend(); it != end; it++)
            m.erase(it->first);
        }
        else
        {
          //if there would be a lot of elements to erase just make another map
          //and fill it, then swap contents
          unordered_map<Key, double> newMap;
          newMap.reserve(m.size());
          newMap.insert(data.cbegin(), data.cbegin() + L);
          newMap.swap(m);
        }
      }
    }


    /**
     * Calculate the norm1 between two unordered_maps, as if they were 2 vectors
     * where the value for unmapped elements is 0.
     * @param  m1 First map.
     * @param  m2 Second map.
     * @return    Norm-1 between the two maps.
     */
    template<typename Key>
    inline double norm1(const unordered_map<Key, double>& m1, const unordered_map<Key, double>& m2)
    {
      double res = 0;

      for(const auto& keyVal: m1)
      {
        //find instead of operator[] is used to avoid default initializing values
        //of unmapped keys to 0
        auto it = m2.find(keyVal.first);
        res += std::abs(keyVal.second - (it == m2.end()? 0 : it->second));
      }

      //check for keys not part of m1
      for(const auto& keyVal: m2)
        res += (m1.find(keyVal.first) == m1.end())? keyVal.second : 0;

      return res;
    }
}

  template<typename Key>
  unordered_map<Key, unordered_map<Key, double>> grank(const unordered_map<Key, vector<Key>>& graph, //the graph
  size_t K,//small top, K <= L
  size_t L,//large top
  size_t iterations,//max number of iterations
  double damping,//damping factor
  double tolerance)//tolerance
  {
    //checking parameters
    if(K == 0){cerr << "K must be positive" << endl; exit(EXIT_FAILURE);}
    if(L == 0){cerr << "L must be positive" << endl; exit(EXIT_FAILURE);}
    if(K > L){cerr << "K must be <= L" << endl; exit(EXIT_FAILURE);}
    if(iterations == 0){cerr << "iterations must be positive" << endl; exit(EXIT_FAILURE);}
    if(damping < 0 || damping > 1){cerr << "damping must be [0,1]" << endl; exit(EXIT_FAILURE);}
    //note: no checks on tolerance to allow having no tolerance at all by setting
    //it to a negative number

    //allocate scores maps
    unordered_map<Key, unordered_map<Key, double>> scores;
    scores.reserve(graph.size());
    unordered_map<Key, unordered_map<Key, double>> nextScores;
    nextScores.reserve(graph.size());

    //init score for each vertex  in the graph
    for(const auto& keyVal: graph)
    {
      //retrieve key value (the vertex) from each key-value pair
      Key v = keyVal.first;

      //get successors of node
      const vector<Key>& successors = graph.find(v)->second;
      double factor = damping / successors.size();

      //assign to itself a score of 1 - damping
      //scores[v] retrieves the map of scores for source node v
      //scores[v][v] is operating on the map of scores of source node v
      scores[v][v] = 1.0 - damping;

      //add score to each neighbour (needs += because a node might have an edge to itself)
      for(const Key& successor: successors)
        scores[v][successor] += factor;

      ppr::grankInternal::keepTop(L, scores[v]);
    }

    pair<unordered_set<Key>, unordered_set<Key>> partitions = ppr::grankInternal::findPartitions<Key>(graph);
    //max difference between old and new map between iterations, a variable for each
    //partition is needed to avoid some edge cases where a very simple partitition (i.e. no edges etc.)
    //might make the algorithm converge during the first iteration, before the
    //other partition is considered
    double maxDiff[2] = {tolerance, tolerance};

    for(size_t i = 0; i < iterations && max(maxDiff[0], maxDiff[1]) >= tolerance; i++)
    {
      maxDiff[0] = 0;

      for(const Key& v: partitions.first)
      {
        //get nextScores map for current vertex, clear it and obtain results by combining
        //maps from the successors
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
           for(const auto& keyValue: scores[successor])
             currentMap[keyValue.first] += keyValue.second * factor;
        }

        //keep the top L values only
        ppr::grankInternal::keepTop(L, currentMap);

        //check difference between new and old map for this now and eventually
        //updated the maxDiff
        maxDiff[0] = max(maxDiff[0], ppr::grankInternal::norm1(currentMap, scores[v]));

        currentMap.swap(nextScores[v]);
      }

      //swap partitions
      partitions.first.swap(partitions.second);

      //carry on results for the partition that wasn't elaborated
      //during this iteration to the next iteration
      for(const Key& v: partitions.first)
        nextScores[v].swap(scores[v]);

      //swap scores (results from this iteration are the new current results)
      scores.swap(nextScores);

      //swap diffs
      swap(maxDiff[0], maxDiff[1]);
    }

    for(auto& keyVal: scores)
      ppr::grankInternal::keepTop(K, keyVal.second);

    return scores;
  }
}
#endif
