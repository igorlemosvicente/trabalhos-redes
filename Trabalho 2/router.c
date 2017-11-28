#include "routing.h"

int id, port, sock, neigh_qtty = 0; //Id, porta, socket e numero de vizinhos do roteador
int neigh_list[NROUT]; //Lista de vizinhos do roteador
char adress[MAX_ADRESS]; //Endereço do roteador (ip)
struct sockaddr_in si_me, si_send; //Estrutura de endereço do roteador e outro usado para envios
int slen = sizeof(si_me); //Tamanho do endereço
neighbour_t neigh_info[NROUT]; //Informacoes dos vizinhos (custo, porta, endereco)
dist_t routing_table[NROUT][NROUT]; //Tabela de roteamento do nó
pack_queue_t in, out; //Filas de entrada e saida de pacotes
pthread_t sender_id, receiver_id, unpacker_id; // Threads

void* sender(void *nothing); //Thread responsavel por enviar pacotes
void* receiver(void *nothing); //Thread responsavel por receber pacotes
void* unpacker(void *nothing); //Thread responsavel por desembrulhar pacotes e trata-los

int main(int argc, char *argv[]){
  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id, confira o arquivo de configuração");

  //Rotina de inicializacao do roteador
  initialize(id, &port, &sock, adress, &si_me, &si_send, neigh_list, neigh_info, &neigh_qtty, routing_table, &in, &out);

  info(id, port, adress, neigh_qtty, neigh_list, neigh_info, routing_table);

  pthread_create(&sender_id, NULL, sender, NULL); //Cria thread enviadora
  pthread_create(&receiver_id, NULL, receiver, NULL); //Cria thread receptora
  pthread_create(&unpacker_id, NULL, unpacker, NULL); //Cria thread desempacotadora

  while(1);
  close(sock);

  return 0;
}

void* sender(void *nothing){
  printf("[SENDER] Enviador iniciado!\n");
  while(1){
    pthread_mutex_lock(&out.mutex);
    while(out.begin != out.end){
      printf("[SENDER] Pacote de %s sendo processado para o nó %d\n",out.queue[out.begin].control ? "controle" : "dados",
            out.queue[out.begin].dest);
      package_t *pck = &(out.queue[out.begin++]); //Aponta o pacote a ser enviado
      int new_dest = routing_table[id][pck->dest].nhop; //Pega o próximo destino (next hop)
      si_send.sin_port = htons(neigh_info[new_dest].port); //Atribui a porta do pacote a ser enviado
      if (inet_aton(neigh_info[new_dest].adress , &si_send.sin_addr) == 0)
        printf("[SENDER] Falha ao obter endereco do destinatario\n");
      else{
        //Envia para o socket requisitado(socket, dados, tamanho dos dados, flags, endereço, tamanho do endereço)
        if (sendto(sock, pck, sizeof(*pck), 0, (struct sockaddr*) &si_send, slen) == -1)
          printf("\n[SENDER] Falha ao enviar pacote...\n");
        else printf("[SENDER] Pacote enviado com sucesso!\n");
      }
    }
    pthread_mutex_unlock(&out.mutex);
    //printf("[SENDER] Agora, um soninho...\n");
    //sleep(INF);
  }
}

void* unpacker(void *nothing){
  int i, changed, retransmit, new_min;
  package_t *pck;

  printf("[UNPACKER] Desempacotador iniciado!\n");
  while(1){
    pthread_mutex_lock(&in.mutex);
    while(in.begin != in.end){
      pck = &in.queue[in.begin];
      if(pck->dest == id){
        if(pck->control){
          printf("[UNPACKER] Processando pacote de controle\n");
          for(i = changed = 0; i < NROUT; i++){
            //Se o vetor de distancias que o no enviou eh diferente do o no possui, atualiza
            if(routing_table[pck->orig][i].dist != pck->dist_vector[i].dist ||
               routing_table[pck->orig][i].nhop != pck->dist_vector[i].nhop){
              routing_table[pck->orig][i].dist = pck->dist_vector[i].dist;
              routing_table[pck->orig][i].nhop = pck->dist_vector[i].nhop;
              changed = 1;
              //Se a distancia ate o destino, mais o custo ate o no for maior o que ja tem, relaxa
              if(pck->dist_vector[i].dist + neigh_info[pck->orig].cost < routing_table[id][i].dist){
                routing_table[id][i].dist = pck->dist_vector[i].dist + neigh_info[pck->orig].cost;
                routing_table[id][i].nhop = pck->orig;
              }
            }
          }
          if(changed){
            info(id, port, adress, neigh_qtty, neigh_list, neigh_info, routing_table);

          }
        }
      }
      else{

      }
      in.begin++;
    }
    pthread_mutex_unlock(&in.mutex);
  }
}

void* receiver(void *nothing){
  package_t received;

  while(1){
    if ((recvfrom(sock, &received, sizeof(received), 0, (struct sockaddr *) &si_me,
        (socklen_t * restrict ) &slen)) == -1)
        printf("[RECEIVER] Erro ao receber pacote\n");
    else{
      printf("[RECEIVER] Pacote recebido de %d\n", received.orig);
      pthread_mutex_lock(&in.mutex);
      copy_package(&received, &in.queue[in.end++]); //Coloca o pacote no final da fila de recebidos
      pthread_mutex_unlock(&in.mutex);
      printf("%d %d\n", in.begin, in.end);
    }
  }
}
