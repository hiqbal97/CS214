#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define SIZE (36)
#define CINDEX(c) ((int) c - (int)'a')
#define NINDEX(c) (c - '0') + 26

typedef struct linked
{
    struct linked *next;
    bool x;
    char id[512];
    int times;
}linked;

typedef struct Trie
{
    struct Trie *child[SIZE];
    bool leaf;
    char word[512];
    linked *branch;
}Trie;

typedef struct node
{
    char* dir;
    char* name;
    struct node *next;
}node;

node *head = NULL;
node *ptr = NULL;
bool single = false;

void freeNode(node *root)
{
    node *temp;
    while(root != NULL)
    {
        temp = root;
        root = root->next;
        free(temp);
    }
}

void freeLinked(linked *root)
{
    linked* temp;
    while(root != NULL)
    {
        temp = root;
        root = root->next;
        free(temp);
    }
}

Trie *initialize(void)
{
    Trie *temp = NULL;
    temp = (Trie*) malloc(sizeof(Trie));
    if(temp)
    {
        int i;
        temp->leaf = false;
        temp->branch = NULL;
        for(i = 0; i < SIZE; i++)
        {
            temp->child[i] = NULL;
        }
    }
    return temp;
}

void insert(Trie *root, const char *key, const char *file)
{
    int x;
    int index;
    int length = strlen(key);
    Trie *temp = root;
    for(x = 0; x < length; x++)
    {
        if(isalpha(key[x]))
        {
            index = CINDEX(key[x]);
            if(!temp->child[index])
                temp->child[index] = initialize();
            temp = temp->child[index];
        }
        else if(isdigit(key[x]))
        {
            index = NINDEX(key[x]);
            if(!temp->child[index])
                temp->child[index] = initialize();
            temp = temp->child[index];
        }
    }
    temp->leaf = true;
    temp->branch = (linked*) malloc(sizeof(linked));
    temp->branch->times = 1;
    temp->branch->next = NULL;
    temp->branch->x = false;
    sprintf(temp->word, "%s", key);
    sprintf(temp->branch->id, "%s", file);
}

void linkedList(linked **root, const char *fileName)
{
    linked *temp = *root;
    while(true)
    {
        if(strcmp(fileName, temp->id) == 0)
        {
            temp->times++;
            return;
        }
        if(temp->next == NULL)
        {
            break;
        }
        temp = temp->next;
    }
    linked *tmp;
    tmp = (linked*) malloc(sizeof(linked));
    sprintf(tmp->id, "%s", fileName);
    tmp->times = 1;
    tmp->x = false;
    tmp->next = (*root);
    (*root) = tmp;
}

void TrieInsert(Trie *root, const char *input, const char *file)
{
    int i;
    int index;
    int len = strlen(input);
    Trie *temp = root;
    for(i = 0; i < len; i++)
    {
        if(isalpha(input[i]))
        {
            index = CINDEX(input[i]);
            if(!temp->child[index])
            {
                insert(root, input, file);
                break;
            }
            temp = temp->child[index];
        }
        else if(isdigit(input[i]))
        {
            index = NINDEX(input[i]);
            if(!temp->child[index])
            {
                insert(root, input, file);
                break;
            }
        }
        if(i == (len - 1) && temp->leaf == true && temp != NULL)
        {
            linkedList(&(temp->branch), file);
            break;
        }
    }
}

int numnodes(const void *x, const void *y)
{
    const linked *first = *(const linked **) x;
    const linked *second = *(const linked **) y;
    if (first->times == second->times)
        return strcmp(first->id, second->id);
    else
        return (first->times - second->times) * (-1);
}

void printList(linked *root, FILE *fp)
{
   if(!root)
   {
       return;
   }
   linked *list[512];
   linked *tmp = root;
   int x = 0;
   while(tmp != NULL)
   {
       list[x++] = tmp;
       tmp = tmp->next;
   }
   qsort(list, x, sizeof(linked *), numnodes);
   int i;
   for(i = 0; i < x; i++)
   {
       fprintf(fp, "\t\t<file name=\"%s\">%d</file>\n", list[i]->id, list[i]->times);
   }
}

void result(FILE *fp, Trie *root)
{
    int i;
    if(!root)
        return;
    if(root->leaf)
    {
        fprintf(fp, "\t<word text=\"%s\">\n", root->word);
        printList(root->branch, fp);
        freeLinked(root->branch);
        fprintf(fp, "\t</word>\n");
    }
    for(i = 0; i < SIZE; i++)
        result(fp, root->child[i]);
    free(root);
}

