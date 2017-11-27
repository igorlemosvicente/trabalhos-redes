#ifndef ROUTING_H
#define ROUTING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_ADRESS 50 //Tamanho máximo de um endereço
#define NROUT 4 //Numero de Roteadores
#define INF 112345678 //Infinito =~ 10^8

//Estrutura de vizinho
typedef struct{
  int id, cost, port;
  char adress[MAX_ADRESS];
} neighbour_t;

//Estrutura de distância, para o vetor de distâncias
typedef struct{
  int dist, nhop;
}dist_t;

void die(char* msg);
int toint(char *str);
void initialize(int id, int *port, int *sock, char adress[MAX_ADRESS], struct sockaddr_in *si_me, int neigh_list[NROUT],
                neighbour_t neigh_info[NROUT], int *neigh_qtty, dist_t routing_table[NROUT][NROUT]);
void info(int id, int port, char adress[MAX_ADRESS], int neigh_qtty, int neigh_list[NROUT],
                neighbour_t neigh_info[NROUT], dist_t routing_table[NROUT][NROUT]);
#endif
