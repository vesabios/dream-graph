#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NODES       20
#define MAX_LINKS       4
#define MAX_POOLS       4
#define MAX_NAME_LEN    64
#define MAX_DESC_LEN    128
#define MAX_GATE_FLAGS  8

typedef enum {
    GATE_NONE,
    GATE_KEY,
    GATE_FLAG,
    GATE_STAT
} GateType;

typedef struct {
    GateType type;
    char     key[MAX_NAME_LEN];
    int      value;
} GateCondition;

typedef enum {
    NODE_NORMAL,
    NODE_HUB,
    NODE_GATE,
    NODE_KEY,
    NODE_FLAVOR
} NodeRole;

typedef struct {
    int  target;
    char vague_desc[MAX_DESC_LEN];
    char revealed_desc[MAX_DESC_LEN];
    bool revealed;
    GateCondition gate;
} Link;

typedef struct {
    int      id;
    char     name[MAX_NAME_LEN];
    NodeRole role;
    int      importance;
    int      visit_count;
    bool     collapsed;
    int      pool_id;
    Link     links[MAX_LINKS];
    int      link_count;
} Node;

typedef struct {
    int  id;
    char name[MAX_NAME_LEN];
    int  node_ids[MAX_NODES];
    int  node_count;
} Pool;

typedef struct {
    Node     nodes[MAX_NODES];
    int      node_count;
    Pool     pools[MAX_POOLS];
    int      pool_count;
    uint64_t seed;
} Graph;

void graph_init(Graph *g);
void graph_destroy(Graph *g);
Node *graph_get_node(Graph *g, int id);
Link *graph_get_links(Graph *g, int node_id, int *out_count);
void graph_add_link(Graph *g, int from, int to,
                    const char *vague, const char *revealed);

#endif
