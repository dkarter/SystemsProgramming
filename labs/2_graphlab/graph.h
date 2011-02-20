/* forward declarations so self-referential structures are simpler */
typedef struct vertex vertex_t;
typedef struct adj_vertex adj_vertex_t;
typedef struct tour_info tour_info_t;

struct vertex {
	char *name;
	adj_vertex_t *adj_list;
	vertex_t *next;
};

struct adj_vertex {
	vertex_t *vertex;
	int edge_weight;
	adj_vertex_t *next;	
};

struct tour_info {
    vertex_t *path;
    int total_distance;
};
/* This is the one function you really should implement as part of your 
 * graph data structure's public API. 
 *
 * `add_edge` adds the specified edge to the graph passed in via the first
 * argument. If either of the edge's vertices are not already in the graph,
 * they are added before their adjacency lists are updated. If the graph
 * is currently empty (i.e., *vtxhead == NULL), a new graph is created,
 * and the caller's vtxhead pointer is modified. 
 *
 * `vtxhead`: the pointer to the graph (more specifically, the head of the
 *            list of vertex_t structures)
 * `v1_name`: the name of the first vertex of the edge to add
 * `v2_name`: the name of the second vertex of the edge to add
 * `weight` : the weight of the edge to add
 */
 void add_edge (vertex_t **vtxhead, char *v1_name, char *v2_name, int weight);

 void print_out (vertex_t *head);

 vertex_t **graph_last (vertex_t **vtxhead);

 vertex_t *graph_contains (vertex_t *vtxhead, char *name);

 vertex_t *vtx_insert(vertex_t **head, char *name);
 vertex_t *vtx_insert_obj(vertex_t **head, vertex_t *child);
 void adj_insert(vertex_t *parent, vertex_t *child, int weight);

 int graph_size(vertex_t *head);
 int adjlist_size(adj_vertex_t *head);

 void free_adj(adj_vertex_t *target);
 void free_vtx(vertex_t *target);
void freemem(vertex_t **target);

 adj_vertex_t **adjlist_last(adj_vertex_t **head);
 vertex_t *subtract_list(vertex_t *first, vertex_t *second);

 adj_vertex_t *adjlist_contains (adj_vertex_t *head, char *name);
 tour_info_t *find_tour(vertex_t *head);
 int tour_recursive(vertex_t *head, int max_graph_size, vertex_t **path, int dist);
 int find_weight(vertex_t *head, char *v1_name, char* v2_name);
