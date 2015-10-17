/*
 * getopt - POSIX like getopt for Windows console Application
 *
 * win-c - Windows Console Library
 * Copyright (c) 2015 Koji Takami
 * Released under the MIT license
 * https://github.com/takamin/win-c/blob/master/LICENSE
 */
#include <stdio.h>
#include <string.h>
#include "getopt.h"

char* optarg = 0;
int optind = 1;
int opterr = 1;
int optopt = 0;

static void postpone(int argc, char* const argv[], int index) {
    char** nc_argv = (char**)argv;
    char* p = nc_argv[index];
    int j = index;
    for(; j < argc - 1; j++) {
        nc_argv[j] = nc_argv[j + 1];
    }
    nc_argv[argc - 1] = p;
}
static int postpone_noopt(int argc, char* const argv[], int index) {
    int i = index;
    for(; i < argc; i++) {
        if(*(argv[i]) == '-') {
            postpone(argc, argv, index);
            return 1;
        }
    }
    return 0;
}
static int _getopt_(int argc, char* const argv[],
        const char* optstring,
        const struct option* longopts, int* longindex)
{
    static int postpone_count = 0;
    static int nextchar = 0;
    while(1) {
        int c;
        const char* optptr = 0;
        if(optind >= argc - postpone_count) {
            break;
        }
        c = *(argv[optind] + nextchar);
        if(c == '\0') {
            nextchar = 0;
            ++optind;
            continue;
        }
        if(nextchar == 0) {
            if(optstring[0] != '+' && optstring[0] != '-') {
                while(c != '-') {
                    /* postpone non-opt parameter */
                    if(!postpone_noopt(argc, argv, optind)) {
                        break; /* all args are non-opt param */
                    }
                    ++postpone_count;
                    c = *argv[optind];
                }
            }
            if(c != '-') {
                if(optstring[0] == '-') {
                    optarg = argv[optind];
                    nextchar = 0;
                    ++optind;
                    return 1;
                }
                break;
            } else {
                if(strcmp(argv[optind], "--") == 0) {
                    optind++;
                    break;
                }
                ++nextchar;
                if(longopts != 0 && *(argv[optind] + nextchar) == '-') {
                    char const* spec_long = argv[optind] + nextchar + 1;
                    int spec_len = 0;
                    char const* pos_eq = strchr(spec_long, '=');
                    if(pos_eq == NULL) {
                        spec_len = strlen(spec_long);
                    } else {
                        spec_len = pos_eq - spec_long;
                    }
                    int index_search = 0;
                    int index_found = -1;
                    const struct option* optdef = 0;
                    while(longopts->name != 0) {
                        if(strncmp(spec_long, longopts->name, spec_len) == 0) {
                            if(optdef != 0) {
                                break;
                            }
                            optdef = longopts;
                            index_found = index_search;
                        }
                        longopts++;
                        index_search++;
                    }
                    if(longopts->name != 0) {
                        if(opterr) {
                            fprintf(stderr, "ambiguous option: %s\n", spec_long);
                        }
                        return '?';
                    }
                    if(optdef == 0) {
                        if(opterr) {
                            fprintf(stderr, "no such a option: %s\n", spec_long);
                        }
                        return '?';
                    }
                    if(optdef->has_arg == no_argument && pos_eq > 0) {
                        if(opterr) {
                            fprintf(stderr, "no such a option: %s\n", spec_long);
                        }
                        return '?';
                    }
                    switch(optdef->has_arg) {
                        case no_argument:
                            optarg = 0;
                            break;
                        case required_argument:
                            if(pos_eq == NULL) {
                                ++optind;
                                if(strcmp(argv[optind], "=") == 0) {
                                    optarg = argv[++optind];
                                } else if(strncmp(argv[optind], "=", 1) == 0) {
                                    optarg = argv[optind] + 1;
                                } else {
                                    optarg = argv[optind];
                                }
                            } else {
                                if(*(argv[optind] + spec_len + 1) == '\0') {
                                    optarg = argv[++optind];
                                } else {
                                    optarg = argv[optind] + spec_len + 1;
                                }
                            }
                            break;
                    }
                    ++optind;
                    nextchar = 0;
                    if(longindex != 0) {
                        *longindex = index_found;
                    }
                    if(optdef->flag != 0) {
                        *optdef->flag = optdef->val;
                        return 0;
                    }
                    return optdef->val;
                }
                continue;
            }
        }
        optptr = strchr(optstring, c);
        if(optptr == NULL) {
            optopt = c;
            if(opterr) {
                fprintf(stderr,
                        "%s: invalid option -- %c\n",
                        argv[0], c);
            }
            ++nextchar;
            return '?';
        }
        if(*(optptr+1) != ':') {
            nextchar++;
            if(*(argv[optind] + nextchar) == '\0') {
                ++optind;
                nextchar = 0;
            }
            optarg = 0;
        } else {
            nextchar++;
            if(*(argv[optind] + nextchar) != '\0') {
                optarg = argv[optind] + nextchar;
            } else {
                ++optind;
                if(optind < argc - postpone_count) {
                    optarg = argv[optind];
                } else {
                    optopt = c;
                    if(opterr) {
                        fprintf(stderr,
                            "%s: option requires an argument -- %c\n",
                            argv[0], c);
                    }
                    if(optstring[0] == ':' ||
                        (optstring[0] == '-' || optstring[0] == '+') &&
                        optstring[1] == ':')
                    {
                        c = ':';
                    } else {
                        c = '?';
                    }
                }
            }
            ++optind;
            nextchar = 0;
        }
        return c;
    }

    /* end of option analysis */

    /* fix the order of non-opt params to original */
    while((argc - optind - postpone_count) > 0) {
        postpone(argc, argv, optind);
        ++postpone_count;
    }

    nextchar = 0;
    postpone_count = 0;
    return -1;
}

int getopt(int argc, char* const argv[],
            const char* optstring)
{
    return _getopt_(argc, argv, optstring, 0, 0);
}
int getopt_long(int argc, char* const argv[],
        const char* optstring,
        const struct option* longopts, int* longindex)
{
    return _getopt_(argc, argv, optstring, longopts, longindex);
}
/********************************************************
int getopt_long_only(int argc, char* const argv[],
        const char* optstring,
        const struct option* longopts, int* longindex)
{
    return -1;
}
********************************************************/

