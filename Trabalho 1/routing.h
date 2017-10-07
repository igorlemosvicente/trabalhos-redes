#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"

#define leftson(x) 2 * (x) //Filho a esquerda
#define rightson(x) 2 * (x) + 1 //Filho a direita
#define dad(x) (x)/2 //Pai
#define valido(x, last) (x) <= last //Verifica se posicao e valida

typedef struct{ // Cada uma das linhas da tabela de roteamento
  int nhope, dist;
} hope_t;

typedef struct{ // Estrutura de aresta, contendo destino e distancia, para o dijkstra
  int u, v, dist;
} edge_t;

typedef struct{
  int id, port;
  char adress[30];
} router_t;

typedef struct{
  int orig, dest, num;
  char msg[MESSAGE_SIZE+1];
} message_t;

void initialize(int id, int *sock, struct sockaddr_in *si_me, router_t routers[NROUT], hope_t routing_table[NROUT]);
void dijkstra(int id, int graph[NROUT][NROUT], hope_t routing_table[NROUT]);
void die(char* msg);
int toint(char *str);
void print_graph(int graph[NROUT][NROUT]);
void print_routing_table(hope_t routing_table[NROUT]);
void insert(edge_t e, int last, edge_t heap[]);
edge_t extract(int target, edge_t heap[], int last);
void checaHeapFilho(int i, edge_t heap[], int last);
int checaHeapPai(int i, edge_t heap[]);
