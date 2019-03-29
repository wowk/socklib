#ifndef DEBUG_H__
#define DEBUG_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef error
#undef error
#endif

#ifdef debug
#undef debug
#endif

#ifdef info
#undef info
#endif

#define error(need_exit, errcode, fmt, args...)  do{\
    fprintf(stderr, "\033[1;31m");\
    fprintf(stderr, "[ error ] [ %-.16s : %-5d] ", __FUNCTION__, __LINE__-1);\
    fprintf(stderr, fmt, ##args);\
    fprintf(stderr, "  : %s\n", strerror(errcode));\
    fprintf(stderr, "\033[1;0m");\
    fflush(stderr);\
    if( need_exit )\
        exit(errcode);\
    }while(0)

#define debug(onoff, fmt, args...)do{\
    if(onoff){\
        fprintf(stdout, "\033[1;32m");\
        fprintf(stdout, "[ debug ] [ %-.16s : %-5d] ", __FUNCTION__, __LINE__);\
        fprintf(stdout, fmt, ##args);\
        fprintf(stdout, "\033[1;0m");\
        fprintf(stdout, "\n");\
        fflush(stdout);\
    }\
    }while(0)

#define info(fmt, args...)do{\
    fprintf(stdout, "\033[1;34m");\
    fprintf(stdout, "[ info ] [ %-.16s : %-5d] ", __FUNCTION__, __LINE__);\
    fprintf(stdout, fmt, ##args);\
    fprintf(stdout, "\033[1;0m");\
    fprintf(stdout, "\n");\
    fflush(stdout);\
    }while(0)

#endif
