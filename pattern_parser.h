#pragma once

#include <stdbool.h>

#define MAX_PATTERN 64
#define MAX_PATTERN_CHAR_VAL 100
#define MAX_CHILDREN 20

#define SIZE_OF_WURD 64

enum pattern_token_type
{
    SQUARE_BRACKET_LEFT,
    SQUARE_BRACKET_RIGHT,
    CURLY_BRACKET_LEFT,
    CURLY_BRACKET_RIGHT,
    ANGLE_BRACKET_LEFT,
    ANGLE_BRACKET_RIGHT,
    BLANK,
    VAR_NAME
} pattern_token_type;

typedef struct pattern_token
{
    unsigned int type;
    char value[MAX_PATTERN_CHAR_VAL];
    int idx;
    bool has_multiplier;
    int multiplier;
    bool has_divider;
    int divider;
    bool has_euclid;
    int euclid_hits;
    int euclid_steps;
} pattern_token;

typedef struct pg_child
{
    bool new_level;
    int level_idx;
} pg_child;

typedef struct pattern_group
{
    int num_children;
    pg_child children[MAX_CHILDREN];
    int parent;
} pattern_group;

bool parse_pattern(char *line);
bool is_valid_pattern(char *line);
void work_out_positions(pattern_group pgroups[MAX_PATTERN], int level,
                        int start_idx, int pattern_len,
                        int ppositions[MAX_PATTERN], int *numpositions);
int extract_tokens_from_line(pattern_token *tokens, int *token_idx,
                                      char *wurd);
void parse_tokens_into_groups(pattern_token tokens[MAX_PATTERN],
                              int num_tokens);
