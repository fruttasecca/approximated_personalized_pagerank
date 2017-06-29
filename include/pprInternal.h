#ifndef PPRINTERNAL_H
#define PPRINTERNAL_H

#include <algorithm>//max
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>//make pair
#include <queue>
#include <stdlib.h>//exit

using std::unordered_set;
using std::unordered_map;
using std::queue;

namespace ppr
{
  namespace pprInternal
  {
    /**
     * Given a graph which is an unordered_map which maps nodes to
     * a list of their direct successors get two partitions.
     * @param graph The graph for which to find two partitions.
     */
    template<typename Key>
    inline pair<unordered_set<Key>, unordered_set<Key>> findPartitions(const unordered_map<Key, vector<Key>>& graph)
    {
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
        }
      }

      return partitions;
    }


    /**
     * Keep the top-L scoring elements (key-val pairs), a pair scores better
     * than another if it's value is greater than the value of the other.
     * @param L Number of elements to retain.
     * @param m Unordered_map for which to keep the top-L elements.
     */
    template<typename Key>
    inline void keepTop(size_t L, unordered_map<Key, double>& m)
      {
        //make vector of pairs and sort it
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
}

#endif
