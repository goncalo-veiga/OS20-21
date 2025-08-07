#ifndef API_H
#define API_H

#include "tecnicofs-api-constants.h"
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <stdio.h>

FILE* inputFile;
char* serverName;
int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;
char buffer[1024];

int tfsCreate(char *path, char nodeType);
int tfsDelete(char *path);
int tfsLookup(char *path);
int tfsMove(char *from, char *to);
int tfsMount(char* serverName);
int tfsUnmount();
int tfsPrintTree(char *outputName);
int setSockAddrUn(char *path, struct sockaddr_un *addr);

#endif /* CLIENT_H */
