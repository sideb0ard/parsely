#include <stdio.h>
#include <stdlib.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "pattern_parser.h"

void interpret(char *line)
{
    printf("%s\n", line);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Parsy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    char *line;
    while ((line = readline("parsy> ")) != NULL)
    {
        if (line && *line)
        {
            add_history(line);
            parse_pattern(line);
            free(line);
        }
    }

}
