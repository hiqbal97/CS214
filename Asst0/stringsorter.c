#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void sort(char *words)
{
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    char doubleA[50][100];
    char array[50];
    for(a = 0; a < strlen(words); a++)
    {
        if ((words[a] == ' '))
            d++;
    }
    a = 0;
    c = 0;
    for(b = 0; b < strlen(words); b++)
    {
        if((words[b] == ' '))
        {
        	doubleA[a][c] = '\0';
                a++;
                c = 0;
        }        
        else
        {
            doubleA[a][c++] = words[b];
        }
    }
    for(a = 0; a < d; a++)
    {
        for(b = a+1; b <= d; b++)
        {
            if((strcmp(doubleA[a], doubleA[b]) > 0))
            {
                strcpy(array, doubleA[a]);
                strcpy(doubleA[a], doubleA[b]);
                strcpy(doubleA[b], array);
            }
        }
    }
    for(a = 0;a <= d;a++)
    {
        printf("%s", doubleA[a]);
        for(b = a-1; b < a; b++)
            printf("\n");
    }
}

int main(int argc, char** argv)
{    
    int i;
    int length;
    char* input;
    if(argc != 2)
    	exit(0);   
    length = strlen(argv[1]);
    input = (char*)malloc(length*sizeof(char));    
    for(i=0; i<length; i++)
        input[i] = argv[1][i];
    for(i=0; i<length; i++)
    {
        if(!isalpha(input[i]))
            input[i] = ' ';
    }
    sort(input);
    return(0);
}
