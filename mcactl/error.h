#ifndef ERROR_H
#define ERROR_H


#include <stdio.h>
#include <stdlib.h>


#if 0
/* 定義するとデバッグ用の表示をする */
#define DEBUG_
#endif



#define error_only(...) ( \
	printf_dbg("\n%s(%d)", __FILE__, __LINE__), \
	fputs("\nError : ", stderr), \
	fprintf(stderr, __VA_ARGS__), \
	fputs("\n", stderr)) \


#define error_exit(...) ( \
	error_only(__VA_ARGS__), \
	exit(EXIT_FAILURE))


#ifdef DEBUG_
# define printf_dbg(...) fprintf(stderr, __VA_ARGS__)
#else
# define printf_dbg(...) ((void)0)
#endif

#define printf_dbg_d(...) ( \
	printf_dbg("%s(%d)  ", __FILE__, __LINE__), \
	printf_dbg(__VA_ARGS__ ))


#endif
