#include "routing.h"

int id, port, sock, neigh_qtty = 0;
int neigh_list[NROUT];
char adress[MAX_ADRESS];
struct sockaddr_in si_me; //Endereço do roteador
neighbour_t neigh_info[NROUT];
dist_t routing_table[NROUT][NROUT];

int main(int argc, char *argv[]){
  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id, confira o arquivo de configuração");
  initialize(id, &port, &sock, adress, &si_me, neigh_list, neigh_info, &neigh_qtty, routing_table);


  info(id, port, adress, neigh_qtty, neigh_list, neigh_info, routing_table);
  //printf("%d\n", sock);
  while(1);
  close(sock);


  return 0;
}
