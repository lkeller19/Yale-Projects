#define _GNU_SOURCE

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "lugraph.h"
#include "gmap.h"
#include "string_key.h"
#include "mergesort.h"

enum state {START, INSIDE, COMMA, SECONDINSIDE, NEWLINE};

struct edge
{
  int vertex;
  float ratio;
};

struct degree_sort
{
  size_t vertex;
  float ratio;
  size_t outdegree;
};

struct topo_sort
{
  size_t vertex;
  size_t ratio;
  size_t outdegree;
};

struct dfs_struct
{
  int vertex;
  int outdegree;
  int indegree;
};

struct lugraph
{
  size_t n;          // the number of vertices
  size_t *list_size; // the size of each adjacency list
  size_t *list_cap;  // the capacity of each adjacency list
  size_t **adj;      // the adjacency lists
  // NEED TO PUT INDEGREE AND OUTDEGREE IN HERE
  // keep track of adj[inedges] of all the things that go into it
  // building up the adjency set and only add to list if count >0
  gmap *vertices;
  size_t *outdegrees;
  size_t *indegrees;
  float *ratios;
  char** names;
  // size_t **in_adg;
  // gmap **adjset;
};

struct lug_search
{
  const lugraph *g;
  size_t from;
  int *color;
  size_t *pred;
  size_t *visited;
  size_t visit_count;
};

enum {DFS_UNSEEN, DFS_ACTIVE, DFS_DONE};

#define LUGRAPH_ADJ_LIST_INITIAL_CAPACITY 4

int compare(const void *p1, const void *p2);

char *grow_word(char *word, size_t *end, size_t *capacity, const int c);

void lugraph_add_degrees(lugraph *g);

void error(lugraph *g);

void lu_dfs_visit(lugraph* g, lug_search *s, size_t from);

lug_search *ludfs(lugraph *g, size_t from);

int topo_compare(const void *p1, const void *p2);

int* order_array(lugraph *g, int *arr);

void topo_help(lugraph* g, int* color, int* ordered, int* size);

int *topological(lugraph *g);



/**
 * Resizes the adjacency list for the given vertex in the given graph.
 * 
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 */
void lugraph_list_embiggen(lugraph *g, size_t from);

gmap* indegreee(lugraph *g, char** names, int* color);

int dfs_comp(const void *p1, const void *p2);

/**
 * Visits the given vertex in the given search of the given graph.
 *
 * @param g a pointer to a directed graph
 * @param s a search in that graph
 * @param from a vertex in that graph
 * @param sort a function used to sort the neighbors of that vertex, or NULL
 */
void lugraph_dfs_visit(const lugraph* g, lug_search *s, size_t from, int* cycle);

/**
 * Prepares a search result for the given graph starting from the given
 * vertex.  It is the responsibility of the caller to destroy the result.
 *
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 * @return a pointer to a search result
 */
lug_search *lug_search_create(const lugraph *g, size_t from);

/**
 * Adds the to vertex to the from vertex's adjacency list.
 *
 * @param g a pointer to a graph, non-NULL
 * @param from the index of a vertex in g
 * @param to the index of a vertex in g
 */
void lugraph_add_half_edge(lugraph *g, size_t from, size_t to);

void print_adj_list(lugraph *g, char** names) {
  for (size_t i = 0; i < g->n; i++) {
    fprintf(stderr, "%s: ", names[i]);
    for (size_t j = 0; j < g->list_size[i]; j++) {
      fprintf(stderr, "%ld ", g->adj[i][j]);
    }
    fprintf(stderr, "\n");
  }
}

void error(lugraph *g) {
  fprintf(stderr, "%ld %ld\n", g->n, g->list_cap[0]);

}

void lugraph_add_degrees(lugraph *g) {
  for (size_t i = 0; i < g->n; i++) {
    g->outdegrees[i] = g->list_size[i];
  }
  for (size_t i = 0; i < g->n; i++) {
    for (size_t j = 0; j < g->list_size[i]; j++) {
      g->indegrees[g->adj[i][j]]++;

    }
  }
  for (size_t i = 0; i < g->n; i++) {
    if (g->indegrees[i] == 0) {
      if (g->outdegrees[i] == 0) {
        g->ratios[i] = 0;
      }
      else {
        g->ratios[i] = 1.0;
      }
    }
    else {
      g->ratios[i] = ((float)g->outdegrees[i]) / (float)(g->indegrees[i] + g->outdegrees[i]);
      fprintf(stderr, "%f\n", g->ratios[i]);
    }
  }
}

size_t* lugraph_outdegrees(lugraph *g) {
  return g->outdegrees;
}

size_t* lugraph_indegrees(lugraph *g) {
  return g->indegrees;
}

float* lugraph_ratios(lugraph *g) {
  return g->ratios;
}

int degree_compare(const void *p1, const void *p2)
{
  const degree_sort *q1 = p1;
  const degree_sort *q2 = p2;
  if (q1->ratio > q2->ratio)
    {
      return -1;
    }
  else if (q1->ratio < q2->ratio)
    {
      return 1;
    }
  else if (q1->outdegree > q2->outdegree) {
    return -1;
  }
  else if (q1->outdegree < q2->outdegree) {
    return 1;
  }
  else if (q1->vertex > q2->vertex) {
    return 1;
  }
  else if (q1->vertex < q2->vertex) {
    return -1;
  }
  else {
    return 0;
  }
}

