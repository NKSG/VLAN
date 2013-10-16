//from simpletun c slide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

/* buffer for reading from tun/tap interface, must be >= 1500 */
#define BUFSIZE 2024 
#define CLIENT 0
#define SERVER 1
#define PORT 55555
#define true 1
#define false 0

typedef int boolean; 
typedef struct sockaddr_in sockaddr_in; 
typedef struct sockaddr sockaddr;

int Server(char **argv);
int Client(char **argv); 
int printHelp(); 
