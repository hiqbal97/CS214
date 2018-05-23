#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mymalloc.h"

static double a[100];
static int counterA = 0;
static double b[100];
static int counterB = 0;
static double c[100];
static int counterC = 0;
static double d[100];
static int counterD = 0;
static double e[100];
static int counterE = 0;
static double f[100];
static int counterF = 0;

/*
A: malloc() 1 byte and immediately free it - do this 150 times
*/
void workloadcaseA()
{
    char *ptr = NULL;
    int i;
    struct timeval tv1;
    struct timeval tv2;
    gettimeofday(&tv1, NULL);
    for (i = 0; i < 150; i++) 
    {
                ptr = malloc(1);
                if (ptr != NULL)
                {
                        free(ptr);
                }
        }
    gettimeofday(&tv2, NULL);
a[counterA] = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
}

/*
B: malloc() 1 byte, store the pointer in an array - do this 150 times.
Once you've malloc()ed 150 byte chunks, then free() the 150 1 byte pointers one by one.
*/
void workloadcaseB()
{
        char * ptr[150];
        int i;
        struct timeval tv1;
        struct timeval tv2;
        gettimeofday(&tv1, NULL);
        for(i=0;i<150;i++){
                ptr[i]=malloc(1);
        }
        for(i=0;i<150;i++){
                free(ptr[i]);
        }
        gettimeofday(&tv2, NULL);
        b[counterB] = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
}
/*
C: Randomly choose between a 1 byte malloc() or free()ing a 1 byte pointer - do this 150 times
- Keep track of each operation so that you eventually malloc() 150 bytes, in total
- Keep track of each operation so that you eventually free() all pointers
(don't allow a free() if you have no pointers to free())
*/
void workloadcaseC(){
        char * ptr[150];
        struct timeval tv1;
        struct timeval tv2;
        int i;
        gettimeofday(&tv1, NULL);
        int mallocCount = 0;
        int freeCount = 0;
        int numberofnotfreedptrs = 0;

        while(mallocCount<150){
                int randomnum = random()%2; //randomnum is 0 or 1
                if(randomnum == 1){
                        char *temp = malloc(1);
                        mallocCount++;
                        ptr[numberofnotfreedptrs] = temp;
                        numberofnotfreedptrs++;
                }
                else if(randomnum==0&&numberofnotfreedptrs!=0){
                        free(ptr[numberofnotfreedptrs-1]);
                                numberofnotfreedptrs--;
                                freeCount++;
                        }
                }
                while(freeCount<150){
                        free(ptr[numberofnotfreedptrs-1]);
                        numberofnotfreedptrs--;
                        freeCount++;

                }

                gettimeofday(&tv2, NULL);
                c[counterC] = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
        

}
/*
D: Randomly choose between a randomly-sized malloc() or free()ing a pointer – do this many times (see below)
- Keep track of each malloc so that all mallocs do not exceed your total memory capacity
- Keep track of each operation so that you eventually malloc() 150 times
- Keep track of each operation so that you eventually free() all pointers
- Choose a random allocation size between 1 and 64 bytes 
*/

void workloadcaseD(){
		char * ptr[150];
        struct timeval tv1;
        struct timeval tv2;
        int i;
        gettimeofday(&tv1, NULL);
        int mallocCount = 0;
        int freeCount = 0;
        int numberofnotfreedptrs = 0;
        int totalsizemallocd = sizeof(node);

        while(mallocCount<150){
        	int randomnum = random()%2;
        	if(randomnum == 1){
        		int randomallocationsize = random()%64+1;
        		if(randomallocationsize>(5000 - totalsizemallocd)){
        			continue;
        		}
        		char * temp = malloc(randomallocationsize);
        		totalsizemallocd+= sizeof(node) + randomallocationsize;
        		mallocCount++;
        		ptr[numberofnotfreedptrs]=temp;
        		numberofnotfreedptrs++;
        	}
        	else if(randomnum==0&&numberofnotfreedptrs>0){
        		node * temp = (node*)ptr[numberofnotfreedptrs-1]-1;
        		short temporarySize=temp->size;
        		totalsizemallocd-=temporarySize+sizeof(node);
        		free(ptr[numberofnotfreedptrs-1]);
        		numberofnotfreedptrs--;
        		freeCount++;
        	}

        	}

        	 while(freeCount<150){
                        free(ptr[numberofnotfreedptrs-1]);
                        numberofnotfreedptrs--;
                        freeCount++;

                }

                gettimeofday(&tv2, NULL);
                d[counterD] = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
        


        } 