int* order_degrees(lugraph *g) {
  degree_sort *comp = malloc(sizeof(degree_sort)*g->n);
  for (int i = 0; i < g->n; i++) {
      comp[i].vertex = i;
      comp[i].ratio = g->ratios[i];
      comp[i].outdegree = g->outdegrees[i];
  }
  degree_sort *out = malloc(sizeof(degree_sort)*g->n);
  merge_sort(g->n, sizeof(degree_sort), comp, out, degree_compare);
  int *ordered_vertices = malloc(sizeof(int)*g->n);

  for (int i = 0; i < g->n; i++) {
    ordered_vertices[i] = out[i].vertex;
  }
  free(comp);
  free(out);
  return ordered_vertices;
}



lugraph *lugraph_create(size_t n, gmap *vertices, char** names)
{
  if (n < 1)
    {
      return NULL;
    }
  
  lugraph *g = malloc(sizeof(lugraph));
  if (g != NULL)
    {
      g->n = n;
      g->list_size = malloc(sizeof(size_t) * n);
      g->list_cap = malloc(sizeof(size_t) * n);
      g->adj = malloc(sizeof(size_t *) * n);
      g->outdegrees = malloc(sizeof(size_t) * n);
      g->indegrees = malloc(sizeof(size_t) * n);
      g->ratios = malloc(sizeof(float) * n);
      g->vertices = vertices;
      g->names = names;
      // g->adjset = adjset;
      
      if (g->list_size == NULL || g->list_cap == NULL || g->adj == NULL || g->vertices == NULL || g->outdegrees == NULL || g->indegrees == NULL || g->ratios == NULL)
      {
        free(g->list_size);
        free(g->list_cap);
        free(g->adj);
        free(g->vertices);
        free(g->outdegrees);
        free(g->indegrees);
        free(g->ratios);
        free(g);

        return NULL;
      }

      for (size_t i = 0; i < n; i++)
      {
        g->list_size[i] = 0;
        g->adj[i] = malloc(sizeof(size_t) * LUGRAPH_ADJ_LIST_INITIAL_CAPACITY);
        g->list_cap[i] = g->adj[i] != NULL ? LUGRAPH_ADJ_LIST_INITIAL_CAPACITY : 0;
        g->outdegrees[i] = 0;
        g->indegrees[i] = 0;
        g->ratios[i] = 0;
      }
    }

  return g;
}

size_t lugraph_size(const lugraph *g)
{
  if (g != NULL)
    {
      return g->n;
    }
  else
    {
      return 0;
    }
}

void lugraph_list_embiggen(lugraph *g, size_t from)
{
  if (g->list_cap[from] != 0)
    {
      size_t *bigger = realloc(g->adj[from], sizeof(size_t) * g->list_cap[from] * 2);
      if (bigger != NULL)
      {
        g->adj[from] = bigger;
        g->list_cap[from] = g->list_cap[from] * 2;
      }
    }
}

void lugraph_add_edge(lugraph *g, size_t from, size_t to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      lugraph_add_half_edge(g, from, to);
      // lugraph_add_half_edge(g, to, from);
    }
}

void lugraph_add_half_edge(lugraph *g, size_t from, size_t to)
{
  if (g->list_size[from] == g->list_cap[from])
    {
      lugraph_list_embiggen(g, from);
    }
  
  if (g->list_size[from] < g->list_cap[from])
    {
      g->adj[from][g->list_size[from]++] = to;
      //fprintf(stderr, "%ld\n", g->adj[from][g->list_size[from] - 1]);
    }
}

bool lugraph_has_edge(const lugraph *g, size_t from, size_t to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      size_t i = 0;
      while (i < g->list_size[from] && g->adj[from][i] != to)
	{
	  i++;
	}
      return i < g->list_size[from];
    }
  else
    {
      return false;
    }
}

size_t lugraph_degree(const lugraph *g, size_t v)
{
  if (g != NULL && v >= 0 && v < g->n)
    {
      return g->list_size[v];
    }
  else
    {
      return -1;
    }
}

lug_search *lugraph_dfs(const lugraph *g, size_t from, int *cycle)
{
  if (g == NULL || from < 0 || from >= g->n)
    {
      return NULL;
    }
  // TO DO: initialize search results and start recursion
  lug_search *s = lug_search_create(g, from);
  lugraph_dfs_visit(g, s, from, cycle);
  return s;
}

// index* not_cleared(lugraph *g, size_t from) {
//   int check = 0;
//   for (int i = 0; i < g->list_size[from]; i++) {
//     if (color[i] == DFS_UNSEEN) {
//       return &i;
//     }
//   }
//   return NULL;
  
// }

// int dfs_compare(const void *p1, const void *p2)
// {
//   const dfs_struct *q1 = p1;
//   const dfs_struct *q2 = p2;

//   if (q1->outdegree < q2->outdegree)
//     {
//       return -1;
//     }
//   else if (q1->outdegree > q2->outdegree)
//     {
//       return 1;
//     }
//   else if(q1->indegree < q2->indegree) {
//       return -1;
//     }
//   else if(q1->indegree > q2->indegree) {
//       return 1;
//     }

//   else if(q1->vertex > q2->vertex) {
//     return -1;
//   }
//   else if(q1->vertex < q2->vertex) {
//     return 1;
//   }
//   else {
//     return 0;
//   }
// }

