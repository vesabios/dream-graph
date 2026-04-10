#include "gen.h"
#include <string.h>

/* ---------- PRNG ---------- */

uint64_t gen_rand(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

int gen_rand_range(uint64_t *state, int min, int max)
{
    if (min >= max) return min;
    uint64_t r = gen_rand(state);
    return min + (int)(r % (uint64_t)(max - min + 1));
}

/* ---------- Name tables ---------- */

static const char *NODE_NAMES[] = {
    "Hollow of Echoes",    "Celestial Meadow",    "Withered Sanctum",
    "Lantern Grotto",      "Obsidian Threshold",  "Murmuring Well",
    "Verdant Cradle",      "Shattered Reliquary", "Duskfall Basin",
    "Amber Crossing",      "Frostbitten Aerie",   "Crimson Antechamber",
    "Twilight Spire",      "Sunken Archive",      "Petrified Garden",
    "Wanderer's Rest",     "Ashen Corridor",      "Starlit Cistern",
    "Iron Vestibule",      "Forgotten Alcove"
};
#define NUM_NODE_NAMES (sizeof(NODE_NAMES) / sizeof(NODE_NAMES[0]))

static const char *VAGUE_DESCS[] = {
    "a dim path",          "a faint trail",       "a narrow passage",
    "a winding route",     "a shadowed opening",  "a half-seen door",
    "a crumbling arch",    "a mossy stairway",    "a rusted gate",
    "a whispered direction"
};
#define NUM_VAGUE_DESCS (sizeof(VAGUE_DESCS) / sizeof(VAGUE_DESCS[0]))

static const char *KEY_NAMES[] = {
    "iron key", "crystal shard", "rusted token", "bone whistle"
};
#define NUM_KEY_NAMES (sizeof(KEY_NAMES) / sizeof(KEY_NAMES[0]))

static const char *POOL_NAMES[] = {
    "Upper Reaches", "Deep Hollows", "Eastern Drift", "Western Shade"
};

/* ---------- helpers ---------- */

static void shuffle_ints(uint64_t *rng, int *arr, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = gen_rand_range(rng, 0, i);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

/* Ensure the graph is connected by walking from node 0 and adding
   edges to any unreachable nodes. */
static void ensure_connected(Graph *g, uint64_t *rng)
{
    bool reached[MAX_NODES] = {false};
    int  stack[MAX_NODES];
    int  sp = 0;

    reached[0] = true;
    stack[sp++] = 0;

    while (sp > 0) {
        int cur = stack[--sp];
        Node *n = &g->nodes[cur];
        for (int i = 0; i < n->link_count; i++) {
            int t = n->links[i].target;
            if (!reached[t]) {
                reached[t] = true;
                stack[sp++] = t;
            }
        }
    }

    for (int i = 0; i < g->node_count; i++) {
        if (!reached[i]) {
            /* connect to a random reached node */
            int target;
            do {
                target = gen_rand_range(rng, 0, g->node_count - 1);
            } while (!reached[target]);

            const char *vd = VAGUE_DESCS[gen_rand_range(rng, 0, NUM_VAGUE_DESCS - 1)];
            graph_add_link(g, i, target, vd, g->nodes[target].name);
            graph_add_link(g, target, i, vd, g->nodes[i].name);
            reached[i] = true;
        }
    }
}

/* ---------- main generation ---------- */

void graph_generate(Graph *g, uint64_t seed)
{
    graph_init(g);
    g->seed = seed;

    uint64_t rng = seed ? seed : 1;

    /* decide node count: 8-20 */
    g->node_count = gen_rand_range(&rng, 8, 20);

    /* shuffle name indices so each run gets different names */
    int name_idx[NUM_NODE_NAMES];
    for (int i = 0; i < (int)NUM_NODE_NAMES; i++) name_idx[i] = i;
    shuffle_ints(&rng, name_idx, (int)NUM_NODE_NAMES);

    /* create nodes */
    for (int i = 0; i < g->node_count; i++) {
        Node *n = &g->nodes[i];
        n->id = i;
        strncpy(n->name, NODE_NAMES[name_idx[i % NUM_NODE_NAMES]],
                MAX_NAME_LEN - 1);
        n->importance = gen_rand_range(&rng, 1, 10);
        n->visit_count = 0;
        n->collapsed = false;
        n->link_count = 0;
        n->pool_id = -1;
        n->role = NODE_NORMAL;
    }

    /* assign roles */
    /* node 0 is always a hub (start) */
    g->nodes[0].role = NODE_HUB;
    g->nodes[0].importance = 10;

    /* pick 1-2 key nodes */
    int num_keys = gen_rand_range(&rng, 1, 2);
    for (int k = 0; k < num_keys && k < g->node_count - 2; k++) {
        int ki = gen_rand_range(&rng, 1, g->node_count - 1);
        g->nodes[ki].role = NODE_KEY;
    }

    /* pick 1-2 gate nodes */
    int num_gates = gen_rand_range(&rng, 1, 2);
    for (int k = 0; k < num_gates; k++) {
        int gi = gen_rand_range(&rng, 1, g->node_count - 1);
        if (g->nodes[gi].role == NODE_NORMAL) {
            g->nodes[gi].role = NODE_GATE;
        }
    }

    /* mark some flavor nodes */
    for (int i = 1; i < g->node_count; i++) {
        if (g->nodes[i].role == NODE_NORMAL &&
            g->nodes[i].importance <= 3) {
            g->nodes[i].role = NODE_FLAVOR;
        }
    }

    /* generate links: each node gets 2-4 links to random other nodes */
    for (int i = 0; i < g->node_count; i++) {
        int target_links = gen_rand_range(&rng, 2, MAX_LINKS);
        for (int l = 0; l < target_links; l++) {
            int target = gen_rand_range(&rng, 0, g->node_count - 1);
            if (target == i) continue;

            /* avoid duplicate links */
            bool dup = false;
            for (int e = 0; e < g->nodes[i].link_count; e++) {
                if (g->nodes[i].links[e].target == target) {
                    dup = true;
                    break;
                }
            }
            if (dup) continue;

            const char *vd = VAGUE_DESCS[gen_rand_range(&rng, 0,
                                          NUM_VAGUE_DESCS - 1)];
            graph_add_link(g, i, target, vd, g->nodes[target].name);
        }
    }

    ensure_connected(g, &rng);

    /* set gate conditions on links leading to gate nodes */
    for (int i = 0; i < g->node_count; i++) {
        Node *n = &g->nodes[i];
        for (int l = 0; l < n->link_count; l++) {
            Node *tn = graph_get_node(g, n->links[l].target);
            if (tn && tn->role == NODE_GATE) {
                n->links[l].gate.type = GATE_KEY;
                strncpy(n->links[l].gate.key,
                        KEY_NAMES[gen_rand_range(&rng, 0,
                                  (int)NUM_KEY_NAMES - 1)],
                        MAX_NAME_LEN - 1);
            }
        }
    }

    /* assign pools if enough nodes */
    if (g->node_count >= 6) {
        g->pool_count = gen_rand_range(&rng, 2, MAX_POOLS);
        if (g->pool_count > g->node_count / 3)
            g->pool_count = g->node_count / 3;

        for (int p = 0; p < g->pool_count; p++) {
            Pool *pool = &g->pools[p];
            pool->id = p;
            strncpy(pool->name, POOL_NAMES[p], MAX_NAME_LEN - 1);
            pool->node_count = 0;
        }

        for (int i = 0; i < g->node_count; i++) {
            int p = gen_rand_range(&rng, 0, g->pool_count - 1);
            g->nodes[i].pool_id = p;
            Pool *pool = &g->pools[p];
            if (pool->node_count < MAX_NODES) {
                pool->node_ids[pool->node_count++] = i;
            }
        }
    }
}
