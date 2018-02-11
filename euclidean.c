#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "euclidean.h"

static void print_bin_num(int num)
{
    int len = 16; // short
    for (int i = len - 1; i >= 0; --i)
    {
        if (num & 1 << i)
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void build_euclidean_pattern_short(int level, uint16_t *bitmap_int,
                                   uint16_t *bitmap_len, int *count,
                                   int *remainderrr)
{
    if (level == -1)
    {
        (*bitmap_len)++;
        *bitmap_int = *bitmap_int << 1;
    }
    else if (level == -2)
    {
        (*bitmap_len)++;
        *bitmap_int = (*bitmap_int << 1) + 1;
    }
    else
    {
        for (int i = 0; i < count[level]; i++)
        {
            build_euclidean_pattern_short(level - 1, bitmap_int, bitmap_len,
                                          count, remainderrr);
        }
        if (remainderrr[level] != 0)
        {
            build_euclidean_pattern_short(level - 2, bitmap_int, bitmap_len,
                                          count, remainderrr);
        }
    }
}

void build_euclidean_pattern_string(int level, char *bitmap_string, int *count,
                                    int *remaindrrr)
{
    if (level == -1)
    {
        strcat(bitmap_string, "0");
    }
    else if (level == -2)
    {
        strcat(bitmap_string, "1");
    }
    else
    {
        for (int i = 0; i < count[level]; i++)
        {
            build_euclidean_pattern_string(level - 1, bitmap_string, count,
                                           remaindrrr);
        }
        if (remaindrrr[level] != 0)
        {
            build_euclidean_pattern_string(level - 2, bitmap_string, count,
                                           remaindrrr);
        }
    }
}

uint16_t create_euclidean_rhythm(int num_hits, int num_steps)
{
    int remaindrrr[num_steps];
    int count[num_steps];
    memset(count, 0, num_steps * sizeof(int));
    memset(remaindrrr, 0, num_steps * sizeof(int));

    int level = 0;
    int divisor = num_steps - num_hits;
    remaindrrr[level] = num_hits;
    do
    {
        count[level] = divisor / remaindrrr[level];
        remaindrrr[level + 1] = divisor % remaindrrr[level];
        divisor = remaindrrr[level];
        level++;
    } while (remaindrrr[level] > 1);
    count[level] = divisor;

    // now calculate return value
    uint16_t bitmap_len = 0;
    uint16_t bitmap_int = 0;

    build_euclidean_pattern_short(level, &bitmap_int, &bitmap_len, count,
                                  remaindrrr);

    uint16_t max_bits_to_align_with = 16; // max

    uint16_t first_bit;
    for (int i = max_bits_to_align_with; i >= 0; i--)
    {
        if (bitmap_int & (1 << i))
        {
            first_bit = i;
            break;
        }
    }

    uint16_t bitshift_by = (max_bits_to_align_with - 1) - first_bit;
    uint16_t aligned_bitmap = bitmap_int << bitshift_by;
    print_bin_num(aligned_bitmap);
    return aligned_bitmap;
}
