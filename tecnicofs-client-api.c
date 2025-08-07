#include "tecnicofs-client-api.h"
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


int tfsCreate(char *filename, char nodeType) {
  char message[strlen(filename)+5];
  strcpy(message, "c ");
  strcat(message, filename);
  strcat(message, " ");
  strcat(message, &nodeType); 

  printf("%s\n", message);

  if (sendto(sockfd, message, strlen(message)+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  } 
  printf("Recebeu resposta do servidor: %s\n", buffer);

  return 0;
}

int tfsDelete(char *path) {
  char message[strlen(path) + 3];
  strcpy(message, "d ");
  strcat(message, path);

  if (sendto(sockfd, message, strlen(message)+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  } 
  printf("Recebeu resposta do servidor: %s\n", buffer);

  return 0;
}

int tfsMove(char *from, char *to) {
  char message[strlen(from) + strlen(to) + 3];
  strcpy(message, "m ");
  strcat(message, from);
  strcat(message, " ");
  strcat(message, to);

  if (sendto(sockfd, message, strlen(message)+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  } 
  printf("Recebeu resposta do servidor: %s\n", buffer);

  return 0;
}

int tfsLookup(char *path) {
  char message[strlen(path) + 3];
  strcpy(message, "l ");
  strcat(message, path);

  if (sendto(sockfd, message, strlen(message)+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  } 
  printf("Recebeu resposta do servidor: %s\n", buffer);

  return 0;
}

int tfsPrintTree(char *outputName){
  char message[strlen(outputName)+ 3];
  strcpy(message, "p ");
  strcat(message, outputName);
  
  if (sendto(sockfd, message, strlen(message)+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  } 
  printf("Recebeu resposta do servidor: %s\n", buffer);

  return 0;
}

int tfsMount(char * sockPath) {

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0) ) < 0) {
    perror("client: can't open socket");
    exit(EXIT_FAILURE);
    }

    unlink("/tmp/cliente");

    clilen = setSockAddrUn ("/tmp/cliente", &client_addr);
    servlen = setSockAddrUn(sockPath, &serv_addr);

    if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
    perror("client: bind error");
    exit(EXIT_FAILURE);
  }  
    return 0;
}

int tfsUnmount() {
	  close(sockfd);
    unlink("/tmp/cliente");
    return 0;
}

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}