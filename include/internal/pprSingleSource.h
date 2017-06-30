#ifndef PPRSINGLESOURCE_H
#define PPRSINGLESOURCE_H

#include <unordered_map>
#include <stdlib.h>//exit

#include <pprInternal.h>
#include <pprSingleSource.h>

using std::unordered_set;
using std::unordered_map;
using std::queue;
using std::cout; using std::endl;

namespace ppr
{
  namespace pprInternal
  {
    /**
     * Returns the personalized Pagerank of a single source node.
     * @param graph
     * @param iterations Max number of iterations to run.
     * @param damping    Pagerank damping factor;
     * @param tolerance  Stopping tolerance based on the norm-1 between the old and new map at each iteration, a negative
     * tolerance can be used to have no tolerance at all, making it so that the
     * algorithm stops only once the max number of iterations are done.
     * @param source    Node for which to calculate the ppr.
     */
    template<typename Key>
    unordered_map<Key, double> pprSingleSource(const unordered_map<Key, vector<Key>>& graph, //the graph
      size_t iterations,//max number of iterations
      double damping,//damping factor
      double tolerance,//tolerance
      Key source)//source node for which ppr is going to be computed
      {
        //checking parameters
        if(iterations <= 0){cout << "iterations must be positive" << endl; exit(EXIT_FAILURE);}
        if(damping < 0 || damping > 1){cout << "damping must be [0,1]" << endl; exit(EXIT_FAILURE);}
        if(graph.find(source) == graph.end()){cout << "source node not part of the graph" << endl; exit(EXIT_FAILURE);}
        //note: no checks on tolerance to allow having no tolerance at all by setting
        //it to a negative number

        unordered_map<Key, double> scores;
        unordered_map<Key, double> nextScores;

        //init the source node has having a score of 1
        scores[source] = 1.0;

        double diff = tolerance;
        for(size_t i = 0; i < iterations && diff >= tolerance; i++)
        {
            diff = 0;
            //clear the map and set the score of the source at the teleport contribution
            nextScores.clear();
            nextScores[source] = 1.0 - damping;

            //move score from each node towards its children
            for(const auto& keyVal: scores)
            {
              const Key& father = keyVal.first;
              const double score = keyVal.second;
              const vector<Key>& successors = graph.find(father)->second;
              double factor = damping/successors.size();

              for(const Key& successor : successors)
                nextScores[successor] += score * factor;
            }

            //check if the norm1 of the difference is greater than the maxDiff
            diff = ppr::pprInternal::norm1(scores, nextScores);

            scores.swap(nextScores);
        }

        return scores;
      }
  }
}
#endif
