#ifndef ROUTING_H
#define ROUTING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define CLEAR_LOG 1 //1 para ocultar informações sobre pacotes de controle
#define REFRESH_TIME 2//Tempo entre os envios periodicos de vetor de distancia aos vizinhos
#define TOLERANCY 2 * REFRESH_TIME //Tempo até o roteador assumir que o vizinho caiu
#define MAX_QUEUE 1123456 //Tamanho máximo da fila =~ 1123456
#define MAX_MESSAGE 200 //Tamanho maximo da mensagem
#define MAX_ADRESS 50 //Tamanho máximo de um endereço
#define NROUT 4 //Numero de Roteadores
#define INF 112345678 //Infinito =~ 10^8

//Estrutura de vizinho
typedef struct{
  int id, orig_cost, cost, port, news;
  char adress[MAX_ADRESS];
} neighbour_t;

//Estrutura de distância, para o vetor de distâncias
typedef struct{
  int dist, nhop;
}dist_t;

//Estrutura de pacote
typedef struct{
  int control, dest, orig;
  char message[MAX_MESSAGE];
  dist_t dist_vector[NROUT];
}package_t;

//Fila de pacotes
typedef struct{
  pthread_mutex_t mutex;
  int begin, end;
  package_t queue[MAX_QUEUE];
}pack_queue_t;

void die(char* msg);
int toint(char *str);
void initialize(int id, int *port, int *sock, char adress[MAX_ADRESS], struct sockaddr_in *si_me,
                struct sockaddr_in *si_send, int neigh_list[NROUT], neighbour_t neigh_info[NROUT],
                int *neigh_qtty, dist_t routing_table[NROUT][NROUT], pack_queue_t *in, pack_queue_t *out,
                pthread_mutex_t *log_mutex, pthread_mutex_t *messages_mutex, pthread_mutex_t *news_mutex);
void info(int id, int port, char adress[MAX_ADRESS], int neigh_qtty, int neigh_list[NROUT],
                neighbour_t neigh_info[NROUT], dist_t routing_table[NROUT][NROUT]);
void copy_package(package_t *a, package_t *b);
void queue_dist_vec(pack_queue_t *out, int neigh_list[NROUT], dist_t routing_table[NROUT][NROUT],
                    int id, int neigh_qtty);
void print_pack_queue(pack_queue_t *queue);
void print_rout_table(dist_t routing_table[NROUT][NROUT], FILE *file, int infile);
void print_file(FILE *file, pthread_mutex_t *mutex);

#endif
