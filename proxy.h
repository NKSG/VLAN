#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>


#define true 1
#define false 0

typedef int boolean; 
typedef struct sockaddr_in sockaddr_in; 
typedef struct sockaddr sockaddr;

int Server(char **argv);
int Client(char **argv); 
int printHelp(); 
