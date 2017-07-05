#ifndef MCCOMPLETEPATHV2HEADER_H
#define MCCOMPLETEPATHV2HEADER_H

#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <vector>
#include <stdlib.h>//exit
#include <utility>//make pair
#include <algorithm>//max
#include <random>


using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::queue;
using std::cerr; using std::endl;
using std::make_pair;
using std::move;
using std::max;
using std::swap;
using std::tuple;
using std::get;
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
   * @param iterations Number of random walks to do for each node in the worst case.
   * @param damping    Damping factor, a la Pagerank.
   * @return Maps of each node, storing theirs personalized pagerank top-K basket.
   */
  template<typename Key>
  unordered_map<Key, unordered_map<Key, double>> mccompletepathv2(const unordered_map<Key, vector<Key>>& graph, //the graph
  size_t K,//small top
  size_t L,//large top
  size_t iterations,//number of monte carlo random walks for each node in the worst case
  double damping);//damping factor


  /*****************************************************************************
  The definition of mccompletepathv2 starts at around line 266 in this file, but you should
  probably read the algorithm from the more readable file "include/mccompletepathv2.h"
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

  namespace mccompletepathv2Internal
  {
    std::random_device pprDevice;
    std::mt19937 pprGenerator(ppr::mccompletepathv2Internal::pprDevice());
    std::uniform_real_distribution<double> pprDis(0.0, 1.0);

    template<typename Key>
    vector<Key> executionOrder(const unordered_map<Key, vector<Key>>& graph)
    {
      //get list of predecessors for each node
      unordered_map<Key, vector<Key>> predecessors;
      predecessors.reserve(graph.size());
      for(const auto& keyVal: graph)
      {
        const Key& node = keyVal.first;
        //init predecessors[v] to an empty vector, in case the node has no predecessors
        predecessors[node];
        const vector<Key>& successors = keyVal.second;

        for(const Key& v: successors)
          predecessors[v].push_back(node);
      }

      //tuple<node, indegree, outdegree>
      vector<tuple<Key, size_t, size_t>> data;
      data.reserve(graph.size());
      for(const auto& keyVal: graph)
        data.push_back(std::make_tuple(keyVal.first, predecessors[keyVal.first].size(), keyVal.second.size()));

      std::sort(data.begin(), data.end(),
        [](const tuple<Key, size_t, size_t>& t1, const tuple<Key, size_t, size_t>& t2)
        {
          return (get<1>(t1) > get<1>(t2))? true :
            (get<1>(t1) == get<1>(t2)? get<2>(t1) < get<2>(t2) : false);
        });

      vector<Key> sorted;
      sorted.reserve(data.size());
      for(const auto& t: data)
        sorted.push_back(get<0>(t));

      //after sorting the nodes use a heuristic to get a more
      //efficient order
      vector<Key> order;
      order.reserve(data.size());
      queue<Key> qu;

      //remaining successors to wait for
      unordered_map<Key, size_t> waitFor;
      waitFor.reserve(data.size());
      for(const auto& keyVal: graph)
        waitFor[keyVal.first] = keyVal.second.size();

      //keep track of visited nodes
      unordered_set<Key> visited;
      visited.reserve(data.size());

      for(const Key& node: sorted)
      {
        if(visited.find(node) == visited.end())
        {
          qu.push(node);
          while(!qu.empty())
          {
            const Key& next = qu.front();
            order.push_back(next);
            visited.insert(next);
            const vector<Key>& nodePredecessors = predecessors[next];
            qu.pop();

            /*
            for each predecessor decrement the remaining successors to wait
            for and eventually consider it done when the remaining successors
            get to 0
            */
            for(const Key& pred: nodePredecessors)
            {
              if(waitFor[pred]-- > 0)
              {
                //if the node doesn't have to wait for any successor
                //it could be computed
                if(!waitFor[pred] && visited.find(pred) == visited.end())
                  qu.push(pred);
              }
            }
          }
        }
      }
      return order;
    }

    template<typename Key>
    inline unordered_map<Key, double> walkNode(const unordered_map<Key, vector<Key>>& graph,
      unordered_map<Key, size_t>& index, Key node, const size_t K, double damping, size_t walks)
    {
      unordered_map<Key, double> res;
      if(graph.find(node)->second.size() > 0)
      {
        res.reserve(K);
        //each walk will surely start from the origin node
        res[node] = walks;
        size_t bk = walks;

        /*
        a part of the walks is wasted because a teleport happens before traversing
        the first edge, so we account for those walks here (lowering the total walks)
        but make it so that the first edge is always traversed
        */
        walks = static_cast<size_t> (static_cast<double>(walks) * damping);

        for(size_t i = 0; i < walks; i++)
        {
          const Key* currentNode = &node;

          /*
          random walk which stops if a teleport happens (teleported > damping)
          or if it gets into a node without out going edges
          */
          do
          {
            if(graph.find(*currentNode)->second.size() == 0)
              break;
            else
            {
              //increment index of the current node and pick the next node
              currentNode = &graph.find(*currentNode)->second[(index[*currentNode] = (++index[*currentNode]) % graph.find(*currentNode)->second.size())];

              //increment node score only if it won't make the map size greater than what's allowed
              if(res.find(*currentNode) != res.end() || res.size() < K)
                res[*currentNode]++;
            }
          }while(mccompletepathv2Internal::pprDis(mccompletepathv2Internal::pprGenerator) <= damping);

        }
        //divide by the number of walks done to obtain the mean
        for(auto& keyVal: res)
          keyVal.second /= bk;
      }
      else
        res[node] = 1.0;
      return res;
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
   * @param iterations Number of random walks to do for each node in the worst case.
   * @param damping    Damping factor, a la Pagerank.
   * @return Maps of each node, storing theirs personalized pagerank top-K basket.
   */
  template<typename Key>
  unordered_map<Key, unordered_map<Key, double>> mccompletepathv2(const unordered_map<Key, vector<Key>>& graph, //the graph
  size_t K,//small top
  size_t L,//large top
  size_t iterations,//number of monte carlo random walks for each node in the worst case
  double damping)//damping factor
  {
    //checking parameters
    if(K == 0){cerr << "K must be positive" << endl; exit(EXIT_FAILURE);}
    if(L == 0){cerr << "L must be positive" << endl; exit(EXIT_FAILURE);}
    if(K > L){cerr << "K must be <= L" << endl; exit(EXIT_FAILURE);}
    if(iterations == 0){cerr << "iterations must be positive" << endl; exit(EXIT_FAILURE);}
    if(damping < 0 || damping > 1){cerr << "damping must be [0,1]" << endl; exit(EXIT_FAILURE);}

    //allocate  maps
    //there is no map storing the results from the random walks because "scores"
    //is used to store them while the node still doesn't have a final result
    unordered_map<Key, unordered_map<Key, double>> scores;
    scores.reserve(graph.size());

    //each node has an index that tells which successor is going to be picked
    //next while moving away from the node during a random walk
    unordered_map<Key, size_t> index;
    index.reserve(graph.size());
    //init the index of every node at 0
    for(const auto& keyVal: graph)
      index[keyVal.first];

    vector<Key> order = mccompletepathv2Internal::executionOrder(graph);

    for(const Key& node: order)
    {
      unordered_map<Key, double> map;
      map.reserve(L * graph.find(node)->second.size());
      double factor = (graph.find(node)->second.size() == 0) ? 1.0 : damping / graph.find(node)->second.size();

      /*
      every walk starts from the node, this can't be added later otherwise
      keepTop might remove a small score for "node" and then adding 1 to the node
      will cause the map to have a size of smallTop + 1.
      division by the factor is needed to take into consideration
      the map multiplication of each value (see around line 248), which averages by outdegree and
      scales down values using the damping factor; since
      the score for the node itself must not be scaled down the division
      is performed
      */
      map[node] = 1.0 / factor;

      for(const Key& successor: graph.find(node)->second)
      {
        /*if nothing is mapped to the successor it means that there are no
        final results for that node AND that the node has not walked yet, so
        random walks are done for the node.
        When the node will be finally executed the map resulting from the walks
        will be simply "overwritten" by the final result.*/
        if(scores.find(successor) == scores.end())
        {
          scores.insert(make_pair(successor, move(ppr::mccompletepathv2Internal::walkNode(graph,
            index, successor, L, damping, iterations))));
        }
        for(const auto& keyVal: scores[successor])
          map[keyVal.first] += keyVal.second;
      }
      ppr::mccompletepathv2Internal::keepTop(L, map);

      //multiply each value in the map for the factor
      for(auto& keyVal: map)
        keyVal.second *= factor;

      scores[node] = move(map);//checkare che nn copia
    }

    for(auto& keyVal: scores)
    {
      ppr::mccompletepathv2Internal::keepTop(K, keyVal.second);
      keyVal.second.reserve(K);
      keyVal.second.rehash(K);
    }
    return scores;
  }
}
#endif
