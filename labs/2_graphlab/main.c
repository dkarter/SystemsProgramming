#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

int main (int argc, char *argv[]) {
	/* 
	 * Programatically constructing the following simple graph:
	 *
	 *     10         5
	 * A <-----> B <-----> C
	 *
	 * Delete this code and add your own!
	 */
	/*vertex_t *v1, *v2, *v3, *vlist_head;
	adj_vertex_t *adj_v;*/

        vertex_t *vlist_head = NULL;
        add_edge(&vlist_head, "A", "B", 10);
        add_edge(&vlist_head, "B", "C", 5);
        

        tour_info_t *tour = find_tour(vlist_head);

        print_out(vlist_head);

        printf("Tour:\n");
        print_out(tour->path);

        printf("Total Distance: %d\n", tour->total_distance);

        freemem(&vlist_head);

        print_out(vlist_head);

        return 0;


        /*
        //graph size test
        printf("graph size (expected 3): %d\n", graph_size(vlist_head));
        printf("graph size - NULL (expected 0): %d\n", graph_size(NULL));
        
        //graph last test
        vertex_t **a = graph_last(&vlist_head);
        printf("graph last (expected C): %s\n", (*a)->name);
        vertex_t *null_list = NULL;
        a = graph_last(&null_list);
        printf("graph last - NULL list head pointer (expected OK): %s\n", ((*a)==null_list? "OK" : "Error"));
        null_list = malloc(sizeof(vertex_t));
        a = graph_last(&null_list);
        printf("graph last - malloc'd  (expected OK): %s\n", ((*a)==null_list? "OK" : "Error"));

        //graph_contains
        char *B = "B";
        vertex_t *b = graph_contains(vlist_head, B);
        printf("graph contains B (expected Yes): %s\n", ((b->name == B)? "Yes" : "No"));
        b = graph_contains(vlist_head, "D");
        printf("graph contains D (expected No): %s\n", ((b == NULL)? "No" : "Yes"));
        b = graph_contains(NULL, B);
        printf("graph contains B - empty graph (expected NULL): %s\n", ((b == NULL)? "NULL" : "Error"));


        //add edge
        */

        //dealloc all
        
}