int* next_dfs(lugraph *g, size_t from) {
  //dfs_struct *input = malloc(sizeof(edge)*(g->list_size[from]));
  dfs_struct input[g->list_size[from]];

  for (int i = 0; i < g->list_size[from]; i++) {
    input[i].vertex = g->adj[from][i];
    input[i].outdegree = g->outdegrees[g->adj[from][i]];
    input[i].indegree = g->indegrees[g->adj[from][i]];
  }

  dfs_struct out[g->list_size[from]];
  merge_sort(g->list_size[from], sizeof(dfs_struct), &input, &out, dfs_comp);
  int *ordered_vertices = malloc(sizeof(int)*g->list_size[from]);
  for (int i = 0; i < g->list_size[from]; i++) {
    ordered_vertices[i] = out[i].vertex;
  }
  return ordered_vertices;
}

// int* nexter(lugraph *g, size_t from, char** names) {
//   //dfs_struct *input = malloc(sizeof(edge)*(g->list_size[from]));
//   edge *input = malloc(sizeof(edge)*(g->list_size[from]));
//   int* color = calloc(g->n, sizeof(int));
//   gmap *indegrees = indegreee(g, names, color);

//   for (int i = 0; i < g->list_size[from]; i++) {
//     input[i].vertex = g->adj[from][i];
//     input[i].ratio = (float)lugraph_degree(g, g->adj[from][i]);
//     // input[i].outdegree = lugraph_degree(g, g->adj[from][i]);
//     // input[i].indegree = *(int*)gmap_get(indegrees, names[g->adj[from][i]]);
//   }

//   edge *out = malloc(sizeof(edge)*g->list_size[from]);
//   merge_sort(g->list_size[from], sizeof(edge), input, out, dfs_compare);
//   int *ordered_vertices = malloc(sizeof(int)*g->list_size[from]);
//   for (int i = 0; i < g->list_size[from]; i++) {
//     ordered_vertices[i] = out[g->list_size[from] - 1 - i].vertex;
//   }
//   return ordered_vertices;
// }


// TO DO: implement lugraph_dfs_visit helper function
void lugraph_dfs_visit(const lugraph* g, lug_search *s, size_t from, int *cycle) {
  s->color[from] = DFS_ACTIVE;
  s->visited[s->visit_count++] = from;
  for (size_t i = 0; i < g->list_size[from]; i++) {    
    if (s->color[g->adj[from][i]] == DFS_UNSEEN) {
      s->pred[g->adj[from][i]] = from;
      lugraph_dfs_visit(g, s, g->adj[from][i], cycle);
    }
    else if (s->color[g->adj[from][i]] == DFS_ACTIVE) {
      *cycle = 1;
    }
  }
  s->color[from] = DFS_DONE;
}

bool lugraph_connected(const lugraph *g, size_t from, size_t to, int* cycle)
{
  if (g == NULL || from < 0 || from >= g->n || to < 0 || to >= g->n)
    {
      return false;
    }
  
  lug_search *dfs = lugraph_dfs(g, from, cycle);
  if (dfs == NULL)
    {
      return false;
    }
  else
    {
      bool result = dfs->color[to] == DFS_DONE;
      lug_search_destroy(dfs);
      return result;
    }
}

void lugraph_destroy(lugraph *g)
{
  if (g != NULL)
    {
      for (size_t i = 0; i < g->n; i++)
	{
	  free(g->adj[i]);
	}
      free(g->adj);
      free(g->list_cap);
      free(g->list_size);
      free(g->outdegrees);
      free(g->indegrees);
      free(g->ratios);
      free(g);
    }
}

lug_search *lug_search_create(const lugraph *g, size_t from)
{
  if (g != NULL && from >= 0 && from < g->n)
    {
      lug_search *s = malloc(sizeof(lug_search));
      
      if (s != NULL)
	{
	  s->g = g;
	  s->from = from;
	  s->color = malloc(sizeof(int) * g->n);
	  s->visited = malloc(sizeof(size_t) * g->n);
	  s->visit_count = 0;
	  s->pred = malloc(sizeof(size_t) * g->n);

	  if (s->color != NULL && s->visited != NULL && s->pred != NULL)
	    {
	      for (size_t i = 0; i < g->n; i++)
		{
		  s->color[i] = DFS_UNSEEN;
		  s->pred[i] = g->n;
		}
	    }
	  else
	    {
	      free(s->pred);
	      free(s->visited);
	      free(s->color);
	      free(s);
	      return NULL;
	    }
	}

      return s;
    }
  else
    {
      return NULL;
    }
}

size_t *lug_search_visited(const lug_search *s, size_t *len)
{
  if (s == NULL && len != NULL)
    {
      return NULL;
    }

  size_t *visited = malloc(sizeof(size_t) * s->visit_count);
  if (visited != NULL)
    {
      *len = s->visit_count;
      memcpy(visited, s->visited, sizeof(size_t) * s->visit_count);
    }
  
  return visited;
}

void lug_search_destroy(lug_search *s)
{
  if (s != NULL)
    {
      free(s->color);
      free(s->visited);
      free(s->pred);
      free(s);
    }
}

