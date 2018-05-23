#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define INVALID_FILE_MODE 9998
#define unrestricted 0
#define exclusive    1
#define transaction  2

int isExclusive(char *pathName);
void initList(void *d);
void insertHead(void *data);
void deleteTail();
int queue_size();
void check();
void *first_element();
void *checkQueue();
void createThreads(void *data);
void errorChange(char *e);
void *initSocket(void *d);
char **rTokens(char *c);
int simulateNetopen(char**t);
int recieveRequest();
void *decodeRequest(void *data);
int simulateNetread(char **t, char *b);
void sendE(void *d);
int checkCanOpen(char *p, int d, int x);
int getFileIndexFromFileDescp();
int isTransaction(char *pathName);
pthread_t queueChecker;
static int portNumber = 8052;
short socketInitalized = 0;
int totalThreads = 0;
int totalFiles = 0;
int serverSocketFildes;
struct sockaddr_in serverAdressInfo;
pthread_t *threadHolder;
extern int h_errno;
extern int errno;

typedef struct threadArg
{
  int clientStructLength;
  int clientSocketFildes;
  struct sockaddr_in clientAdressInfo;
  char buffer[512];
  int fileIndex;
}arg;

typedef struct fileInfo
{
  int flag;
  int mode;
  int fileDescriptor;
  char * fileName;
}filesInfo;

filesInfo *fileInfo;

struct node
{
    arg *thread;
    struct node *link;
}*front, *rear;

void insertHead(void *data)
{
    struct node *ptr;
    arg *x = data;
    ptr = (struct node*)malloc(sizeof(struct node));
    ptr->thread = x;
    ptr->link = NULL;
    if(rear  ==  NULL || front == NULL)
        front = rear = ptr;
    else
    {
        rear->link = ptr;
        rear = ptr;
    }    
}

void deleteTail()
{
    struct node *ptr;
    ptr = front;
    if(front == NULL)
    {
        printf("Queue is Empty\n");
        front = rear = NULL;
  }
    else
        front = front->link;
    free(ptr);
    return;
}

void* first_element()
{
    if(front == NULL)
        return NULL;
    else
        return front->thread;
    return NULL;
}

int queue_size()
{
    struct node *ptr;
    ptr = front;
    int x = 0;
    if(front  ==  NULL)
        printf("Queue is empty \n");
    while(ptr)
    {
        ptr = ptr->link;
        x++;
    }
    return x;
}

void sendE(void *data)
{
  arg *ptr = data;
  int eLength = snprintf(NULL, 0, "%d", errno);
  char *buffer = calloc(1, sizeof(char) *eLength + 3);
  buffer[0] = 'e';
  buffer[1] = ';';
  sprintf(& buffer[2], "%d", errno);
  buffer[2 + eLength] = '\0';
  if(errno != INVALID_FILE_MODE)
      printf("%s\n", "Error -> Sent To Client");
  if(send(ptr-> clientSocketFildes, buffer, strlen(buffer), 0) < 0)
      errorChange("ERROR: FAILED TO SEND ERROR MESSAGE TO CLIENT");
    return;
}

void errorChange(char *errorMessage)
{
    perror(errorMessage);
    printf("ERROR #: %d\n", errno);
    return;
}

void *initSocket(void *data)
{
    struct sockaddr_in cli_addr;
    int clientLol;
    serverSocketFildes = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocketFildes < 0)
    {
      errorChange("ERROR: SOCKET CONNECTION FAILED");
      return NULL;
    }
    bzero((char *) & serverAdressInfo, sizeof(serverAdressInfo));
    serverAdressInfo.sin_port = htons(portNumber);
    serverAdressInfo.sin_family = AF_INET;
    serverAdressInfo.sin_addr.s_addr = INADDR_ANY;
    if(bind(serverSocketFildes, (struct sockaddr * ) & serverAdressInfo, sizeof(serverAdressInfo)) < 0)
    {
      errorChange("ERROR: ERROR ON BINDING");
      return NULL;
    }
    listen(serverSocketFildes, 5);
    socketInitalized = 1;
    printf("%s\n", "SOCKET INITALIZATION COMPLETE");
    totalThreads++;
    int newsockfd;
    while(1)
    {
      clientLol = sizeof(cli_addr);
      newsockfd = accept(serverSocketFildes, (struct sockaddr *) & cli_addr, & clientLol);
      if(newsockfd < 0)
      {
          perror("ERROR: CAUSED ON ACCEPT");
          exit(1);
      }
      threadHolder =  realloc(threadHolder, sizeof(pthread_t *) *(totalThreads + 1));
      arg *ptr = calloc(1, sizeof(arg));
      ptr-> clientStructLength = clientLol;
      ptr-> clientSocketFildes = newsockfd;
      bzero(ptr-> buffer, 512);
      int cc = recv(ptr-> clientSocketFildes, ptr-> buffer, sizeof(ptr-> buffer), 0);
      if(cc < 0)
      {
          errorChange("ERROR: FAILED TO RECEIVE FROM SERVER");
          free(ptr);
          return NULL;
      }
      pthread_create( & threadHolder[totalThreads], 0, & decodeRequest, ptr);
      pthread_join(threadHolder[totalThreads], NULL);
      totalThreads++;
    }
    return NULL;
}

