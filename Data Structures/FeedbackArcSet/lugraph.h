#ifndef __LUGRAPH_H__
#define __LUGRAPH_H__

#include <stdlib.h>
#include <stdbool.h>
#include "gmap.h"
#include "mergesort.h"

typedef struct lugraph lugraph;
typedef struct lug_search lug_search;
typedef struct edge edge;
typedef struct dfs_struct dfs_struct;
typedef struct degree_sort degree_sort;
typedef struct topo_sort topo_sort;

/**
 * Creates a new undirected graph with the given number of vertices.  The
 * vertices will be numbered 0, ..., n-1.
 *
 * @param n a nonnegative integer
 * @return a pointer to the new graph
 */
lugraph *lugraph_create(size_t n, gmap *vertices, char** names);
 
/**
 * checks for errror in the graph creation
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
void error(lugraph *g);

/**
 * prints adjacency list
 * @param g a pointer to an undirected graph, non-NULL
 * @param names list of character names
 * @return the number of vertices in that graph
 */
void print_adj_list(lugraph *g, char** names);

/**
 * Returns the number of vertices in the given graph.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
size_t lugraph_size(const lugraph *g);

/**
 * Adds an undirected edge between the given pair of vertices to
 * the given undirected graph.  The behavior is undefined if the edge
 * already exists.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v1 the index of a vertex in the given graph
 * @param v2 the index of a vertex in the given graph, not equal to from
 */
void lugraph_add_edge(lugraph *g, size_t v1, size_t v2);

/**
 * Determines if the given undirected graph contains an edge between
 * the given vertices.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v1 the index of a vertex in the given graph
 * @param v2 the index of a vertex in the given graph, not equal to from
 * @return true if and only if the edge exists
 */
bool lugraph_has_edge(const lugraph *g, size_t v1, size_t v2);

/**
 * Returns the degree of the given vertex in the given graph.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v the index of a vertex in the given graph
 * @return the degree of v
 */
size_t lugraph_degree(const lugraph *g, size_t v);

/**
 * Returns the result of running breadth-first search on the given
 * graph starting with the given vertex.  When the search arrives
 * at a vertex, its neighbors are considered in the order the
 * corresponding edges were added to the graph.  The result is returned
 * as a pointer to a lug_search that must subsequently be passed to
 * lugraph_search_destroy. 
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param from the index of a vertex in the given graph
 * @return a pointer to the result of the search
 */
lug_search *lugraph_dfs(const lugraph *g, size_t from, int *cycle);

/**
 * Determines if the two given vertices are connected by some path
 * in the given graph.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v1 the index of a vertex in the given graph
 * @param v2 the index of a vertex in the given graph
 * @return true if the two vertices are connected in the graph, false otherwise
 */
bool lugraph_connected(const lugraph *g, size_t v1, size_t v2, int *cycle);

/**
 * Destroys the given undirected graph.
 *
 * @param g a pointer to a undirected graph, non-NULL
 */
void lugraph_destroy(lugraph *g);

/**
 * Returns the vertices visited by the given search in the order
 * they were first visited by the search.  Returns NULL if there is
 * an allocation error, in which case the contents of n are undefined.
 * It is the caller's responsibility to free the returned array if it
 * is non-NULL.
 *
 * @param s a pointer to a search result, non-NULL
 * @param n a pointer to an int
 * @return a pointer to an array of distinct vertex indices
 */
size_t *lug_search_visited(const lug_search *s, size_t *n);

/**
 * Destroys the given search result.
 *
 * @param s a pointer to a search result, non-NULL
 */
void lug_search_destroy(lug_search *s);

/**
 * reads the input of the fine character by character and returns a list with all the team battles
 * @param g a pointer to an undirected graph, non-NULL
 * @param n point to int
 * @param vertices pointer to a gmap
 * @param total pointer to an int
 * @param names_holder pointer to an array to hold the names of the teams
 * @return the number of vertices in that graph
 */
char** read_input(FILE *stream, size_t *n, gmap **vertices, size_t *total, char*** names_holder);

/**
 * checks for cycles
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
bool cycle(lugraph *g);

/**
 * checks for wrong way edges
 * @param ordered a pointer to a malloced array
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
int wrong_way(lugraph *g, int* ordered);

/**
 * sorts dfs hueristic
 * @param ordered a pointer to a malloced array
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
void dfs(lugraph *g, int* ordered);

/**
 * for adding degrees to a struct
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
void lugraph_add_degrees(lugraph *g);

/**
 * retrieves outdegress
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
size_t* lugraph_outdegrees(lugraph *g);

/**
 * retrieves indegrees
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
size_t* lugraph_indegrees(lugraph *g);

/**
 * retrieves ratios
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
float* lugraph_ratios(lugraph *g);

/**
 * sorts with the degree hueristic
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
int* order_degrees(lugraph *g);

/**
 * sorts with the topological hueristic
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
int *topological(lugraph *g);

#endif
