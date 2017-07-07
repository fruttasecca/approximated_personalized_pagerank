# Personalized Pagerank approximation algorithms

Two c++11 Personalized Pagerank approximation algorithms, which was the subject of my thesis.
In the directory "header-only" you can find their header-only implementation, so that you can just
copy the header and use it in your projects.  
If you wish to know the inner workings of the algorithms the easier way to do so is to check out
the [thesis](gobbi_jacopo_informatica_2017.pdf), which contains the pseudocode, the reasoning behind
the algorithms, advice on how to set the parameters, credits, and much more.
If you use the header-only versions in a project of yours I suggest to compile with at least -O2, since
the difference is noticeable.

## Header-only
In the directory "header-only" you can find header-only implementations of the algorithms.
If you are interested in reading the algorithms it's better to read the non header only versions from "include", or learn directly their inner workings from the pdf document in the project directory.

## Prerequisites

c++11, cmake, pthread (only for compiling and running the example or for using grankMulti.h)

## Getting Started

Get into the project directory and do
```
cmake CMakeLists.txt
```

This will configure your make file and clone gtest in your project directory from their repo.  
After having your make file ready you can use "make" to compile the project into a binary
file named "ppr", it's a simple example of getting a graph from a csv file, approximating ppr
with one of the algorithms and sampling some nodes to have a general idea of the goodness of the approximation.  
You can use the example to try out different parameters on your graphs to see
what kind of results you can expect.

## Usage

The provided algorithms work in the following manner:
```c++
template<typename Key>
unordered_map<Key, unordered_map<Key, double>> res =  algorithm(unordered_map<key, vector<key>> graph, ... rest of the parameters);
```
A key can be anything that has a std::hash specialization, to specialize something all you
have to do is
```c++
// example taken from http://en.cppreference.com
// custom specialization of std::hash can be injected in namespace std
namespace std
{
    template<> struct hash<S>
    {
        typedef S argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type const h1 ( std::hash<std::string>{}(s.first_name) );
            result_type const h2 ( std::hash<std::string>{}(s.last_name) );
            return h1 ^ (h2 << 1);
        }
    };
}
```
Despite these functions being templates, and allowing basically anything as a Key, mind that Keys are going to be
copied during the call of the functions, this obviously happens at least once, since the returned unordered_map
contains Keys, and not wrapped references.
For this reason it is best to use primitive or extremely simple types as Keys.
I have decided to not use wrapped references because it would make the implementation more complex without
actually having real use cases.

A graph is simply a map mapping each node to a vector containing all the direct successors
of the node. Nodes with no direct successors must be mapped to an empty vector, which can
be simply done by doing:
```
//this will default initialize an empty vector mapped to myLonelyNode
graph[myLonelyNode];
```

The algorithms return a map which maps each node in the graph to a basket
of nodes (which is actually just another map mapping nodes to a score).
To get the approximated Personalized Pagerank of a node (as if it was the only node in the
teleport set of classic Pagerank) you can do
```
auto res = algorithm(unordered_map<key, vector<key>> graph, ... rest of the parameters);
unordered_map<key, double> myNodePPR = res[myNode];
```
"Res" will have the approximated Personalized Pagerank results for each node in the graph.
"MyNodePPR" will contain the pagerank score of the top-K scoring (where K is a parameter) nodes for
a source node, "myNode".

## GRank
GRank is based on iteratively combining the maps of each node direct successors, until convergence or
until all iterations are done.
Let's say you want to use the header-only version and run it on a graph, the minimum you have to do is:

```c++
#include <unordered_map>
#include <iostream>
#include <vector>

#include "grank.h"

using namespace std;
using ppr::grank;

int main()
{
	size_t K = 50;
	size_t L = 100;
	size_t iterations = 30;
	double damping = 0.85;
	double tolerance = 0.001;
	unordered_map<int, vector<int>> graph;
	for(int i = 0; i < 100; i++)
		graph[i].push_back( (i+1)%100);

	auto ppr = grank(graph, K, L, iterations, damping, tolerance);

	return 0;
}

```
Compiling can then be simply done with `g++ main.cc -std=c++11 -O2`.  
The map `ppr` will contain `K` top scoring nodes for each node in the graph. `L` controls how large the maps
of each node are during computation, it is strongly recommended to have `L > K`, since it really affects results.  
Damping works like in Pagerank, so keeping it at 0.85 is a good choice.
A `tolerance` between 0.0001 and 0.01 is recommended, and `iterations` between 10 and 40 should do the trick.  

## GRank multi threaded
The multi threaded version has one more parameter, which controls the number of threads.
```c++
#include <unordered_map>
#include <iostream>
#include <vector>

#include "grankMulti.h"

using namespace std;
using ppr::grankMulti;

int main()
{
	size_t K = 50;
	size_t L = 100;
	size_t iterations = 300;
	double damping = 0.85;
	double tolerance = 0.001;
	size_t nThreads = 4;
	unordered_map<int, vector<int>> graph;
	for(int i = 0; i < 100; i++)
		graph[i].push_back( (i+1)%100);

	auto ppr = grankMulti(graph, K, L, iterations, damping, tolerance, nThreads);

	return 0;
}
```
Remember that you will need to link pthread now: `g++ main.cc -std=c++11 -O2 -lpthread`.

## MCCompletePathV2
MCComppletePathV2 is a probabilistic algorithm based on doing random walks and
combining the results between neighbours and propagating them through the graph.
Example using header-only:
```c++
#include <unordered_map>
#include <iostream>
#include <vector>

#include "mccompletepathv2.h"

using namespace std;
using ppr::mccompletepathv2;

int main()
{
    size_t K = 50;
    size_t L = 100;
    size_t iterations = 1000;
    double damping = 0.85;

    unordered_map<int, vector<int>> graph;
    for(int i = 0; i < 100; i++)
        graph[i].push_back( (i+1)%100);

    auto ppr = mccompletepathv2(graph, K, L, iterations, damping);

    return 0;
}
```
`K` and `L` work in the exact same way as in GRank (even the fact that `K > L` is strongly
  suggested), same for the `damping.`  
`Iterations` now stands for how many random walks are done for each node in the worst case,
so we are talking about figures much higher than `iterations` in GRank.  
Compile with: `g++ main.cc -std=c++11 -O2 `.
## Running the tests

```
make tests
```
As the name suggests this is going to compile the tests. Once compiled
simply run them as "./pprTest".
There are no tests for the implementation of the kendall algorithm because i took the header-only
implementation from another repository of mine, named kendall (which is where the kendall tests are).