char** read_input(FILE *stream, size_t *n, gmap **vertices, size_t *total, char*** names_holder)
{
  size_t game_capacity = 10;
  char **games = malloc(sizeof(char*)*game_capacity);
  size_t capacity = 2;
  char **names = malloc(sizeof(char*)*capacity);
  enum state curr = START;
  size_t leadingspace = 1;
  size_t tailingspace = 0;
  size_t commafound = 0;
  size_t charpos = 0;
  size_t size = 30;
  size_t size2 = 30;
  size_t index = 0;
  char *winner = malloc(size*sizeof(char)*size);
  char *loser = malloc(size*sizeof(char)*size2);
  char c;
  size_t lines = 0;
  size_t exist1 = 0;
  size_t exist2 = 0;
  while ((c = getc(stream)) != EOF) {
      switch (curr) {
          case START:
              if (c != '"') {
                for (size_t i = 0; i < *n; i++) {
                  free(names[i]);
                }
                free(names);
                for (size_t i = 0; i < *total; i++) {
                  free(games[i]);
                }
                free(games);
                free(winner);
                free(loser);
                  return NULL;
              }
              else {
                  lines++;
                  curr = INSIDE;
                  break;
              }
          case INSIDE:
              if (c != '"') {
                  if (c == ' ' && leadingspace == 1) {
                      for (size_t i = 0; i < *n; i++) {
                        free(names[i]);
                      }
                      free(names);
                      for (size_t i = 0; i < *total; i++) {
                        free(games[i]);
                      }
                      free(games);
                      free(winner);
                      free(loser);
                      return NULL;
                  }
                  else if (c == ' ') {
                      if (charpos+2 > size) {
                          size *=2;
                          winner = realloc(winner, size*sizeof(char));
                      }
                      winner[charpos++] = c;
                      winner[charpos] = '\0';
                      tailingspace = 1;
                      break;
                  }
                  if (charpos+2 > size) {
                      size *=2;
                      winner = realloc(winner, size*sizeof(char));
                  }
                  winner[charpos++] = c;
                  winner[charpos] = '\0';
                  // fprintf(stderr, "%s\n",winner);
                  exist1 = 1;
                  tailingspace = 0;
                  leadingspace = 0;
                  break;
              }
              else if(c == '"' && tailingspace == 1) {
                for (size_t i = 0; i < *n; i++) {
                  free(names[i]);
                  }
                  free(names);
                  for (size_t i = 0; i < *total; i++) {
                    free(games[i]);
                  }
                  free(games);
                  free(winner);
                  free(loser);
                  return NULL;
              }
              else {
                if (exist1 == 1) {
                  charpos = 0;
                  curr = COMMA;
                  break;
                }
                else {
                  for (size_t i = 0; i < *n; i++) {
                    free(names[i]);
                  }
                  free(names);
                  for (size_t i = 0; i < *total; i++) {
                    free(games[i]);
                  }
                  free(games);
                  free(winner);
                  free(loser);
                  return NULL;
                }
              }
          case COMMA:
              if (c == ',' && commafound == 0) {
                  commafound = 1;
                  break;
              }
              if (c == '"') {
                  leadingspace = 1;
                  tailingspace = 0;
                  curr = SECONDINSIDE;
                  break;
              }
              else {
                for (size_t i = 0; i < *n; i++) {
                  free(names[i]);
                }
                free(names);
                for (size_t i = 0; i < *total; i++) {
                  free(games[i]);
                }
                free(games);
                free(winner);
                free(loser);
                return NULL;
              }
          case SECONDINSIDE:
              if (c != '"') {
                  if (c == ' ' && leadingspace == 1) {
                      for (size_t i = 0; i < *n; i++) {
                        free(names[i]);
                      }
                      free(names);
                      for (size_t i = 0; i < *total; i++) {
                        free(games[i]);
                      }
                      free(games);
                      free(winner);
                      free(loser);
                      return NULL;
                  }
                  else if (c == ' ') {
                      if (charpos+2 > size2) {
                          size2 *=2;
                          loser = realloc(loser, size2*sizeof(char));
                      }
                      loser[charpos++] = c;
                      loser[charpos] = '\0';
                      tailingspace = 1;
                      break;
                  }
                  if (charpos+2 > size2) {
                          size2 *=2;
                          loser = realloc(loser, size2*sizeof(char));
                      }
                  loser[charpos++] = c;
                  loser[charpos] = '\0';
                  // fprintf(stderr, "%s\n",loser);
                  exist2 = 1;
                  tailingspace = 0;
                  leadingspace = 0;
                  break;
              }
              else if(c == '"' && tailingspace == 1) {
                for (size_t i = 0; i < *n; i++) {
                  free(names[i]);
                  }
                  free(names);
                  for (size_t i = 0; i < *total; i++) {
                    free(games[i]);
                  }
                  free(games);
                  free(winner);
                  free(loser);
                  return NULL;
              }
              else {
                if (exist2 == 1) {
                  curr = NEWLINE;
                    break;
                }
                else {
                  for (size_t i = 0; i < *n; i++) {
                    free(names[i]);
                  }
                  free(names);
                  for (size_t i = 0; i < *total; i++) {
                    free(games[i]);
                  }
                  free(games);
                  free(winner);
                  free(loser);
                  return NULL;
                }
                    
              }
          case NEWLINE:
          // need to error check
              if (c == '\n') {
                  if (!gmap_contains_key(*vertices, winner)) {
                      size_t *ptr = malloc(sizeof(size_t));
                      *ptr = index;
                      gmap_put(*vertices, winner, ptr);
                      index++;
                      (*n)++;
                      if (*n > capacity) {
                        names = realloc(names, sizeof(char*)*capacity*2);
                        capacity *=2;
                      }
                      names[*n-1] = duplicate(winner);

                  }
                  if (!gmap_contains_key(*vertices,loser)) {
                      size_t *ptr = malloc(sizeof(size_t));
                      *ptr = index;
                      gmap_put(*vertices, loser, ptr);
                      index++;
                      (*n)++; 
                      if (*n > capacity) {
                        names = realloc(names, sizeof(char*)*capacity*2);
                        capacity *=2;
                      }
                      names[*n - 1] = duplicate(loser);

                  }
                  //fprintf(stderr, "winner: %s, loser: %s\n", winner, loser);
                  // lugraph_add_edge(g, (size_t)gmap_get(g->vertices, winner), (size_t)gmap_get(g->vertices, loser));
                  if (*total + 2 > game_capacity) {
                    games = realloc(games, sizeof(char*)*game_capacity*2);
                    game_capacity*=2;
                  }
                  games[(*total)++] = duplicate(winner);
                  games[(*total)++] = duplicate(loser);
                  charpos = 0;
                  leadingspace = 1;
                  tailingspace = 0;
                  commafound = 0;
                  curr = START;
                  // lines++;
                  break;
              }
              else if (c == ' '){
                  break;
              }
              else {
                for (size_t i = 0; i < *n; i++) {
                  free(names[i]);
                }
                free(names);
                for (size_t i = 0; i < *total; i++) {
                  free(games[i]);
                }
                free(games);
                free(winner);
                free(loser);
                return NULL;
                     
              }
      }
  }
  if (*total == 0) {
    free(games);
    free(names);
    free(winner);
    free(loser);
    return NULL;
  }
  // if (lines * 2 != *total) {
  //   for (size_t i = 0; i < *n; i++) {
  //     free(names[i]);
  //   }
  //   for (size_t i = 0; i < *total; i++) {
  //     free(games[i]);
  //   }
  //   free(games);
  //   free(names);
  //   free(winner);
  //   free(loser);
  //   return NULL;
  // }


  // not gonna work because theyupdate at the same time
  // if (*total != lines * 2) {
  //   for (size_t i = 0; i < *n; i++) {
  //     free(names[i]);
  //   }
  //   free(names);
  //   for (size_t i = 0; i < *total; i++) {
  //     free(games[i]);
  //   }
  //   free(games);
  //   free(winner);
  //   free(loser);
  //   return NULL;
  // }
  *names_holder = names;
  // if (*total < game_capacity) {
  //   char** new = realloc(games, (*total)*sizeof(char*));
  //   if (new != NULL) {
  //     games = new;
  //   }
  // }
  // if (*n < capacity) {
  //   char ** new = realloc(names, (*n)*sizeof(char*));
  //   if (new != NULL) {
  //     names = new;
  //   }
  // }
  // free(names);
  free(winner);
  free(loser);
  return games;
}

