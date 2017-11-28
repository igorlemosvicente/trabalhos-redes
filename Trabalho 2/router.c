#include "routing.h"

int id, port, sock, neigh_qtty = 0; //Id, porta, socket e numero de vizinhos do roteador
int neigh_list[NROUT]; //Lista de vizinhos do roteador
char adress[MAX_ADRESS]; //Endereço do roteador (ip)
struct sockaddr_in si_me, si_send; //Endereço do roteador e outro usado para envios
int slen = sizeof(si_me); //Tamanho do endereço
neighbour_t neigh_info[NROUT]; //Informacoes dos vizinhos (custo, porta, endereco)
dist_t routing_table[NROUT][NROUT]; //Tabela de roteamento do nó
pack_queue_t in, out; //Filas de entrada e saida de pacotes
pthread_t sender_id, receiver_id; // Threads

void* sender(void *nothing); //Thread responsavel por enviar pacotes
void* receiver(void *nothing); //Thread responsavel por receber pacotes

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

  while(1);
  close(sock);

  return 0;
}

void* sender(void *nothing){
  while(1){
    if(out.begin != out.end){
      pthread_mutex_lock(&out.mutex);
      while(out.begin != out.end){
        printf("Pacote sendo processado para o no %d\n", out.queue[out.begin].dest);
        package_t *pck = &(out.queue[out.begin]);
        si_send.sin_port = htons(neigh_info[pck->dest].port); //Atribui a porta do pacote a ser enviado
        if (inet_aton(neigh_info[pck->dest].adress , &si_send.sin_addr) == 0)
          printf("Falha ao obter endereco do destinatario\n");
        else{
          //Envia para o socket requisitado(socket, dados, tamanho dos dados, flags, endereço, tamanho do endereço)
          if (sendto(sock, pck, sizeof(*pck), 0, (struct sockaddr*) &si_send, slen) == -1)
            printf("\nFalha ao enviar pacote...\n");
          else printf("Um pacote foi enviado com sucesso!\n");
        }
        out.begin++;
      }
      pthread_mutex_unlock(&out.mutex);
    }
  }
}

void* receiver(void *nothing){
  package_t received;

  while(1){
    if ((recvfrom(sock, &received, sizeof(received), 0, (struct sockaddr *) &si_me,
        (socklen_t * restrict ) &slen)) == -1)
        printf("Erro ao receber mensagem\n");
    pthread_mutex_lock(&in.mutex);
    //else printf("Recebi um negocio\n");
    copy_package(&received, &in.queue[in.end++]); //Coloca o pacote no final da fila de recebidos
    pthread_mutex_lock(&out.mutex);
  }
}
