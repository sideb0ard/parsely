#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
    if (isalnum(c) || c == '/' || c == '*' || c == '[' || c == ']' ||
        c == '(' || c == ')' || c == ',' || c == '<' || c == '>' || c == '{' ||
        c == '}' || c == ' ')
        return true;
    return false;
}
static bool isvalidtokenchar(char c)
{
    if (isalnum(c) || c == '/' || c == '*' || c == '(' || c == ')' ||
        c == ',' || c == '<' || c == '>' || c == '{' || c == '}')
        return true;
    return false;
}

static bool is_start_of_modifier(char *c)
{
    printf("Is %c the same as \n", *c);
    if (*c && (*c == '(' || *c == '*' || *c == '/'))
        return true;
    return false;
}

static void extract_mod(char *mod, pattern_token *token) {}

static int parse_expander(char *wurd, pattern_token *token)
{
    // looks for multiplier e.g. bd*2 or [bd bd]*3
    regmatch_t asterisk_group[4];
    regex_t asterisk_rgx;
    //regcomp(&asterisk_rgx, "([][:alnum:]]+)([\\*/]([[:digit:]]+)",
    regcomp(&asterisk_rgx, "([][:alnum:]]+)([\\*/])([[:digit:]]+)",
            REG_EXTENDED | REG_ICASE);

    // looks for euclidean/bjorklund expander e.g. bd(3,8)
    regmatch_t euclid_group[4];
    regex_t euclid_rgx;
    regcomp(&euclid_rgx, "([[:alnum:]]+)\\(([[:digit:]]),([[:digit:]])\\)",
            REG_EXTENDED | REG_ICASE);

    int num_to_increment_by = 0;
    if (regexec(&asterisk_rgx, wurd, 4, asterisk_group, 0) == 0)
    {
        int var_name_len = asterisk_group[1].rm_eo - asterisk_group[1].rm_so;
        char var_name[var_name_len + 1];
        var_name[var_name_len] = '\0';
        strncpy(var_name, wurd + asterisk_group[1].rm_so, var_name_len);

        int op_len = asterisk_group[2].rm_eo - asterisk_group[2].rm_so;
        char var_op[op_len + 1];
        var_op[op_len] = '\0';
        strncpy(var_op, wurd + asterisk_group[2].rm_so, op_len);
        printf("OP is %s\n", var_op);

        int multi_len = asterisk_group[3].rm_eo - asterisk_group[3].rm_so;
        char var_mod[multi_len + 1];
        var_mod[multi_len] = '\0';
        strncpy(var_mod, wurd + asterisk_group[3].rm_so, multi_len);
        int modifier = atoi(var_mod);

        printf("Found an expansion! %s should be %s by %d\n", var_name,
                var_op,
               modifier);

        if (var_op[0] == '*')
        {
            token->has_multiplier = true;
            token->multiplier = modifier;
        }
        else if (var_op[0] == '/')
        {
            token->has_divider = true;
            token->divider = modifier;
        }
        strncpy(token->value, var_name, 99);

        num_to_increment_by = var_name_len+multi_len;
    }
    else if (regexec(&euclid_rgx, wurd, 4, euclid_group, 0) == 0)
    {
        printf("BjORKLUND MATCH!\n");
        int var_name_len = euclid_group[1].rm_eo - euclid_group[1].rm_so;
        char var_name[var_name_len + 1];
        var_name[var_name_len] = '\0';
        strncpy(var_name, wurd + euclid_group[1].rm_so, var_name_len);

        int euclid_hit_len = euclid_group[2].rm_eo - euclid_group[2].rm_so;
        char euc_hit[euclid_hit_len + 1];
        euc_hit[euclid_hit_len] = '\0';
        strncpy(euc_hit, wurd + euclid_group[2].rm_so, euclid_hit_len);
        int ehit = atoi(euc_hit);

        int euclid_step_len = euclid_group[3].rm_eo - euclid_group[3].rm_so;
        char euc_step[euclid_step_len + 1];
        euc_step[euclid_step_len] = '\0';
        strncpy(euc_step, wurd + euclid_group[3].rm_so, euclid_step_len);
        int estep = atoi(euc_step);

        printf("Found an Euclid!! %s should have %d hits over %d steps\n",
               var_name, ehit, estep);

        token->has_euclid = true;
        token->euclid_hits = ehit;
        token->euclid_steps = estep;
        strncpy(token->value, var_name, 99);

        num_to_increment_by = var_name_len+euclid_hit_len+ euclid_step_len;;
    }

    regfree(&asterisk_rgx);
    regfree(&euclid_rgx);

    return num_to_increment_by;
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
            tokens[*token_idx].type = SQUARE_BRACKET_RIGHT;
            if (is_start_of_modifier(c + 1))
            {
                printf("GOTS A MOD!\n");
                int inc = parse_expander(c, &tokens[*token_idx]);
                c += inc;
            }
            (*token_idx)++;
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
            if (is_start_of_modifier(c + 1))
            {
                printf("GOTS A MOD!\n");
                int inc = parse_expander(c, &tokens[*token_idx]);
                c += inc;
            }

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
            while (isvalidtokenchar(*c))
                var_name[var_name_idx++] = *c++;
            tokens[(*token_idx)].type = VAR_NAME;
            strncpy(tokens[*token_idx].value, var_name, MAX_PATTERN_CHAR_VAL);
            parse_expander(var_name, &tokens[*token_idx]);
            printf("VAR! %s\n", var_name);
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

typedef struct wee_stack
{
    unsigned int stack[_STACK_SIZE];
    int idx;
} wee_stack;

static bool _stack_push(wee_stack *s, char *c)
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

static bool _stack_pop(wee_stack *s, unsigned int *ret)
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

bool is_valid_pattern(char *line)
{
    // checks for char in valid chars and balanced parens
    wee_stack parens_stack = {0};

    char *c = line;
    while (*c) // && isvalidpatchar(*c))
    {
        if (!isvalidpatchar(*c) && *c != ' ')
            printf("BARF! '%c'\n", *c);
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

static void parse_cycle_expansions(pattern_token tokens[MAX_PATTERN], int len)
{
    for (int i = 0; i < len; i++)
    {
        // if (tokens[i].type == VAR_NAME)
        //    printf("%s", tokens[i].value);
        // else
        //    printf("%s", token_type_names[tokens[i].type]);
        // if (i < (len - 1))
        //    printf(" ");
        // else
        //    printf("\n");
    }
}

bool parse_pattern(char *line)
{
    if (!is_valid_pattern(line))
    {
        printf("Belched on yer pattern, mate. it was stinky\n");
        return false;
    }

    pattern_token tokens[MAX_PATTERN] = {0};
    int token_idx = 0;

    extract_tokens_from_line(tokens, &token_idx, line);
    printf("I got %d tokens\n", token_idx);
    print_pattern_tokens(tokens, token_idx);

    // parse_tokens_into_groups(tokens, token_idx);

    return true;
}
