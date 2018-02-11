#pragma once

#include <stdbool.h>

void build_euclidean_pattern_short(int level, uint16_t *bitmap_int,
                                   uint16_t *bitmap_len, int *count,
                                   int *remainderrr);
uint16_t create_euclidean_rhythm(int num_hits, int num_steps);
