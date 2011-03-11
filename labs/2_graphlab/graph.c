#include <stdlib.h>
#include <string.h>
#include "graph.h"

/* implement the functions you declare in graph.h here */

vertex_t *graph;

void add_edge (vertex_t **vtxhead, char *v1_name, char *v2_name, int weight) {

  //find if head exists
  int i;
  vertex_t head;
  //get length of graph
  int len = sizeof(graph)/sizeof(vertex_t);
 
  for (i = 0 i<len; i++) {
    if (vertex[i] == vtxhead)
      head = vertex[i];
  }

  if (head == null)
    vertex[len] = malloc
}
