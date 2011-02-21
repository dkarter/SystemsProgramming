#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"

/* implement the functions you declare in graph.h here */


void add_edge (vertex_t **vtxhead, char *v1_name, char *v2_name, int weight) {
    //if head pointer points to nothing then we allocate a new graph
    if (vtxhead == NULL)
        vtxhead = malloc(sizeof(vertex_t));

    vertex_t *parent = vtx_insert(vtxhead, v1_name);
    vertex_t *child = vtx_insert(vtxhead, v2_name);
    
    adj_insert(parent, child, weight);
    adj_insert(child, parent, weight);
}

adj_vertex_t *adjlist_contains (adj_vertex_t *head, char *name) {
    adj_vertex_t *cursor;

    for (cursor = head; cursor != NULL; cursor = cursor->next) {
      if (cursor->vertex != NULL && !strcmp(cursor->vertex->name, name))
            return cursor;
    }

    return NULL;
}

adj_vertex_t **adjlist_last(adj_vertex_t **head) {
    if (*head == NULL)
        return head;

    adj_vertex_t **cursor = head;

    while ((*cursor) != NULL)
        cursor = &(*cursor)->next;

    return cursor;
}

//returns the last element in the list or vtxhead itself if list == empty
vertex_t **graph_last(vertex_t **vtxhead) {
    if (*vtxhead == NULL)
        return vtxhead;

    vertex_t **cursor = vtxhead;

    while ((*cursor) != NULL)
        cursor = &(*cursor)->next;

    return cursor;
}

//searches for a vertex in a list and returns it or null in !exists
vertex_t *graph_contains (vertex_t *vtxhead, char *name) {
    vertex_t *cursor;

    for (cursor = vtxhead; cursor != NULL; cursor = cursor->next) {
      if (!strcmp(cursor->name,name))
            return cursor;
    }

    return NULL;
}

//checks if an element exists in the linked list and adds it if not
//returns the new/existing item
vertex_t *vtx_insert(vertex_t **head, char *name) {
    vertex_t *exists = graph_contains(*head, name);

    //avoid further actions if already exists
    if (exists != NULL)
        return exists;

    //we couldn't find it then let's add it
    vertex_t **last = graph_last(head);

    *last = malloc(sizeof(vertex_t));
    (*last)->name = name;

    //return newly created item
    return *last;
}

vertex_t *vtx_insert_obj(vertex_t **head, vertex_t *child) {
    vertex_t *vtx = vtx_insert(head, child->name);
    vtx->adj_list = child->adj_list;
    return vtx;
}

//inserts an item into adj list if doesn't exist already
void adj_insert(vertex_t *parent, vertex_t *child, int weight) {
    adj_vertex_t *adj_exists;
    adj_vertex_t **last_adj;

    //parent has the child in adjancy list
    adj_exists = adjlist_contains(parent->adj_list, child->name);

    if (adj_exists == NULL) {
        last_adj = adjlist_last(&(parent->adj_list));
        *last_adj = malloc(sizeof(adj_vertex_t));

        //set properties
        (*last_adj)->edge_weight = weight;
        (*last_adj)->vertex = child;
    }
}

// needs to dealloc after use
tour_info_t *find_tour(vertex_t *head) {
  vertex_t *path = NULL;
  tour_info_t *result = malloc(sizeof(tour_info_t));
  
  result->total_distance = tour_recursive(head, graph_size(head), &path, 0);
  
  if (path != NULL) {
    result->path = path;
  } else {
    result = NULL;
  }

  //return result (need to free mem afterwards as well)
  return result;
}

/*
   Finds a tour of the graph, given as an adjacency list. When found,
   return a vector where the first element is a seq of the vertices
   in the order they're traversed, and the second element is the distance
   of the tour. The tour returned isn't necessarily the shortest one.
*/
int tour_recursive(vertex_t *head, int max_graph_size, vertex_t **path, int dist) {
     
   if (head != NULL && graph_contains(*path, head->name) == NULL)
      //track the path we've toured
      vtx_insert(path, head->name);
   else
     //head is empty so we can't really do anything
     return 0;


    //check if done touring
    if(graph_size(*path) == max_graph_size)
        return dist;

    vertex_t *subtraction = subtract_list(head, *path);

    //already toured
    if (subtraction == NULL) {
        return 0;
    }
        

    adj_vertex_t *cursor;
    int a;
    int d;

    for (cursor = head->adj_list; cursor != NULL; cursor = cursor->next) {
      //call recursive touring on each adj vertex (also add up distance)
      d = dist+cursor->edge_weight;
      if ((a = tour_recursive(cursor->vertex, max_graph_size, path, d)))
	return a;
    }

    return 0;
}


 int find_weight(vertex_t *head, char *v1_name, char* v2_name) {
   vertex_t *first = graph_contains(head, v1_name);
   adj_vertex_t *second = adjlist_contains(first->adj_list, v2_name);
   return second->edge_weight;
 }

 //returns every item in the first list that's not in the second
 vertex_t *subtract_list(vertex_t *first, vertex_t *second) {
     vertex_t *newlist = NULL;
     vertex_t *cursor;
     for (cursor = first; cursor != NULL; cursor = cursor->next) {
         if (graph_contains(second, cursor->name) == NULL)
             vtx_insert_obj(&newlist, cursor);
     }
     
     return newlist;
 }

//gets the size of a vertex linked list
int graph_size(vertex_t *head) {
    
    int size = 0;
    vertex_t *cursor;

    for (cursor = head; cursor != NULL; cursor = cursor->next)
        size++;

    return size;
}

//gets the size of a vertex linked list
int adjlist_size(adj_vertex_t *head) {

    int size = 0;
    adj_vertex_t *cursor;

    for (cursor = head; cursor != NULL; cursor = cursor->next)
        size++;

    return size;
}

//prints the graph using iteration
void print_out (vertex_t *head, vertex_t *tour_path, int distance) {
  /* print out our adjacency list */
  printf("Adjacency list:\n");

  if (head == NULL) {
    printf("Empty.\n");
    return;
  }

  vertex_t *vp;
  adj_vertex_t *adj_v;

  for (vp = head; vp != NULL; vp = vp->next) {
    printf("  %s: ", vp->name);
    
    //iterate over adjacency list
    for (adj_v = vp->adj_list; adj_v != NULL; adj_v = adj_v->next) {
      printf("%s(%d) ", adj_v->vertex->name, adj_v->edge_weight);
    }

    printf("\n");
  }

  /* print out our tour path list */
  printf("\nTour path:\n  ");

  vertex_t *cursor;

  for (cursor = tour_path; cursor != NULL; cursor = cursor->next)
    printf("%s ", cursor->name);
  
  printf("\n");

  /* print out our adjacency list */
  printf("\nTour length: %d\n", distance);


}

void free_adj(adj_vertex_t *target) {
    if (target != NULL) {
        free_adj(target->next);
        target->edge_weight = 0;
        target->vertex = NULL;
        target->next = NULL;
        target = NULL;
    }
}

void free_vtx(vertex_t *target) {
    if (target != NULL) {
        free_vtx(target->next);
        target->name = NULL;
        free_adj(target->adj_list);
        target->adj_list = NULL;
        target->next = NULL;
        target = NULL;
    }
}

void freemem(vertex_t **target) {
    free_vtx(*target);
    target = NULL;
}
