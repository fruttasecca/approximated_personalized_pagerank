# Personalized Pagerank approximation algorithms

Two c++11 Personalized Pagerank approximation algorithms, which was the subject of my thesis.

## Prerequisites

c++11, cmake

## Getting Started

Get into the project directory and do
```
cmake CMakeLists.txt
```

This will configure your make file and clone gtest in your project directory from their repo.

```
make
```
After having your make file ready you can use "make" to compile the project into a binary
file named "ppr", it's a simple example of getting a graph from a csv file, approximating ppr with one of the algorithms and sampling some nodes to have a general idea of the goodness of the approximation.

## Usage

The provided algorithms work in the following manner:
```
template<typename Key>
unordered_map<Key, unordered_map<Key, double>> res =  algorithm(unordered_map<key, vector<key>> graph, ... rest of the parameters);
```
A key can be anything that has a std::hash specialization, to specialize something all you 
have to do is
```
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

## Running the tests

```
make tests
```
As the name suggests this is going to compile the tests. Once compiled
simply run them as "./pprTest".