char **rTokens(char *b)
{
    char **tokens;
    char *buffer = strdup(b);
    if(buffer[0] == '2')
    {
      char *first = strtok(buffer, ";");
      char *second = strtok(NULL, ";");
      char *third = strtok(NULL, ";");
      tokens = malloc(sizeof(char * ) * 3);
      tokens[0] = first;
      tokens[1] = second;
      tokens[2] = third;
  }
    else if(buffer[0] == '1' || buffer[0] == '3')
    {
      char *first = strtok(buffer, ";");
      char *second = strtok(NULL, ";");
      char *third = strtok(NULL, ";");
      char *fourth = strtok(NULL, ";");
      tokens = malloc(sizeof(char * ) * 4);
      tokens[0] = first;
      tokens[1] = second;
      tokens[2] = third;
      tokens[3] = fourth;
    } 
    else if(buffer[0] == '4')
    {
      char *first = strtok(buffer, ";");
      char *second = strtok(NULL, ";");
      tokens = malloc(sizeof(char * ) * 4);
      tokens[0] = first;
      tokens[1] = second;
    }
    return tokens;
}

int isTransaction(char *pathName)
{
    int i;
    for (i = 0; i < totalFiles; ++i)
    {
      if(strcmp(fileInfo[i].fileName, pathName) == 0)
      {
          if(fileInfo[i].fileDescriptor != -1)
          {
            if(fileInfo[i].mode != transaction)
              continue;
            else 
              return 1;
          }
          else 
            continue;
      }
    }
    return -1;
}

int checkFileOpen(char *pathName)
{
    int i;
    for(i = 0; i < totalFiles; ++i)
    {
      if(strcmp(fileInfo[i].fileName, pathName) == 0)
      {
          if(fileInfo[i].fileDescriptor != -1)
            return 1;
          else 
            continue;
      } 
      else 
        continue;
    }
    return -1;
}

int isExclusive(char *pathName)
{
    int i;
    for(i = 0; i < totalFiles; ++i)
    {
      if(strcmp(fileInfo[i].fileName, pathName) == 0)
      {
          if(fileInfo[i].fileDescriptor != -1)
          {
            if(fileInfo[i].mode == exclusive)
            {
              if(fileInfo[i].flag == O_RDWR || fileInfo[i].flag == O_WRONLY)
                  return 1;
                else
                  continue;
        }
        else
          continue;
      }
      else
        continue;
    }
    else
      continue;
  }
    return -1;
}

int checkUnrestrictedWrite(char *pathName)
{
    int i;
    for (i = 0; i < totalFiles; ++i)
    {
      if(strcmp(fileInfo[i].fileName, pathName) == 0)
      {
      if(fileInfo[i].fileDescriptor != -1)
      {
        if(fileInfo[i].mode == unrestricted)
        {
          if(fileInfo[i].flag == O_WRONLY || fileInfo[i].flag == O_RDWR)
            return 1;
            }
            else
              continue;
          }
          else
            continue;
    }
      else
        continue;
  }
    return -1;
}

int checkCanOpen(char *pathName, int flag, int mode)
{
    switch(mode)
    {
      case transaction:
        if(checkFileOpen(pathName) == -1)
          return 1;
        else
          return -2;
      case exclusive:
        if(isTransaction(pathName) == -1)
        {
            if(flag == O_RDONLY)
              return 1;
            if(isExclusive(pathName) == -1)
            {
              if(checkUnrestrictedWrite(pathName) == -1)
                  return 1;
            return -1;
            }
            return -1;
      }
        return -1;
      case unrestricted:
        if(isTransaction(pathName) == -1)
        {
          if((isExclusive(pathName) == 1) && flag == O_RDONLY)
              return 1;
        if(isExclusive(pathName) == -1)
          return 1;
      }
        return 0;
    default:
        printf("%s\n", "Could not find mode");
        return -5;
  }
}

