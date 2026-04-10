#include "graph.h"
#include <string.h>

void graph_init(Graph *g)
{
    memset(g, 0, sizeof(*g));
}

void graph_destroy(Graph *g)
{
    /* All storage is inline, nothing to free */
    (void)g;
}

Node *graph_get_node(Graph *g, int id)
{
    if (id < 0 || id >= g->node_count) return NULL;
    return &g->nodes[id];
}

Link *graph_get_links(Graph *g, int node_id, int *out_count)
{
    Node *n = graph_get_node(g, node_id);
    if (!n) {
        *out_count = 0;
        return NULL;
    }
    *out_count = n->link_count;
    return n->links;
}

void graph_add_link(Graph *g, int from, int to,
                    const char *vague, const char *revealed)
{
    Node *n = graph_get_node(g, from);
    if (!n || n->link_count >= MAX_LINKS) return;

    Link *l = &n->links[n->link_count++];
    l->target = to;
    l->revealed = false;
    l->gate.type = GATE_NONE;
    strncpy(l->vague_desc, vague, MAX_DESC_LEN - 1);
    strncpy(l->revealed_desc, revealed, MAX_DESC_LEN - 1);
}
