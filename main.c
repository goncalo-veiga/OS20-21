#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <strings.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100
#define INDIM 30
#define OUTDIM 512

socklen_t addrlen;
int sockfd;
pthread_mutex_t mutex;

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;


void pthread_mutex_unlock2(pthread_mutex_t* lock){
  if (pthread_mutex_unlock(lock) != 0) {
      printf("Error: couldn't unlock the thread.\n");
      exit(EXIT_FAILURE);
  }
}

void pthread_mutex_lock2(pthread_mutex_t* lock){
  if (pthread_mutex_lock(lock) != 0) {
      printf("Error: couldn't lock the thread.\n");
      exit(EXIT_FAILURE);
  }
}


int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

 void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}



void applyCommands(){
    while (1){

        struct sockaddr_un client_addr;
        char in_buffer[INDIM], out_buffer[OUTDIM];
        int c;

        addrlen=sizeof(struct sockaddr_un);
        c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
             (struct sockaddr *)&client_addr, &addrlen);

        if (c <= 0) {
            printf("Erro\n");
            continue;
        }
        
        //Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
        in_buffer[c]='\0';
        
        printf("Recebeu mensagem de %s\n", client_addr.sun_path);

        char token, type;
        char bigToken[MAX_INPUT_SIZE];
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(in_buffer, "%c %s %s", &token, name, bigToken);
        type = bigToken[0];

        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
                switch (type) {
                    case 'f':
                        pthread_mutex_lock2(&mutex);
                        printf("Create file: %s\n", name);
                        create(name, T_FILE);
                        pthread_mutex_unlock2(&mutex);
                        break;
                    case 'd':
                        pthread_mutex_lock2(&mutex);
                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);
                        pthread_mutex_unlock2(&mutex);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l': 
                pthread_mutex_lock2(&mutex);
                searchResult = lookup(name);
                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);
                pthread_mutex_unlock2(&mutex);
                break;
            case 'd':
                pthread_mutex_lock2(&mutex);
                printf("Delete: %s\n", name);
                delete(name);
                pthread_mutex_unlock2(&mutex);
                break;
            case 'p':
                pthread_mutex_lock2(&mutex);
                printf("Print tree to %s\n", name);
                FILE *fout;
                if ((fout = fopen(name, "w")) == NULL) {
                    printf("Error! opening file");
                    // Program exits if file pointer returns NULL.
                    exit(1);
                }
                print_tecnicofs_tree(fout);
                fclose(fout);
                pthread_mutex_unlock2(&mutex);
                break;

            case 'm':
                pthread_mutex_lock2(&mutex);
                printf("Move: %s to %s\n", name, bigToken);
                move(name,bigToken);
                pthread_mutex_unlock2(&mutex);
                break;

            default: { // error 
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
        c = sprintf(out_buffer, "%s\n", "Executou a operacao\n");
        
        sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *)&client_addr, addrlen);        

    }
}



int main(int argc, char* argv[]) {
    struct sockaddr_un server_addr;
    char *path;
    int i=0;
    int j=0;

    numberThreads = atoi(argv[1]);
    pthread_t tid[numberThreads];

    if(argv[1] == NULL || argv[2] == NULL || argc > 3){
        fprintf(stderr, "Error: invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    if (numberThreads < 1){
        fprintf(stderr, "Error: can't run with less than 1 thread\n");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open socket");
        exit(EXIT_FAILURE);
      }
    
    path = argv[2];
    strcat(path,"-g62");

    unlink(path);

    addrlen = setSockAddrUn (argv[2], &server_addr);
    if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
            perror("server: bind error");
            exit(EXIT_FAILURE);
        }

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Error: mutex lock didn't initialize properly.\n");
        exit(EXIT_FAILURE);
    }

    /* init filesystem */
    init_fs();
    
    while( i < numberThreads){

        if (pthread_create(&tid[i], 0, (void *)applyCommands, NULL) != 0){
            fprintf(stderr, "Error: couldn't create the thread properly\n");
            exit(EXIT_FAILURE);
        }
        i+=1;
    }

    while( j < numberThreads){

        if(pthread_join(tid[j], NULL) != 0){
            fprintf(stderr, "Error: couldn't join the thread properly\n");
            exit(EXIT_FAILURE);
        }
        j+=1;
    }


    //Fechar e apagar o nome do socket, apesar deste programa 
    //nunca chegar a este ponto
    close(sockfd);
    unlink(argv[1]);

    /* release allocated memory */
    destroy_fs();

    if (pthread_mutex_destroy(&mutex) != 0) {
        printf("Error: couldn't destroy the mutex lock.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
