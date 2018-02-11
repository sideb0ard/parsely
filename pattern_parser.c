#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "pattern_parser.h"
#define PPQN 960         // Pulses Per Quarter Note // one beat
#define PPBAR (PPQN * 4) // Pulses per loop/bar - i.e 4 * beats

#define _STACK_SIZE 32

// algo inspired from
// https://www.geeksforgeeks.org/check-for-balanced-parentheses-in-an-expression/

static char *token_type_names[] = {"SQUARE_LEFT", "SQUARE_RIGHT", "CURLY_LEFT",
                                   "CURLY_RIGHT", "ANGLE_LEFT",   "ANGLE_RIGHT",
                                   "BLANK",       "VAR_NAME"};

static bool is_in_array(int num_to_look_for, int *nums, int nums_len)
{
    for (int i = 0; i < nums_len; i++)
        if (nums[i] == num_to_look_for)
            return true;
    return false;
}

static void print_pattern_tokens(pattern_token tokens[MAX_PATTERN], int len)
{
    for (int i = 0; i < len; i++)
    {
        if (tokens[i].type == VAR_NAME)
            printf("%s", tokens[i].value);
        else
            printf("%s", token_type_names[tokens[i].type]);
        if (i < (len - 1))
            printf(" ");
        else
            printf("\n");
    }
}

void parse_tokens_into_groups(pattern_token tokens[MAX_PATTERN], int num_tokens)
{
    pattern_group pgroups[MAX_PATTERN] = {0};
    int current_pattern_group = 0;
    int num_pattern_groups = 0;

    pattern_token var_tokens[100] = {0};
    int var_tokens_idx = 0;

    for (int i = 0; i < num_tokens; i++)
    {
        if (tokens[i].type == SQUARE_BRACKET_LEFT)
        {
            pgroups[++num_pattern_groups].parent = current_pattern_group;
            int cur_child = pgroups[current_pattern_group].num_children++;
            pgroups[current_pattern_group].children[cur_child].level_idx =
                num_pattern_groups;
            pgroups[current_pattern_group].children[cur_child].new_level = true;
            current_pattern_group = num_pattern_groups;
        }
        else if (tokens[i].type == SQUARE_BRACKET_RIGHT)
            current_pattern_group = pgroups[current_pattern_group].parent;
        else if (tokens[i].type == BLANK || tokens[i].type == VAR_NAME)
        {
            pgroups[current_pattern_group].num_children++;
            var_tokens[var_tokens_idx++] = tokens[i];
        }
    }

    printf("Num Groups:%d\n", num_pattern_groups);
    for (int i = 0; i <= num_pattern_groups; i++)
        printf("Group %d - parent is %d contains %d members\n", i,
               pgroups[i].parent, pgroups[i].num_children);

    int level = 0;
    int start_idx = 0;
    int pattern_len = PPBAR;
    int ppositions[MAX_PATTERN] = {0};
    int numpositions = 0;
    work_out_positions(pgroups, level, start_idx, pattern_len, ppositions,
                       &numpositions);

    int num_uniq = 0;
    int uniq_positions[MAX_PATTERN] = {0};
    for (int i = 0; i < numpositions; i++)
        if (!is_in_array(ppositions[i], uniq_positions, num_uniq))
            uniq_positions[num_uniq++] = ppositions[i];

    if (num_uniq != var_tokens_idx)
    {
        printf("Vars and timings don't match, ya numpty: num_uniq:%d "
               "var_tokens:%d\n",
               num_uniq, var_tokens_idx);
        return;
    }

    //// 3. verify env vars
    // for (int i = 0; i < var_tokens_idx; i++)
    //{
    //    char *var_key = var_tokens[i].value;
    //    if (mixer_is_valid_env_var(mixr, var_key))
    //    {
    //        printf("Valid ENV VAR! %s\n", var_key);
    //        int sg_num;
    //        if (get_environment_val(var_key, &sg_num))
    //        {
    //            printf("CLEARING %s(%d)\n", var_key, sg_num);
    //            sample_sequencer *seq =
    //                (sample_sequencer *)mixr->sound_generators[sg_num];
    //            seq_clear_pattern(&seq->m_seq, 0);
    //        }
    //    }
    //    else if (var_tokens[i].type == BLANK)
    //    {
    //    } // no-op
    //    else
    //    {
    //        printf("NAE Valid ENV VAR! %s\n", var_key);
    //        return;
    //    }
    //}

    // for (int i = 0; i < num_uniq; i++)
    //{
    //    if (var_tokens[i].type != BLANK)
    //    {
    //        int sg_num;
    //        get_environment_val(var_tokens[i].value, &sg_num);
    //        printf("Play at %s %d\n", var_tokens[i].value, uniq_positions[i]);
    //        sample_sequencer *seq =
    //            (sample_sequencer *)mixr->sound_generators[sg_num];
    //        seq_add_micro_hit(&seq->m_seq, 0, uniq_positions[i]);
    //    }
    //}
}

static bool isvalidpatchar(char c)
{
    if (isalnum(c)
        || c == '/'
        || c == '*'
        || c == '('
        || c == ')'
        || c == ',')
        return true;
    return false;
}

