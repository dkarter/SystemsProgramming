#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

int main (int argc, char *argv[]) {
  vertex_t *vlist_head = NULL; 
 
  int i;
  //parse command line into a graph object
  for (i = 1; i<argc; i=i+3)
    add_edge(&vlist_head, argv[i], argv[i+1], atoi(argv[i+2]));
  

  
  tour_info_t *tour = find_tour(vlist_head);

  print_out(vlist_head, tour->path, tour->total_distance);

  freemem(&vlist_head);
  freemem(&(tour->path));

  vlist_head = NULL;
  tour = NULL;
  
  return 0;
        
}
