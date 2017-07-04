#ifndef MCCOMPLETEPATHV2_H
#define MCCOMPLETEPATHV2_H

#include <unordered_set>
#include <vector>
#include <stdlib.h>//exit
#include <utility>//make pair
#include <algorithm>//max

#include <pprInternal.h>

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::cerr; using std::endl;
using std::make_pair;
using std::max;
using std::swap;

using ppr::pprInternal::keepTop;
using ppr::pprInternal::norm1;
using ppr::pprInternal::findPartitions;

namespace ppr
{
  namespace pprInternal
  {
    template<typename Key>
    vector<Key> executionOrder(const unordered_map<Key, vector<Key>>& graph)
    {
      vector<Key> order;

      return order;
    }

  }
  template<typename Key>
  unordered_map<Key, unordered_map<Key, double>> mccompletepathv2(const unordered_map<Key, vector<Key>>& graph, //the graph
  size_t K,//small top,
  size_t iterations,//number of monte carlo random walks for each node in the worst case
  double damping)//damping factor
  {
    //checking parameters
    if(K == 0){cerr << "K must be positive" << endl; exit(EXIT_FAILURE);}
    if(iterations == 0){cerr << "iterations must be positive" << endl; exit(EXIT_FAILURE);}
    if(damping < 0 || damping > 1){cerr << "damping must be [0,1]" << endl; exit(EXIT_FAILURE);}

    //allocate scores maps
    unordered_map<Key, unordered_map<Key, double>> scores;
    scores.reserve(graph.size());

    //each node has an index that tells which successor is going to be picked
    //next while moving away from the node during a random walk
    unordered_map<Key, size_t> index;
    index.reserve(graph.size());
    //init the index of every node at 0
    for(const auto& keyVal: graph)
      index[keyVal.first];

    vector<Key> order = pprInternal::executionOrder(graph);



    cout << "memed" << endl;
    return scores;
  }
}
#endif
