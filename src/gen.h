#ifndef GEN_H
#define GEN_H

#include "graph.h"

/* Seeded xorshift64 PRNG */
uint64_t gen_rand(uint64_t *state);
int      gen_rand_range(uint64_t *state, int min, int max);

/* Generate a complete graph from a seed */
void graph_generate(Graph *g, uint64_t seed);

#endif