// gmap* indegreee(lugraph *g, char** names, int* color) {
//   gmap *holder = gmap_create(duplicate, compare_keys, hash29, free);
//   int count = 0;

//   for (int i = 0; i < g->n; i++) {
//     if (color[i] == 0) {
//       edge *arr = malloc(sizeof(edge));
//       arr->vertex = i;
//       arr->ratio = 0;
//       gmap_put(holder, names[i], arr);
//       count++;
//     }
//   }
  
//   edge *reciever;
//   //edge *t;
//   size_t e;
//   for (size_t i = 0; i < g->n; i++) {
//     if (color[i] == 0) {
//       for (size_t j = 0; j < g->list_size[i]; j++) {
//         if (color[i] == 0) {
//           e = g->adj[i][j];
//           //fprintf(stderr, "%ld\n", e);
//           reciever = gmap_get(holder, names[e]);
//           reciever->ratio++;   
//         }   
//       }
//     }
    
//   }
//   return holder;
// }

// gmap* degree(lugraph *g, char** names) {
//   gmap *holder = gmap_create(duplicate, compare_keys, hash29, free);
//   for (int i = 0; i < g->n; i++) {
//     // changed from int!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//     size_t *arr = malloc(sizeof(size_t)*2);
//     arr[0] = g->list_size[i];
//     arr[1] = g->list_size[i];
//     gmap_put(holder, names[i], arr);
//   }

  
//   size_t *reciever;
//   size_t edge;
//   for (size_t i = 0; i < g->n; i++) {
//     for (size_t j = 0; j < g->list_size[i]; j++) {
//       edge = g->adj[i][j];
//       reciever = gmap_get(holder, names[edge]);
//       reciever[1]++;      
//     }
//   }
//   return holder;
// }

bool cycle(lugraph *g) {
  int cycle = 0;
  lug_search *dfs = lugraph_dfs(g, 0, &cycle);
  for (int i = 0; i < g->n; i++) {
      if (cycle == 1) {
        lug_search_destroy(dfs);
        return true;
      }
      if (dfs->color[i] == DFS_UNSEEN) {
        lugraph_dfs_visit(g, dfs, i, &cycle);
      }
  }
  lug_search_destroy(dfs);
  return false;
}

// int* order(lugraph *g, gmap* holder, char** names) {
//   size_t *r;
//   float *arr = malloc(sizeof(float)*g->n*2);
//   int number = 0;
//   for (int i = 0; i < g->n; i++) {
//       fprintf(stderr, "%s: ", names[i]);
//       r = gmap_get(holder, names[i]);
//       arr[number++] = r[0];
//       arr[number++] = r[1];
//       fprintf(stderr, "(%ld/%ld)\n", r[0], r[1]);
//   }
  
//   number = 0;
//   float ratio[g->n];
//   for (int i = 0; i < g->n; i++) {
//       if (arr[i+1] == 0) {
//           ratio[i] = 0;
//           number +=2;
//       }
//       else {
//           ratio[i] = arr[number] / arr[number+1];
//           number +=2;
//       }
//       fprintf(stderr, "%f ", ratio[i]);
//   }
//   fprintf(stderr, "\n");

