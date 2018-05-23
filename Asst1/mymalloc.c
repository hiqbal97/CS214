#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "mymalloc.h"


#define memory 5000

static char myblock[memory]; // block of 5000 bytes memory space

static int init = 0;

void initialize() { //initialize node blcck
	node *ptr= (node*) myblock;
	ptr->free=1;
	ptr->size=4992;
	init=1;
}


void *mymalloc(size_t size, char* file, int linenumber){
	if(size == 0 ){
		return NULL;
	}

	if(init==0) {
		initialize();
	}

	node *ptr = (node*) myblock; //create a pointer at myblock
	while((char*)ptr<myblock+5000){ //goes from each node myblock
		if(ptr->free==1 && (unsigned) ptr->size >= size){
			if((unsigned) ptr->size < (size + sizeof(node) + 1)){  //find one free thats big enough
				// myblock is free and can hold the bytes
				// take all of it / a little extra
				// then make it not free
				// (Case 2)
				ptr->free = 0;
				return (char*)ptr + sizeof(node); //return it
			}
			else{
				//Case 1 (the amount of memory we need to create it)
				ptr->free = 0; //make it not free
				int originalSize=ptr->size;
				ptr->size=size;
				node* orig=ptr;
				
				node* next=(node*)((char*)ptr + ptr->size+sizeof(node));
				next->size=originalSize-size-sizeof(node);
				next->free=1;
				//create new node
				return (char*)ptr+ sizeof(node); // return it
			}
		}
		ptr=(node*)((char*)ptr + sizeof(node) +ptr->size);
	}
	
	
	//error statement
	fprintf(stderr, "Not enough memory for %d bytes in the file - '%s' : This occurs on line : %d\n ", size, file, linenumber);
	return NULL;
}

//check to see if the address of ptr exists
int isValidEntry(node* ptr) {
	int isValid=0;
//creates temporary pointers for myblock to check for
	char* head=myblock;

	char* test=myblock;


	while(head<myblock+5000) {//traverses through the block
		if(head==(char*)ptr) {//head is equal to the ptr
			isValid=1;
			break;
		}
		else {
		}

		head=head+((node*)head)->size + sizeof(node);//head is set to size of node
	}

	return isValid;
}


void myfree(void *ptr, char* file, int linenumber)
{
	if(ptr == NULL)
	{
		fprintf(stderr, "HELP THE POINTER IS NULL in the file - '%s' : This occurs on line : %d\n ", file, linenumber);
		return;
	}

	node *pointer=(node*)((char*)ptr -sizeof(node));

	char* head=myblock;
//calls to the valid command to check if the 
	if(isValidEntry(pointer)==0)
	{
		fprintf(stderr, "MALLOC ERROR in %s:%d: Pointer %d not malloc'd\n", file, linenumber,ptr);
		return;
	}

//checks if the pointer is free
	if(pointer->free==1)
	{
		fprintf(stderr, "MALLOC ERROR in %s:%d: Pointer already free\n", file, linenumber);
		return;
	}
//this is when it will free the pointer
	pointer->free=1;
//creates a node for the next in the block to see if the that is included in the block
//if it is it will be freed
//if its not it will return
	node *next= (node*)((char*)pointer +sizeof(node) + pointer->size);
//checks if the next is allocated
	int nextValidEntry=isValidEntry(next);

	if(nextValidEntry==1&&next->free==1) {
		pointer->size=pointer->size+sizeof(node) + next->size;
	}

	node* curr=(node*) myblock;
	node* prev;
//if the current node and next node are both same memory location wise the program ends
//if not the free next
	if(curr==pointer) {
		return;
	}
	while(curr!=pointer) {
		prev=curr;
		curr=(node*)((char*)curr +sizeof(node)+curr->size);
	}

	if(prev->free==1) {
		prev->size=prev->size+sizeof(node)+pointer->size;
	}

	
}