/*
E: Calls malloc to ptr to malloc once and then free, then taking the same ptr and mallocing with incrementing bytes up to 150
*/
void workloadcaseE()
{
	struct timeval tv1;
    struct timeval tv2;
    gettimeofday(&tv1, NULL);
	void *ptr;
    int i;
    int pointer = 1;
    for (i = 0; i<150;i++)
    {
    	pointer++;
    	ptr = malloc(pointer);
    	if (ptr != NULL)
        {
        	free(ptr);
        }
      
    }
    gettimeofday(&tv2, NULL);
    e[counterE] = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
}
/*
F: Calls malloc 150 times for 1 byte
Stores the pointers in an array
Frees the pointers randomly
*/
void workloadcaseF()
{
		char * ptr[150];
        struct timeval tv1;
        struct timeval tv2;
        int i;
        gettimeofday(&tv1, NULL);

        for(i=0;i<150;i++){
        	ptr[i]=malloc(1);
        }
        int freeCount = 0;

        while(freeCount<150){
        	int randomnum = rand()%150; //random number 0 - 150
        	char *ptrs = ptr[randomnum];
        	if(ptrs=NULL){ //If the ptr is null continue because we shouldn't free it
        		continue;
        	}
        	free(ptrs);
        	ptr[randomnum]=NULL;
        	freeCount++;
        }

        gettimeofday(&tv2,NULL);

        f[counterF] = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

}

void main()
{
    int i = 0;
    for(i = 0; i < 100; i++)
    {
		workloadcaseA();
		counterA++;
    }
    double timeA = 0;
    for(counterA = 0; counterA < 100; counterA++)
    {
    	timeA += a[counterA];
    }
    timeA = timeA/100;
    printf("WorkloadA Time: %lf\n", timeA);
    for(i = 0; i < 100; i++)
    {
		workloadcaseB();
		counterB++;
    }
    double timeB = 0;
    for(counterB = 0; counterB < 100; counterB++)
    {
    	timeB += b[counterB];
    }
    timeB = timeB/100;
    printf("WorkloadB Time: %lf\n", timeB);
    for(i = 0; i < 100; i++)
    {
		workloadcaseC();
		counterC++;
    }
    double timeC = 0;
    for(counterC = 0; counterC < 100; counterC++)
    {
    	timeC += c[counterC];
    }
    timeC = timeC/100;
    printf("WorkloadC Time: %lf\n", timeC);
    for(i = 0; i < 100; i++)
    {
		workloadcaseD();
		counterD++;
    }
    double timeD = 0;
    for(counterD = 0; counterD < 100; counterD++)
    {
    	timeD += d[counterD];
    }
    timeD = timeD/100;
    printf("WorkloadD Time: %lf\n", timeD);
    for(i = 0; i < 100; i++)
    {
		workloadcaseE();
		counterE++;
    }
    double timeE = 0;
    for(counterE = 0; counterE < 100; counterE++)
    {
    	timeE += e[counterE];
    }
    timeE = timeE/100;
    printf("WorkloadE Time: %lf\n", timeE);
    for(i = 0; i < 100; i++)
    {
		workloadcaseF();
		counterF++;
    }
    double timeF = 0;
    for(counterF = 0; counterF < 100; counterF++)
    {
    	timeF += f[counterF];
    }
    timeF = timeF/100;
    printf("WorkloadF Time: %lf\n", timeF);
}