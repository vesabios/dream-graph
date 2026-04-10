#include "traverse.h"
#include <string.h>

int traverse_get_visible(Graph *g, GameState *s,
                         VisibleLink *out, int max_out)
{
    Node *n = graph_get_node(g, s->current_node);
    if (!n) return 0;

    int count = 0;
    for (int i = 0; i < n->link_count && count < max_out; i++) {
        Link *l = &n->links[i];
        Node *target = graph_get_node(g, l->target);
        if (!target || target->collapsed) continue;

        VisibleLink *vl = &out[count++];
        vl->link_index = i;
        vl->target_node = l->target;
        vl->passable = state_check_gate(s, &l->gate);

        if (l->revealed || s->link_revealed[n->id][i]) {
            strncpy(vl->display, l->revealed_desc, MAX_DESC_LEN - 1);
        } else {
            strncpy(vl->display, l->vague_desc, MAX_DESC_LEN - 1);
        }
    }
    return count;
}

bool traverse_move(Graph *g, GameState *s, int link_index)
{
    Node *n = graph_get_node(g, s->current_node);
    if (!n || link_index < 0 || link_index >= n->link_count)
        return false;

    Link *l = &n->links[link_index];

    if (!state_check_gate(s, &l->gate))
        return false;

    int from = s->current_node;
    int to = l->target;

    /* reveal the link we just used */
    traverse_reveal(g, s, from, link_index);

    /* move */
    s->current_node = to;
    s->visited[to] = true;

    Node *dest = graph_get_node(g, to);
    if (dest) dest->visit_count++;

    /* if destination is a key node, pick up the key */
    if (dest && dest->role == NODE_KEY) {
        state_add_key(s, dest->name);
    }

    return true;
}

void traverse_reveal(Graph *g, GameState *s, int node_id, int link_index)
{
    Node *n = graph_get_node(g, node_id);
    if (!n || link_index < 0 || link_index >= n->link_count) return;

    n->links[link_index].revealed = true;
    s->link_revealed[node_id][link_index] = true;
}

void traverse_collapse(Graph *g, GameState *s)
{
    for (int i = 0; i < g->node_count; i++) {
        Node *n = &g->nodes[i];
        if (n->collapsed) continue;
        if (n->role == NODE_HUB || n->role == NODE_GATE) continue;
        if (n->importance >= 5) continue;

        if (n->visit_count >= COLLAPSE_THRESHOLD) {
            n->collapsed = true;

            /* rewire: for each node that linked to this collapsed node,
               redirect links to this node's targets instead */
            for (int j = 0; j < g->node_count; j++) {
                if (j == i || g->nodes[j].collapsed) continue;
                Node *other = &g->nodes[j];

                for (int l = 0; l < other->link_count; l++) {
                    if (other->links[l].target == i) {
                        /* pick the first non-collapsed target of the
                           collapsed node */
                        for (int k = 0; k < n->link_count; k++) {
                            Node *redirect = graph_get_node(g,
                                              n->links[k].target);
                            if (redirect && !redirect->collapsed &&
                                n->links[k].target != j) {
                                other->links[l].target =
                                    n->links[k].target;
                                strncpy(other->links[l].revealed_desc,
                                        redirect->name,
                                        MAX_DESC_LEN - 1);
                                /* keep revealed state */
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}
