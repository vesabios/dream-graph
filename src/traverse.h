#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "graph.h"
#include "state.h"

#define MAX_VISIBLE_LINKS 3
#define COLLAPSE_THRESHOLD 3

typedef struct {
    int  link_index;
    int  target_node;
    bool passable;
    char display[MAX_DESC_LEN];
} VisibleLink;

/* Get the links visible from the current node (filtered, max MAX_VISIBLE_LINKS) */
int traverse_get_visible(Graph *g, GameState *s,
                         VisibleLink *out, int max_out);

/* Move the player through a link. Returns true if successful. */
bool traverse_move(Graph *g, GameState *s, int link_index);

/* Reveal a link's destination after traversal */
void traverse_reveal(Graph *g, GameState *s, int node_id, int link_index);

/* Check and collapse low-importance nodes */
void traverse_collapse(Graph *g, GameState *s);

#endif
