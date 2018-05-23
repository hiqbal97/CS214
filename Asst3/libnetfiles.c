#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>

struct sockaddr_in info;
struct hostent *IP;
char *modes;
char *host;
char buffer[256];
int mode = -1;
int socketmode = -1;
int port = -1;
int x = -1;

int netserverinit(char *name, int filemode)
{
    if(filemode > 2 || filemode < 1)
    {
        printf("ERROR: Not a valid mode\n");
        return -1;
    }
    mode = filemode;
    IP = gethostbyname(name);
    if(IP == NULL)
    {
        printf("ERROR: Not a valid IP address\n");
        return -1;
    }
    socketmode = socket(AF_INET, SOCK_STREAM, 0);
    if (socketmode < 0)
    {
        printf("ERROR: Not valid");
        return -1;
    }
    memset((char *) &info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    memcpy((char *)IP->h_addr, (char *)&info.sin_addr.s_addr, IP->h_length);
    if(connect(socketmode,(struct sockaddr *)&info,sizeof(info)) < 0)
    {
        printf("ERROR: Connection failed\n");
        return -1;
    }
    return 0;
}

int netopen(const char *name, int y)
{
    memset(buffer, 0, 256);
    sprintf(buffer, "o-%s-%d-%d", name, y, mode);
    x = write(socketmode,buffer,strlen(buffer));
    if (x < 0)
    {
         perror("ERROR: Could not write to socket");
         exit(1);
    }
    memset(buffer, 0, 256);
    x = read(socketmode,buffer,strlen(buffer)-1);
    char *tok = strtok(buffer,"-");
    if(x < 0)
    {
        perror("ERROR: Could not read from socket");
        exit(1);
    }
    else if(!strcmp(tok,"error"))
    {
        errno = atoi(strtok(NULL, "-"));
        return -1;
    }
    return atoi(tok);
}

int netread(int destination, void *ptr, size_t y)
{
    memset(buffer, 0, 256);
    sprintf(buffer, "r,%d,%lu", destination, y);
    x = write(socketmode,buffer,strlen(buffer));
    if (x < 0)
    {
         perror("ERROR: Could not write to socket");
         exit(1);
    }
    memset(buffer, 0, 256);
    x = read(socketmode,buffer,strlen(buffer)-1);
    char *tok = strtok(buffer,"-");
    if(x < 0)
    {
        perror("ERROR: Could not read from socket");
        exit(1);
    }
    else if(!strcmp(tok,"error"))
    {
        errno = atoi(strtok(NULL, "-"));
        return -1;
    }
    else if (!strcmp(tok,"0"))
        return 0;
    strcpy(ptr,tok);
    tok = strtok(NULL, "-");
    return atoi(tok);
}

int netwrite(int destination, void *ptr, size_t y)
{
    memset(buffer, 0, 256);
    sprintf(buffer, "w,%d,%lu,%s", destination, y, (char *) ptr);
    x = write(socketmode,buffer,strlen(buffer));
    if (x < 0)
    {
         perror("ERROR: Could not write to socket");
         exit(1);
    }
    memset(buffer, 0, 256);
    x = read(socketmode,buffer,strlen(buffer)-1);
    char *tok = strtok(buffer,"-");
    if(x < 0)
    {
        perror("ERROR: Could not read from socket");
        exit(1);
    }
    else if(!strcmp(tok,"error"))
    {
        errno = atoi(strtok(NULL, "-"));
        return -1;
    }
    return atoi(tok);
}

int netclose(int y)
{
    memset(buffer, 0, 256);
    sprintf(buffer, "c,%d", y);
    x = write(socketmode,buffer,strlen(buffer));
    if (x < 0)
    {
         perror("ERROR: Could not write to socket");
         exit(1);
    }
    memset(buffer, 0, 256);
    x = read(socketmode,buffer,strlen(buffer)-1);
    char *tok = strtok(buffer,"-");
    if(x < 0)
    {
        perror("ERROR: Could not read from socket");
        exit(1);
    }
    else if(!strcmp(tok,"error"))
    {
        errno = atoi(strtok(NULL, "-"));
        return -1;
    }
    return 0;
}


int main(int argc, char* argv[])
{
    host = argv[1];
    modes = argv[2];
    if(argc != 3)
        perror("Usage ./executableFileName HOSTNAME connectionMode");
    mode = -1;
    if(strcmp(modes, "unrestricted") == 0) mode = 0;
    if(strcmp(modes, "exclusive") == 0) mode = 1;
    if(strcmp(modes, "transaction") == 0) mode = 2;
    if(mode == -1)
    { 
        printf("The mode was entered incorrectly\n");
        return -1; 
    }
    if(netserverinit(host, mode) == -1)
    {
        perror("ERROR: No hostname found");
        return -1;
    }
    int k;
    char* path = "/.autofs/ilab/ilab_users/hi60/Desktop/Asst3";
    char* x = malloc(10000);
    int write;
    int read;
    int destination = netopen(path, O_RDWR);
    if(destination < 0) return -1;
    k = destination;
    printf("File: %d\n", k);
    read = netread(k,x,10000);
    printf("%s\n", x);
    printf("Read: %d\n", read);
    write = netwrite(k, "Enter text", 30);
    printf("Write: %d\n", write);
    read = netread(k,x,10000);
    printf("Read: %d\n", read);
    write = netwrite(k, "Enter text", 30);
    printf("Write: %d\n", write);
    read = netread(k,x,10000);
    printf("Read: %d\n", read);
    sleep(30);
    netclose(k);
    return 0;
}

