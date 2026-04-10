#include "state.h"
#include <string.h>

void state_init(GameState *s, int start_node)
{
    memset(s, 0, sizeof(*s));
    s->current_node = start_node;
    s->visited[start_node] = true;
}

void state_add_key(GameState *s, const char *key)
{
    if (s->key_count >= MAX_KEYS) return;
    if (state_has_key(s, key)) return;
    strncpy(s->keys[s->key_count++], key, MAX_NAME_LEN - 1);
}

bool state_has_key(GameState *s, const char *key)
{
    for (int i = 0; i < s->key_count; i++) {
        if (strcmp(s->keys[i], key) == 0) return true;
    }
    return false;
}

void state_set_flag(GameState *s, const char *flag)
{
    if (state_has_flag(s, flag)) return;
    if (s->flag_count >= MAX_FLAGS) return;
    strncpy(s->flags[s->flag_count++], flag, MAX_NAME_LEN - 1);
}

bool state_has_flag(GameState *s, const char *flag)
{
    for (int i = 0; i < s->flag_count; i++) {
        if (strcmp(s->flags[i], flag) == 0) return true;
    }
    return false;
}

void state_set_stat(GameState *s, const char *name, int value)
{
    for (int i = 0; i < s->stat_count; i++) {
        if (strcmp(s->stats[i].key, name) == 0) {
            s->stats[i].value = value;
            return;
        }
    }
    if (s->stat_count >= MAX_STATS) return;
    strncpy(s->stats[s->stat_count].key, name, MAX_NAME_LEN - 1);
    s->stats[s->stat_count].value = value;
    s->stat_count++;
}

int state_get_stat(GameState *s, const char *name)
{
    for (int i = 0; i < s->stat_count; i++) {
        if (strcmp(s->stats[i].key, name) == 0)
            return s->stats[i].value;
    }
    return 0;
}

bool state_check_gate(GameState *s, const GateCondition *gate)
{
    switch (gate->type) {
    case GATE_NONE: return true;
    case GATE_KEY:  return state_has_key(s, gate->key);
    case GATE_FLAG: return state_has_flag(s, gate->key);
    case GATE_STAT: return state_get_stat(s, gate->key) >= gate->value;
    }
    return false;
}