int extract_tokens_from_line(pattern_token *tokens, int *token_idx, char *line)
{

    char *c = line;
    while (*c)
    {
        char var_name[100] = {0};
        int var_name_idx = 0;

        if (*c == ' ')
        {
            c++;
        }
        else if (*c == '[')
        {
            printf("SQ_LEFTBRACKET!\n");
            tokens[(*token_idx)++].type = SQUARE_BRACKET_LEFT;
            c++;
        }
        else if (*c == ']')
        {
            printf("SQ_RIGHTBRACKET!\n");
            tokens[(*token_idx)++].type = SQUARE_BRACKET_RIGHT;
            c++;
        }
        else if (*c == '{')
        {
            printf("CURLY_LEFT!\n");
            tokens[(*token_idx)++].type = CURLY_BRACKET_LEFT;
            c++;
        }
        else if (*c == '}')
        {
            printf("CURLY_RIGHT!\n");
            tokens[(*token_idx)++].type = CURLY_BRACKET_RIGHT;
            c++;
        }
        else if (*c == '<')
        {
            printf("ANGLE LEFT!\n");
            tokens[(*token_idx)++].type = ANGLE_BRACKET_LEFT;
            c++;
        }
        else if (*c == '>')
        {
            printf("ANGLE RIGHT!\n");
            tokens[(*token_idx)++].type = ANGLE_BRACKET_RIGHT;
            c++;
        }
        else if ((*c == '_') || (*c == '-') || (*c == '~'))
        {
            printf("BLANK!\n");
            tokens[(*token_idx)++].type = BLANK;
            c++;
        }
        else
        {
            while (isvalidpatchar(*c))
                var_name[var_name_idx++] = *c++;
            printf("VAR! %s\n", var_name);
            tokens[(*token_idx)].type = VAR_NAME;
            strncpy(tokens[*token_idx].value, var_name, MAX_PATTERN_CHAR_VAL);
            (*token_idx)++;
        }
    }

    return 0;
}

void work_out_positions(pattern_group pgroups[MAX_PATTERN], int level,
                        int start_idx, int pattern_len,
                        int ppositions[MAX_PATTERN], int *numpositions)
{

    // printf("Looking at Level:%d start_idx:%d pattern_len: %d\n", level,
    // start_idx, pattern_len);
    int num_children = pgroups[level].num_children;
    if (num_children != 0)
    {
        int incr = pattern_len / num_children;
        for (int i = 0; i < num_children; i++)
        {
            int child = pgroups[level].children[i].level_idx;
            int chidx = (i * incr) + start_idx;
            // printf("CHILD:%d plays at pos%d\n", child, chidx);
            ppositions[(*numpositions)++] = chidx;
            if (pgroups[level].children[i].new_level)
            {
                // printf("NEW LEVEL!\n");
                work_out_positions(pgroups, child, chidx, incr, ppositions,
                                   numpositions);
            }
        }
    }
}

static bool matchy_matchy(unsigned int left, char *right)
{
    if (left == SQUARE_BRACKET_LEFT && *right == ']')
        return true;
    else if (left == CURLY_BRACKET_LEFT && *right == '}')
        return true;
    else if (left == ANGLE_BRACKET_LEFT && *right == '>')
        return true;
    return false;
}

typedef struct stack_t
{
    unsigned int stack[_STACK_SIZE];
    int idx;
} stack_t;

static bool _stack_push(stack_t *s, char *c)
{
    if (s->idx < _STACK_SIZE)
    {
        unsigned int type = -1;
        switch (*c)
        {
        case ('['):
            type = SQUARE_BRACKET_LEFT;
            break;
        case ('{'):
            type = CURLY_BRACKET_LEFT;
            break;
        case ('<'):
            type = ANGLE_BRACKET_LEFT;
            break;
        }
        s->stack[s->idx] = type;
        ++(s->idx);
        return true;
    }
    else
        printf("stack is full!\n");
    return false;
}

static bool _stack_pop(stack_t *s, unsigned int *ret)
{
    if (s->idx > 0)
    {
        --(s->idx);
        *ret = s->stack[s->idx];
        return true;
    }
    else
        printf("stack is empty!\n");
    return false;
}

bool pattern_parens_is_balanced(char *line)
{
    stack_t parens_stack = {0};

    char *c = line;
    while (*c)
    {
        if (*c == '[' || *c == '{' || *c == '<')
        {
            if (!_stack_push(&parens_stack, c))
                return false;
        }
        else if (*c == ']' || *c == '}' || *c == '>')
        {
            unsigned int matchy = 0;
            if (!_stack_pop(&parens_stack, &matchy))
                return false;

            if (!matchy_matchy(matchy, c))
                return false;
        }
        c++;
    }

    if (parens_stack.idx == 0)
        return true;

    return false;
}

bool parse_pattern(char *line)
{
    if (!pattern_parens_is_balanced(line))
    {
        printf("Belched on yer pattern, mate. it was stinky\n");
        return false;
    }


    pattern_token tokens[MAX_PATTERN] = {0};
    int token_idx = 0;

    extract_tokens_from_line(tokens, &token_idx, line);
    printf("I got %d tokens\n", token_idx);
    print_pattern_tokens(tokens, token_idx);

    //parse_tokens_into_groups(tokens, token_idx);

    return true;
}