int simulateNetopen(char **tokens)
{
    char *pathName = tokens[1];
    char *flags = tokens[2];
    char *modes = tokens[3];
    int flag = atoi(flags);
    int mode = atoi(modes);
    int fileDescriptor;
    if(totalFiles == 0)
    {
      fileInfo = malloc(sizeof(filesInfo) * totalFiles + 1);
      fileDescriptor = open(pathName, flag, mode);
      if(fileDescriptor < 0)
      {
          errorChange("Error while opening file");
          return -1;
      }
      fileInfo[totalFiles].fileName = malloc(strlen(tokens[1] + 1));
      fileInfo[totalFiles].fileName = strcpy( & fileInfo[totalFiles].fileName[0], tokens[1]);
      fileInfo[totalFiles].fileName[strlen(tokens[1])] = '\0';
      fileInfo[totalFiles].fileDescriptor = fileDescriptor;
      fileInfo[totalFiles].mode = mode;
      fileInfo[totalFiles].flag = flag;
      totalFiles++;
      return fileDescriptor;
    }
    if(totalFiles > 0)
    {
      int x = checkCanOpen(pathName, flag, mode);
      if(x == 1)
      {
          fileInfo = (filesInfo * ) realloc(fileInfo, sizeof(filesInfo) * (totalFiles + 1));
          fileDescriptor = open(pathName, flag, mode);
          if(fileDescriptor < 0)
          {
            errorChange("Error while openning file");
            return -1;
          }
          fileInfo[totalFiles].fileName = malloc(strlen(tokens[1] + 1));
          fileInfo[totalFiles].fileName = strcpy( & fileInfo[totalFiles].fileName[0], tokens[1]);
          fileInfo[totalFiles].fileName[strlen(tokens[1])] = '\0';
          fileInfo[totalFiles].fileDescriptor = fileDescriptor;
          fileInfo[totalFiles].mode = mode;
          fileInfo[totalFiles].flag = flag;
          totalFiles++;
    }
      if(x < 1)
      {
          errno = INVALID_FILE_MODE;
          return -1;
      }
    }
  return fileDescriptor;
}

int simulateNetread(char **tokens, char *buffer)
{
    int fileDescriptor = atoi(tokens[1]);
    int bytesToRead = atoi(tokens[2]);
    lseek(fileDescriptor, 0, SEEK_SET);
    int x = read(fileDescriptor, buffer, bytesToRead);
    if (x < 0)
    {
      errorChange("Read failed from file handle");
      return -1;
    }
    printf("Number of bytes read : %d\n", x);
    return x;
}

int simulateNetwrite(char **tokens)
{
    int fileDescriptor = atoi(tokens[1]);
    char *b = tokens[2];
    char *buffer = strdup(b);
    int stringLength = strlen(buffer);
    int bytesToWrite = atoi(tokens[3]);
    char *tempBuffer = calloc(1, sizeof(char) *bytesToWrite);
    int i;
    for (i = 0; i < stringLength; ++i)
      tempBuffer[i] = buffer[i];
  for (i; i < bytesToWrite; i++) 
    tempBuffer[i] = ' ';
    int x = write(fileDescriptor, tempBuffer, bytesToWrite);
    if (x < 0)
    {
      errorChange("Read failed from file handle");
      free(tempBuffer);
      return -1;
    }
  printf("Number of bytes written : %d\n", x);
  free(tempBuffer);
    return x;
}

int simulateNetclose(char **tokens)
{
    int fileDescriptor = atoi(tokens[1]);
    int x = close(fileDescriptor);
    if(x < 0)
    {
      errorChange("Close Failed");
      if(errno == EBADF)
          return -1;
    }
    if(x == 0)
      return x;
}