//   edge *comp = malloc(sizeof(edge)*g->n);
//   for (int i = 0; i < g->n; i++) {
//       comp[i].vertex = i;
//       comp[i].ratio = ratio[i];
//   }
//   edge *out = malloc(sizeof(edge)*g->n);
//   merge_sort(g->n, sizeof(edge), comp, out, compare);
//   int *ordered_vertices = malloc(sizeof(int)*g->n);

//   for (int i = 0; i < g->n; i++) {
//     ordered_vertices[i] = out[g->n - 1 - i].vertex;
//   }
//   return ordered_vertices;
// }


// int compare(const void *p1, const void *p2)
// {
//   const edge *q1 = p1;
//   const edge *q2 = p2;

//   if (q1->ratio < q2->ratio)
//     {
//       return -1;
//     }
//   else if (q1->ratio > q2->ratio)
//     {
//       return 1;
//     }
//   else
//     {
//       return 0;
//     }
// }

int wrong_way(lugraph *g, int* ordered) {
  gmap* hold = gmap_create(duplicate, compare_keys, hash29, free);
  int wrong_way = 0;
  gmap_put(hold, &ordered[0], NULL);
  for (int i = 1; i < g->n; i++) {
    for (int j = 0; j < g->list_size[ordered[i]]; j++) {
      if (gmap_contains_key(hold, &g->adj[ordered[i]][j])) {
        wrong_way++;
      }
    }
    gmap_put(hold, &ordered[i], NULL);
  }
  gmap_destroy(hold);
  return wrong_way;
}

// void topo_helper(lugraph* g, char** names, int* color, int* ordered, int *size) {
//   int* full_color = malloc(sizeof(int)*g->n);
//   gmap *full = indegreee(g, names, full_color);
//   free(full_color);
//   for (int i = 0; i < g->n; i++) {
//       //fprintf(stderr, "%d, ", color[i]);
//     }
//   //fprintf(stderr, "\n");
  
//   gmap *indegrees = indegreee(g, names, color);
//   //fprintf(stderr, "indegree: %ld\n", gmap_size(indegrees));
  
//   edge *ptr; 
//   int number = 0;
//   for (int i = 0; i < g->n; i++) {
//     if (color[i] == 0) {
//       ptr = (edge*)gmap_get(indegrees, names[i]);
//       //fprintf(stderr, "%s indegree: %d\n", names[i], (int)ptr->ratio);
//       if ((int)ptr->ratio == 0) {
//         number++;
//       }
//     }
//   }

//   char** names_holder = malloc(sizeof(char*)*(gmap_size(indegrees) - number));
//   int count = 0;
//   int oop = 0;
//   gmap* leftover = gmap_create(duplicate, compare_keys, hash29, free);
//   edge *input = malloc(sizeof(edge)*number);
//   for (int i = 0; i < g->n; i++) {
//     if (color[i] == 0) {
//           ptr = (edge*)gmap_get(indegrees, names[i]);
//           if ((int)ptr->ratio == 0) {
//             input[oop] = *ptr;
//             edge *eek = gmap_get(full, names[i]);
//             input[oop++].ratio = eek->ratio;
//             //fprintf(stderr,"%d\n", input[oop - 1].vertex);
//             color[i] = 1;
//           }
//           else {
//             gmap_put(leftover, names[i], ptr);
//             names_holder[count++] = names[i];
//             //fprintf(stderr, "%s ", names_holder[count - 1]);
//           }
//     }
//   }
//   for (int i = 0; i < number; i++) {
//     //fprintf(stderr, "%d\n", input[i].vertex);
//   }
//   edge *out = malloc(sizeof(edge)*number);
//   merge_sort(number, sizeof(edge), input, out, compare);
//   //fprintf(stderr, "input: ddddddd%d\n", out[0].vertex);
//   int *ordered_vertices = malloc(sizeof(int)*number);
//   for (int i = 0; i < number; i++) {
//     ordered_vertices[i] = out[number - 1 - i].vertex;
//   }
//   for (int i = 0; i < number; i++) {
//     ordered[(*size)++] = ordered_vertices[i];
//     //fprintf(stderr, "%d ", ordered[i]);
//   }

//   int check = 0;
//   for (int i = 0; i < g->n; i++) {
//     if (color[i]==0 && check == 0) {
//       check = 1;
//     }
//   }
//   if (check == 1) {
//     topo_helper(g, names, color, ordered, size);
//   }
    
// }

// int* topo(lugraph *g, char** names, int* ordered, int *size) {
//   //fprintf(stderr,"here1\n");
//   // gmap *holder = degree(g, names);
//   // size_t *r;
//   // edge *indegree = malloc(sizeof(edge)*g->n);
//   // gmap* log = gmap_create(duplicate, compare_keys, hash29, free);

//   // for (int i = 0; i < g->n; i++) {
//   //     fprintf(stderr, "%s: ", names[i]);
//   //     r = gmap_get(holder, names[i]);
//   //     indegree[i].vertex = i;
//   //     indegree[i].ratio = r[1] - r[0];
//   //     fprintf(stderr, "indegree: %f\n", indegree[i].ratio);
//   // }
//   int *color = calloc(g->n, sizeof(int));
//   gmap *indegrees = indegreee(g, names, color);
//   edge *ptr; 
//   int number = 0;
//   for (int i = 0; i < g->n; i++) {
    
//     ptr = (edge*)gmap_get(indegrees, names[i]);
//     if (ptr == NULL) {
//       //fprintf(stderr, "%ld\n", gmap_size(indegrees));
//     }
//     //fprintf(stderr, "%s indegree: %d\n", names[i], (int)ptr->ratio);
//     if ((int)ptr->ratio == 0) {
      
