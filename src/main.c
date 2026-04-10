#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "graph.h"
#include "gen.h"
#include "state.h"
#include "traverse.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

/* ---------- shared engine state ---------- */

static Graph     g_graph;
static GameState g_state;
static bool      g_running;

/* ---------- WASM-exported API ---------- */

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void engine_init(uint32_t seed)
{
    graph_generate(&g_graph, seed ? seed : (uint64_t)time(NULL));
    state_init(&g_state, 0);
    g_running = true;
}

EMSCRIPTEN_KEEPALIVE
int engine_current_node(void)
{
    return g_state.current_node;
}

EMSCRIPTEN_KEEPALIVE
const char *engine_node_name(int id)
{
    Node *n = graph_get_node(&g_graph, id);
    return n ? n->name : "???";
}

EMSCRIPTEN_KEEPALIVE
int engine_node_role(int id)
{
    Node *n = graph_get_node(&g_graph, id);
    return n ? (int)n->role : 0;
}

EMSCRIPTEN_KEEPALIVE
int engine_get_links(void)
{
    /* returns count; JS reads link data via engine_link_* */
    static VisibleLink links[MAX_VISIBLE_LINKS];
    return traverse_get_visible(&g_graph, &g_state,
                                links, MAX_VISIBLE_LINKS);
}

static VisibleLink s_visible[MAX_VISIBLE_LINKS];
static int         s_visible_count;

EMSCRIPTEN_KEEPALIVE
int engine_refresh_links(void)
{
    s_visible_count = traverse_get_visible(&g_graph, &g_state,
                                           s_visible, MAX_VISIBLE_LINKS);
    return s_visible_count;
}

EMSCRIPTEN_KEEPALIVE
const char *engine_link_display(int idx)
{
    if (idx < 0 || idx >= s_visible_count) return "";
    return s_visible[idx].display;
}

EMSCRIPTEN_KEEPALIVE
int engine_link_passable(int idx)
{
    if (idx < 0 || idx >= s_visible_count) return 0;
    return s_visible[idx].passable ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int engine_move(int visible_idx)
{
    if (visible_idx < 0 || visible_idx >= s_visible_count) return 0;
    int li = s_visible[visible_idx].link_index;
    bool ok = traverse_move(&g_graph, &g_state, li);
    if (ok) traverse_collapse(&g_graph, &g_state);
    return ok ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int engine_key_count(void)
{
    return g_state.key_count;
}

EMSCRIPTEN_KEEPALIVE
const char *engine_key_name(int idx)
{
    if (idx < 0 || idx >= g_state.key_count) return "";
    return g_state.keys[idx];
}

EMSCRIPTEN_KEEPALIVE
int engine_node_count(void)
{
    return g_graph.node_count;
}

EMSCRIPTEN_KEEPALIVE
int engine_visited(int id)
{
    if (id < 0 || id >= g_graph.node_count) return 0;
    return g_state.visited[id] ? 1 : 0;
}

#else /* ---------- native terminal UI ---------- */

static void print_location(void)
{
    Node *n = graph_get_node(&g_graph, g_state.current_node);
    if (!n) return;

    printf("\n=== %s ===\n", n->name);

    const char *role_str = "";
    switch (n->role) {
    case NODE_HUB:    role_str = " [Hub]";    break;
    case NODE_GATE:   role_str = " [Gate]";   break;
    case NODE_KEY:    role_str = " [Key]";    break;
    case NODE_FLAVOR: role_str = " [Flavor]"; break;
    default: break;
    }
    printf("  Role: %s%s\n", n->name, role_str);

    if (n->pool_id >= 0 && n->pool_id < g_graph.pool_count) {
        printf("  Region: %s\n", g_graph.pools[n->pool_id].name);
    }

    if (g_state.key_count > 0) {
        printf("  Keys: ");
        for (int i = 0; i < g_state.key_count; i++) {
            printf("[%s] ", g_state.keys[i]);
        }
        printf("\n");
    }
}

static void print_links(VisibleLink *links, int count)
{
    printf("\nPaths:\n");
    for (int i = 0; i < count; i++) {
        const char *lock = links[i].passable ? "" : " [LOCKED]";
        printf("  %d) %s%s\n", i + 1, links[i].display, lock);
    }
}

int main(int argc, char **argv)
{
    uint64_t seed = (uint64_t)time(NULL);
    if (argc > 1) {
        seed = (uint64_t)strtoull(argv[1], NULL, 10);
    }

    printf("dream-graph engine\n");
    printf("Seed: %llu\n", (unsigned long long)seed);

    graph_generate(&g_graph, seed);
    state_init(&g_state, 0);
    g_running = true;

    printf("Generated %d nodes across %d regions.\n",
           g_graph.node_count, g_graph.pool_count);

    while (g_running) {
        print_location();

        VisibleLink visible[MAX_VISIBLE_LINKS];
        int count = traverse_get_visible(&g_graph, &g_state,
                                         visible, MAX_VISIBLE_LINKS);

        if (count == 0) {
            printf("\nNo paths forward. You are stranded.\n");
            break;
        }

        print_links(visible, count);
        printf("\nChoose (1-%d, 0 to quit): ", count);

        char buf[16];
        if (!fgets(buf, sizeof(buf), stdin)) break;

        int choice = atoi(buf);
        if (choice == 0) {
            printf("Farewell.\n");
            break;
        }

        if (choice < 1 || choice > count) {
            printf("Invalid choice.\n");
            continue;
        }

        int li = visible[choice - 1].link_index;
        if (!traverse_move(&g_graph, &g_state, li)) {
            printf("The way is blocked.\n");
        } else {
            traverse_collapse(&g_graph, &g_state);
        }
    }

    graph_destroy(&g_graph);
    return 0;
}

#endif