int getFileIndexFromFileDescp(int fileDescriptor)
{
    int i;
    for(i = 0; i < totalFiles; ++i)
    {
      if(fileDescriptor == fileInfo[i].fileDescriptor)
        return i;
    }
    return -1;
}
void *decodeRequest(void *data)
{
    arg *ptr = data;
    if(ptr-> buffer[0] == '1')
    {
      char **tokens = rTokens(ptr-> buffer);
      int x = simulateNetopen(tokens);
      free(tokens);
      int returnFileDescriptor = htonl(x);
      write(ptr-> clientSocketFildes, & returnFileDescriptor, sizeof(returnFileDescriptor));
      if(x > 0)
    {
          close(ptr-> clientSocketFildes);
          free(ptr);
          pthread_exit(NULL);
      }
      if (x < 0)
      {
          if(errno == INVALID_FILE_MODE)
          {
            initList(ptr);
            sendE(ptr);
            pthread_exit(NULL);
          } 
          else if(errno == EACCES || errno == EINTR || errno == EISDIR || errno == ENOENT || errno == EROFS)
            sendE(ptr);
          close(ptr-> clientSocketFildes);
          free(ptr);
          pthread_exit(NULL);
      }
    } 
    else if(ptr-> buffer[0] == '2')
    {
      char *buffer;
      char **tokens = rTokens(ptr-> buffer);
      int fsize = lseek(atoi(tokens[1]), 0, SEEK_END);
      buffer = (char * )(calloc(1, sizeof(char) * fsize));
      int x = simulateNetread(tokens, buffer);
      int l = snprintf(NULL, 0, "%d", x);
      char *tempBuffer = calloc(1, sizeof(char) * (x + 2 + 1 + l + 1));
      tempBuffer[0] = '2';
      tempBuffer[1] = ';';
      int i;
      for(i = 0; i < strlen(buffer); ++i) 
        tempBuffer[2 + i] = buffer[i];
      tempBuffer[2 + i] = ';';
      sprintf( & tempBuffer[3 + i], "%d", x);
      tempBuffer[3 + i + l] = '\0';
      if(send(ptr-> clientSocketFildes, tempBuffer, strlen(tempBuffer), 0) < 0)
          errorChange("CLIENT BUFFER NOT FOUND");
      if(x < 0) 
      {
          if(errno == ETIMEDOUT || errno == EBADF || errno == ECONNRESET) 
            sendE(ptr);
      }
      free(buffer);
      free(tempBuffer);
      free(tokens);
      close(ptr-> clientSocketFildes);
      free(ptr);
      pthread_exit(NULL);
    } 
    else if (ptr-> buffer[0] == '3') 
    {
      char **tokens = rTokens(ptr-> buffer);
      int x = simulateNetwrite(tokens);
      int returnBytesWritten = htonl(x);
      write(ptr-> clientSocketFildes, & returnBytesWritten, sizeof(returnBytesWritten));
      if(x < 0) 
      {
          if(errno == ETIMEDOUT || errno == EBADF || errno == ECONNRESET) 
            sendE(ptr);
      }
      free(tokens);
      close(ptr-> clientSocketFildes);
      free(ptr);
      pthread_exit(NULL);
    } 
    else if (ptr-> buffer[0] == '4')
    {
      char **tokens = rTokens(ptr-> buffer);
      int x = simulateNetclose(tokens);
      int returnCloseFile = htonl(x);
      write(ptr-> clientSocketFildes, & returnCloseFile, sizeof(returnCloseFile));
      if(x < 0) 
      {
        if(errno == EBADF) 
          sendE(ptr);
      } 
      else 
      {
          int getFileIndex = getFileIndexFromFileDescp(atoi(tokens[1]));
          if(getFileIndex < 0)
            errorChange("FILE INDEX NOT FOUND");
          else 
          {
            printf("FILE INDEX AT : %d CLOSED \n", getFileIndex);
            fileInfo[getFileIndex].fileDescriptor = -1;
          }
      }
      free(tokens);
      close(ptr-> clientSocketFildes);
      free(ptr);
      pthread_create( & queueChecker, 0, & checkQueue, "");
      pthread_join(queueChecker, NULL);
      pthread_exit(NULL);
    } 
    else 
    {
      errorChange("ERROR: FUNCTION TO RUN NOT FOUND");
      return NULL;
    }
}

void initList(void * data)
{
    arg *ptr = data;
    insertHead(ptr);
    printf("%s %d\n", "INSERTED INTO LIST, POSITION: ", queue_size());
    return;
}

void *checkQueue()
{
    while (queue_size() > 0)
    {
      arg *ptr = first_element();
      char **tokens = rTokens(ptr-> buffer);
      char *pathname = tokens[1];
      int flag = atoi(tokens[2]);
      int mode = atoi(tokens[3]);
      if(checkCanOpen(pathname, flag, mode) == 1)
      {
        int x = simulateNetopen(tokens);
          int returnFileDescriptor = htonl(x);
          write(ptr-> clientSocketFildes, & returnFileDescriptor, sizeof(returnFileDescriptor));
          deleteTail();
          close(ptr-> clientSocketFildes);
          free(ptr);
      } 
      else 
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char * argv[])
{
    threadHolder = (pthread_t * )(malloc(sizeof(pthread_t) *1));
    pthread_create( & threadHolder[totalThreads], 0, & initSocket, "");
    pthread_join(threadHolder[totalThreads], NULL);
    return 0;
}