//       number++;
//     }
//   }
//   //fprintf(stderr, "number! %d\n", number);

//   char** names_holder = malloc(sizeof(char*)*(g->n - number));
//   int count = 0;
//   int opp = 0;
//   gmap* leftover = gmap_create(duplicate, compare_keys, hash29, free);
//   edge *input = malloc(sizeof(edge)*number);
//   for (int i = 0; i < g->n; i++) {
//     ptr = (edge*)gmap_get(indegrees, names[i]);
//     if ((int)ptr->ratio == 0) {
//       input[opp++] = *ptr;
//       color[i] = 1;
//     }
//     else {
//       gmap_put(leftover, names[i], ptr);
//       names_holder[count++] = names[i];
//       //fprintf(stderr, "%s ", names_holder[count - 1]);
//     }
//   }
  
//   for (int i = 0; i < number; i++) {
//     //fprintf(stderr, "%d\n", input[i].vertex);
//   }
//   edge *out = malloc(sizeof(edge)*number);
//   merge_sort(number, sizeof(edge), input, out, compare);
//   int *ordered_vertices = malloc(sizeof(int)*number);
//   for (int i = 0; i < number; i++) {
//     ordered_vertices[i] = out[g->n - 1 - i].vertex;
//   }
//   for (int i = 0; i < number; i++) {
//     ordered[(*size)++] = ordered_vertices[i];
//     //fprintf(stderr, "%d ", ordered_vertices[i]);
//   }

//   for (int i = 0; i < g->n - number; i++) {
//     //fprintf(stderr, "%s ", names_holder[i]);
//   }
//   //fprintf(stderr, "\n");

//   topo_helper(g, names, color, ordered, size);
//   return ordered_vertices;
// }



// lug_search *lugraph_dfs(const lugraph *g, size_t from, int *cycle)
// {
//   if (g == NULL || from < 0 || from >= g->n)
//     {
//       return NULL;
//     }
//   // TO DO: initialize search results and start recursion
//   lug_search *s = lug_search_create(g, from);
//   lugraph_dfs_visit(g, s, from, cycle);
//   return s;
// }

// // TO DO: implement lugraph_dfs_visit helper function
// void lugraph_dfs_visit(const lugraph* g, lug_search *s, size_t from, int *cycle) {
//   s->color[from] = DFS_ACTIVE;
//   s->visited[s->visit_count++] = from;
//   for (size_t i = 0; i < g->list_size[from]; i++) {    
//     if (s->color[g->adj[from][i]] == DFS_UNSEEN) {
//       s->pred[g->adj[from][i]] = from;
//       for (j = 0; j < g->list_size[from]; j++) {

//       }
//       lugraph_dfs_visit(g, s, g->adj[from][i], cycle);
//     }
//     else if (s->color[g->adj[from][i]] == DFS_ACTIVE) {
//       *cycle = 1;
//     }
//   }
//   s->color[from] = DFS_DONE;
// }

void dfs(lugraph *g, int* ordered) {
  for (int i = 0; i < g->n; i++) {
    lug_search *dfs = ludfs(g, i);
    int arr[g->n];
    for (int j = 0; j < g->n; j++) {
      arr[j] = j;
    }
    int *next = order_array(g, arr);
    for (int j = 0; j < g->n; j++) {
        if (dfs->color[next[j]] == DFS_UNSEEN) {
          lu_dfs_visit(g, dfs, next[j]);
        }
    }
    free(next);
    

    if (i == 0) {
      for (int j = 0; j < g->n; j++) {
        ordered[j] = (int)dfs->visited[j];
        fprintf(stderr, "%d ", ordered[j]);
        
      }
      fprintf(stderr, "\n");
    }
    else {
      int* pos = malloc(sizeof(int)*g->n);
      for (int j = 0; j < g->n; j++) {
        pos[j] = (int)dfs->visited[j];
        fprintf(stderr, "%d ", pos[j]);
      }
      fprintf(stderr, "\n");
      int curr = wrong_way(g, pos);
      int min = wrong_way(g, ordered);
      if (curr < min) {
        //fprintf(stderr, "here");
        for (int j = 0; j < g->n; j++) {
          ordered[j] = (int)dfs->visited[j];
        } 
      }
      free(pos);
    }
    lug_search_destroy(dfs);
    
  }
    
  
}

int dfs_comp(const void *p1, const void *p2)
{
  const dfs_struct *q1 = p1;
  const dfs_struct *q2 = p2;
  if (q1->outdegree > q2->outdegree) {
    return -1;
  }
  else if (q1->outdegree < q2->outdegree) {
    return 1;
  }
  else if (q1->indegree < q2->indegree) {
    return -1;
  }
  else if (q1->indegree > q2->indegree) {
    return 1;
  }
  else if (q1->vertex > q2->vertex) {
    return 1;
  }
  else if (q1->vertex < q2->vertex) {
    return -1;
  }

  else {
    return 0;
  }
}

void lu_dfs_visit(lugraph* g, lug_search *s, size_t from) {
  s->color[from] = DFS_ACTIVE;
  s->visited[s->visit_count++] = from;
  int *next = next_dfs(g, from);
  for (size_t i = 0; i < g->list_size[from]; i++) {    
    if (s->color[next[i]] == DFS_UNSEEN) {
      s->pred[next[i]] = from;
      lu_dfs_visit(g, s, next[i]);
    }
  }
  s->color[from] = DFS_DONE;
  free(next);
}