void directory(const char *pname, int x)
{
    DIR *dir;
    struct dirent *k;
    if (!(dir = opendir(pname)))
    {
        single = true;
        return;
    }
    if (!(k = readdir(dir)))
        return;
    do 
    {
        if(k->d_type == DT_DIR)
        {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", pname, k->d_name);
            path[len] = 0;
            if (strcmp(k->d_name, ".") == 0 || strcmp(k->d_name, "..") == 0)
                continue;
            directory(path, x + 1);
        }
        else if(k->d_type == DT_REG)
        {
            char* temp = malloc(sizeof(pname) + sizeof(k->d_name) + 2);
            sprintf(temp, "%s/%s", pname, k->d_name);
            ptr->dir = strdup(temp);
            ptr->name = strdup(k->d_name);
            ptr->next = malloc(sizeof(node) + 1);
            ptr = ptr->next;
        }
    }while((k = readdir(dir)) != NULL);
    closedir(dir);
}

int main(int argc, char **argv) 
{
    node *a = head;
    node *b = ptr;
    if(argc != 3)
    {
        printf("Arguments should be 3, therefore the number of arguments is incorrect\n");
        return 0;
    }
    int fd0=open(argv[1],O_RDWR);
    if(fd0>0)
    {
        printf("%s already exists. Enter y to override.\n",argv[1]);
        char buffer[2];
        read(STDIN_FILENO,buffer,1);
        buffer[1]='\0';
        if(strcmp(buffer,"y")!=0)
        {
            close(fd0);
            exit(-1);
        }
        else
        {
            remove(argv[1]);
        }
    }
    close(fd0);
    head = malloc(sizeof(node) + 1);
    ptr = head;
    Trie *root = initialize();
    const char *make = argv[1];
    FILE *fp;
    fp = fopen(make, "a");
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<fileIndex>\n");
    directory(argv[2], 0);
    if(single)
    {
    	int fd=open(argv[2], O_RDONLY);
        if(fd<0)
        {
            printf("Directory/File doesn't exist\n");
            close(fd);
            exit(0);
        }
        struct stat st;
        fstat(fd,&st);
        off_t size=st.st_size;
        char* path=argv[2];
        char* prev=argv[2];
        char* split0=strtok(path,"/");
        prev=split0;
        while(split0!=NULL)
        {
            prev=split0;
            split0=strtok(NULL,"/");
        }
        Trie* root1=initialize();
        char* data=(char*)malloc(sizeof(char)*(size+1));
        int amountRead=0;
        while(amountRead<size)
        {
            int r=read(fd,data+amountRead,size-amountRead);
            amountRead+=r;
        }
        data[size]='\0';
        int i;
        for(i=0; i< size;i++)
        {
            if(!isalpha(data[i]))
            	data[i] = '|';
        }

        char *split = strtok(data, "|");
        while (split != NULL)
        {
            int length=strlen(split);
            int j;
            for(j=0;j<length;j++)
                *(split+j)=tolower(*(split+j));
            TrieInsert(root1,split,prev);
            split = strtok(NULL, "|");
            if(split==NULL)
                break;
        }
        result(fp,root1);
        close(fd);
    }
    else
    {
    	while(head->next != NULL)
    	{
        	char *wordB;
        	int word = 0;
        	char x;
        	FILE *dirFile;
        	dirFile = fopen(head->dir, "r");
        	do
        	{
            	x = getc(dirFile);
            	if(isalpha(x))
            	{
                	x = tolower(x);
                	if(word == 0)
                	{
                    	wordB = malloc(sizeof(char) * 250);
                    	sprintf(wordB, "%c", x);
                    	word = 1;
                	}
                	else if(word == 1)
                    	sprintf(wordB, "%s%c", wordB, x);
            	}
            	else if(isdigit(x))
            	{
                	if(word == 0)
                	{
                    	wordB = malloc(sizeof(char) * 250);
                    	sprintf(wordB, "%c", x);
                    	word = 1;
                	}
                	else if(word == 1)
                    	sprintf(wordB, "%s%c", wordB, x);
            	}
            	else
            	{
                	if(word == 1)
                	{
                    	TrieInsert(root, wordB, head->name);
                    	word = 0;
                    	free(wordB);
                	}
            	}
        	}while(x != EOF);
        	head = head->next;
    	}
    	result(fp, root);
    }
    fprintf(fp, "</fileindex>\n");
    fclose(fp);
    freeNode(a);
    freeNode(b);
    return 0;
}
