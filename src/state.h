#ifndef STATE_H
#define STATE_H

#include "graph.h"

#define MAX_FLAGS    32
#define MAX_KEYS     16
#define MAX_STATS    8

typedef struct {
    char key[MAX_NAME_LEN];
    int  value;
} StatEntry;

typedef struct {
    /* Current position */
    int current_node;

    /* Inventory */
    char keys[MAX_KEYS][MAX_NAME_LEN];
    int  key_count;

    /* Boolean flags */
    char flags[MAX_FLAGS][MAX_NAME_LEN];
    int  flag_count;

    /* Numeric stats */
    StatEntry stats[MAX_STATS];
    int       stat_count;

    /* Knowledge: which nodes have been visited */
    bool visited[MAX_NODES];

    /* Knowledge: which links have been revealed */
    bool link_revealed[MAX_NODES][MAX_LINKS];
} GameState;

void  state_init(GameState *s, int start_node);
void  state_add_key(GameState *s, const char *key);
bool  state_has_key(GameState *s, const char *key);
void  state_set_flag(GameState *s, const char *flag);
bool  state_has_flag(GameState *s, const char *flag);
void  state_set_stat(GameState *s, const char *name, int value);
int   state_get_stat(GameState *s, const char *name);
bool  state_check_gate(GameState *s, const GateCondition *gate);

#endif