lug_search *ludfs(lugraph *g, size_t from)
{
  if (g == NULL || from < 0 || from >= g->n)
    {
      return NULL;
    }
  // TO DO: initialize search results and start recursion
  lug_search *s = lug_search_create(g, from);
  lu_dfs_visit(g, s, from);
  return s;
}

int* order_array(lugraph *g, int *arr) {
  //dfs_struct *input = malloc(sizeof(edge)*(g->list_size[from]));
  dfs_struct input[g->n];

  for (int i = 0; i < g->n; i++) {
    input[i].vertex = arr[i];
    input[i].outdegree = g->outdegrees[arr[i]];
    input[i].indegree = g->indegrees[arr[i]];
  }

  dfs_struct out[g->n];
  merge_sort(g->n, sizeof(dfs_struct), &input, &out, dfs_comp);
  int *ordered_vertices = malloc(sizeof(int)*g->n);
  for (int i = 0; i < g->n; i++) {
    ordered_vertices[i] = out[i].vertex;
  }
  return ordered_vertices;
}

// int *updated_topo(lugraph *g, int* indeg, int *ordered, int* size) {
//   int number = 0;
//   if (*size == 0) {
//     for (int i = 0; i < g->n; i++) {
//       if (g->indgrees[i] == 0) {
//         number++;
//       }
//       indeg[i] = g->indegrees[i];
//     }

//     for (int i = 0; i < g->n; i++) {
//       if (g->indegrees[i] == 0) {
//         for (int j = 0; j < g->list_size[i]; j++) {
//           indeg = g->indegrees[g->adj[i][j]] - 1;
//         } 
//       }
//       indeg = g->indegrees[i];
//     }

//     topo_sort input[number];
//     int count = 0;
//     for (int i = 0; i < g->n; i++) {
//       if (g->indegrees[i] == 0) {
//         input[count].vertex = i;
//         input[count].outdegree = g->outdegrees[i];
//         color[i] = 1;
//       }
//     } 
//   } 

  
//   topo_sort out[number];
//   merge_sort(number, sizeof(edge), &input, &out, topo_compare);
//   int *ordered = malloc(sizeof(int)*g->n);
//   if (ordered == NULL) {
//     return NULL;
//   }
//   for (int i = 0; i < number; i++) {
//     ordered[(*size)++] = out[i].vertex;
//   }
// }

int *topological(lugraph *g) {
  int *color = calloc(g->n, sizeof(int));
  if (color == NULL) {
    return NULL;
  }
  int number = 0;
  for (int i = 0; i < g->n; i++) {
    if (g->indegrees[i] == 0) {
      number++;
    }
  }
  // look at the outvertices of what you have then remove one from each of the indegrees
  //fprintf(stderr, "number: %d\n", number);
  // for the ones you are subtracting, just look at those to see if they are 0. 
  topo_sort input[number];
  int count = 0;
  for (int i = 0; i < g->n; i++) {
    if (g->indegrees[i] == 0) {
      input[count].vertex = i;
      input[count].outdegree = g->outdegrees[i];
      color[i] = 1;
      count++;
    }
  }
  topo_sort out[number];
  int* size = malloc(sizeof(int));
  if (size == NULL) {
    free(color);
    return NULL;
  }
  *size = 0;
  merge_sort(number, sizeof(topo_sort), &input, &out, topo_compare);
  int *ordered = malloc(sizeof(int)*g->n);
  if (ordered == NULL) {
    free(color);
    free(size);
    return NULL;
  }
  for (int i = 0; i < number; i++) {
    ordered[*size] = out[i].vertex;
    *size = *size + 1;
  }
  topo_help(g, color, ordered, size);
  free(color);
  free(size);
  return ordered;
}

void topo_help(lugraph* g, int* color, int* ordered, int* size) {
  int indegree[g->n];
  for (int i = 0; i < g->n; i++) {
    indegree[i] = 0;
  }
  for (int i = 0; i < g->n; i++) {
    if (color[i] == 0) {
      for (int j = 0; j < g->list_size[i]; j++) {
        if (color[g->adj[i][j]] == 0) {
          indegree[g->adj[i][j]]++;
        }
      }
    }
    else {
      indegree[i] = -1;
    }
  }

  int number = 0;
  for (int i = 0; i < g->n; i++) {
    if (indegree[i] == 0) {
      number++;
    }
  }
  topo_sort input[number];
  int count = 0;
  for (int i = 0; i < g->n; i++) {
    if (indegree[i] == 0) {
      input[count].vertex = i;
      input[count].outdegree = g->outdegrees[i];
      color[i] = 1;
      count++;
    }
  }
  topo_sort out[number];
  merge_sort(number, sizeof(topo_sort), &input, &out, topo_compare);
  for (int i = 0; i < number; i++) {
    ordered[*size] = out[i].vertex;
    *size = *size + 1;
  }

  int check = 0;
  //fprintf(stderr, "Color: ");
  for (int i = 0; i < g->n; i++) {
    if (color[i]==0 && check == 0) {
      check = 1;
    }
    //fprintf(stderr, "%d ", color[i]);
  }
  //fprintf(stderr, "\n");
  if (check == 1) {
    topo_help(g, color, ordered, size);
  }
  
}

int topo_compare(const void *p1, const void *p2)
{
  const topo_sort *q1 = p1;
  const topo_sort *q2 = p2;
  if (q1->outdegree > q2->outdegree) {
    return -1;
  }
  else if (q1->outdegree < q2->outdegree) {
    return 1;
  }
  else if (q1->vertex > q2->vertex) {
    return 1;
  }
  else if (q1->vertex < q2->vertex) {
    return -1;
  }
  else {
    return 0;
  }
}