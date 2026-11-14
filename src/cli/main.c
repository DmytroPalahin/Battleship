/* SPDX-License-Identifier: MIT */
/**
 * @file main.c
 * @brief Terminal front-end: argument parsing and command dispatch.
 *
 * This program exists to prove a point made in the very first lecture of the
 * course: the same model can be driven by more than one interface. It links
 * the game engine and nothing else - no GTK, no display.
 */
#include "cli.h"

#include "battleship/battleship.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_usage(FILE *out)
{
    fputs("Usage: battleship-cli <command> [options]\n"
          "\n"
          "Commands:\n"
          "  play       play against the computer in the terminal\n"
          "  simulate   benchmark the three AI levels against random fleets\n"
          "\n"
          "Options:\n"
          "  -l, --level <easy|medium|hard>  computer level (play: default medium;\n"
          "                                  simulate: restrict to one level)\n"
          "  -s, --seed <n>                  random seed (play: default from clock;\n"
          "                                  simulate: default 1)\n"
          "  -n, --games <n>                 simulate: games per level (default 1000)\n"
          "      --csv                       simulate: print the shot histogram as CSV\n"
          "      --no-duel                   simulate: skip the head-to-head table\n"
          "  -h, --help                      show this help\n"
          "  -V, --version                   show the version\n",
          out);
}

/* Parses a non-negative decimal integer; rejects anything else. */
static bool parse_u64(const char *text, uint64_t *out)
{
    if (text == NULL || *text == '\0' || *text == '-' || *text == '+') {
        return false;
    }
    char *end = NULL;
    errno = 0;
    const unsigned long long value = strtoull(text, &end, 10);
    if (errno != 0 || *end != '\0') {
        return false;
    }
    *out = (uint64_t)value;
    return true;
}

static bool matches(const char *argument, const char *short_name, const char *long_name)
{
    return (short_name != NULL && strcmp(argument, short_name) == 0) ||
           strcmp(argument, long_name) == 0;
}

/* Fetches the value following an option, or reports that it is missing. */
static const char *option_value(int argc, char **argv, int *index)
{
    if (*index + 1 >= argc) {
        fprintf(stderr, "battleship-cli: option '%s' needs a value\n", argv[*index]);
        return NULL;
    }
    (*index)++;
    return argv[*index];
}

int main(int argc, char **argv)
{
    cli_options options = {
        .level = BS_DIFFICULTY_MEDIUM,
        .level_given = false,
        .seed = 1,
        .seed_given = false,
        .games = 1000,
        .csv = false,
        .duel = true,
    };
    const char *command = NULL;

    for (int i = 1; i < argc; i++) {
        const char *argument = argv[i];

        if (matches(argument, "-h", "--help")) {
            print_usage(stdout);
            return EXIT_SUCCESS;
        }
        if (matches(argument, "-V", "--version")) {
            printf("battleship-cli %s\n", bs_version());
            return EXIT_SUCCESS;
        }
        if (matches(argument, "-l", "--level")) {
            const char *value = option_value(argc, argv, &i);
            if (value == NULL || !bs_difficulty_parse(value, &options.level)) {
                fprintf(stderr, "battleship-cli: level must be easy, medium or hard\n");
                return EXIT_FAILURE;
            }
            options.level_given = true;
        } else if (matches(argument, "-s", "--seed")) {
            if (!parse_u64(option_value(argc, argv, &i), &options.seed)) {
                fprintf(stderr, "battleship-cli: seed must be a non-negative integer\n");
                return EXIT_FAILURE;
            }
            options.seed_given = true;
        } else if (matches(argument, "-n", "--games")) {
            uint64_t games = 0;
            if (!parse_u64(option_value(argc, argv, &i), &games) || games == 0 ||
                games > CLI_MAX_GAMES) {
                fprintf(stderr, "battleship-cli: games must be between 1 and %d\n", CLI_MAX_GAMES);
                return EXIT_FAILURE;
            }
            options.games = (int)games;
        } else if (matches(argument, NULL, "--csv")) {
            options.csv = true;
        } else if (matches(argument, NULL, "--no-duel")) {
            options.duel = false;
        } else if (argument[0] == '-') {
            fprintf(stderr, "battleship-cli: unknown option '%s'\n", argument);
            return EXIT_FAILURE;
        } else if (command == NULL) {
            command = argument;
        } else {
            fprintf(stderr, "battleship-cli: unexpected argument '%s'\n", argument);
            return EXIT_FAILURE;
        }
    }

    if (command == NULL) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    if (strcmp(command, "simulate") == 0) {
        return cli_simulate(&options);
    }
    if (strcmp(command, "play") == 0) {
        if (!options.seed_given) {
            options.seed = (uint64_t)time(NULL);
        }
        return cli_play(&options);
    }

    fprintf(stderr, "battleship-cli: unknown command '%s'\n", command);
    print_usage(stderr);
    return EXIT_FAILURE;
}
