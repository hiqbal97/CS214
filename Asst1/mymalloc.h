#ifndef _mymalloc_h_
#define _mymalloc_h_

#include <stdio.h>
#include <stdlib.h>

#define malloc(x) mymalloc( x, __FILE__, __LINE__ )
#define free(x) myfree( x, __FILE__, __LINE__ )

typedef struct node{
	char free; //if the block is free = 1, Otherwise = 0
	int size; //signed data type
} node;

void *mymalloc(size_t size, char* file, int linenumber);
void myfree(void *ptr, char* file, int linenumber);

#endif