#include "routing.h"

int id, port, sock, new = 0, neigh_qtty = 0; //Id, porta, socket e numero de vizinhos do roteador
int neigh_list[NROUT]; //Lista de vizinhos do roteador
char adress[MAX_ADRESS]; //Endereço do roteador (ip)
struct sockaddr_in si_me, si_send; //Estrutura de endereço do roteador e outro usado para envios
int slen = sizeof(si_me); //Tamanho do endereço
neighbour_t neigh_info[NROUT]; //Informacoes dos vizinhos (custo, porta, endereco)
dist_t routing_table[NROUT][NROUT]; //Tabela de roteamento do nó
pack_queue_t in, out; //Filas de entrada e saida de pacotes
pthread_t sender_id, receiver_id, unpacker_id, refresher_id; // Threads
pthread_mutex_t log_mutex;
FILE *logs, *messages;

void* sender(void *nothing); //Thread responsavel por enviar pacotes
void* receiver(void *nothing); //Thread responsavel por receber pacotes
void* unpacker(void *nothing); //Thread responsavel por desembrulhar pacotes e trata-los
void* refresher(void *nothing); //Thread responsavel por enfileirar periodicamente pacotes de distancia
                                //para todos os vizinhos

int main(int argc, char *argv[]){
  int op = -1;
  char log_path[20] = "./logs/log";
  char message_path[20] = "./messages/message";

  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id, confira o arquivo de configuração");

  //Adiciona id do roteador ao caminho dos arquivos de mensagem e log
  strcat(log_path, argv[1]);
  strcat(message_path, argv[1]);

  //Cria o arquivo para armazenar os logs
  if(!(logs = fopen(log_path, "w+"))) die("Falha ao criar arquivo de log");

  //Cria o arquivo para armazenar as mensagens
  if(!(messages = fopen(message_path, "w"))) die("Falha ao criar arquivo de mensagens");

  //Rotina de inicializacao do roteador
  initialize(id, &port, &sock, adress, &si_me, &si_send, neigh_list, neigh_info,
              &neigh_qtty, routing_table, &in, &out, &log_mutex);

  pthread_create(&sender_id, NULL, sender, NULL); //Cria thread enviadora
  pthread_create(&receiver_id, NULL, receiver, NULL); //Cria thread receptora
  pthread_create(&unpacker_id, NULL, unpacker, NULL); //Cria thread desempacotadora
  pthread_create(&refresher_id, NULL, refresher, NULL); //Cria thread atualizadora

  while(1){
    system("clear");
    if(op == -1){
      printf("ROTEADOR %d| %d novas mensagens\n", id, new);
      printf("------------------------------------------------------\n");
      printf("0 - Atualizar\n");
      printf("1 - Informações sobre o roteador\n");
      printf("2 - Log\n");
      printf("3 - Ler Mensagens\n");
      printf("4 - Escrever Mensagem\n");
      printf("5 - Sair\n\n");
      scanf("%d", &op);
    }
    else if(op == 1){
      info(id, port, adress, neigh_qtty, neigh_list, neigh_info, routing_table);
      printf("\nInsira 0 para voltar, 1 para atualizar\n");
      scanf("%d", &op);
      if(op == 0) op = -1;
      else op = 1;
    }
    else if(op == 2){
      print_log(logs, &log_mutex);
      printf("\nInsira 0 para voltar, 1 para atualizar\n");
      scanf("%d", &op);
      if(op == 0) op = -1;
      else op = 2;
    }
    else if(op == 5){
      system("clear");
      break;
    }
  }

  //Fecha sockets e arquivos
  close(sock);
  fclose(logs);
  fclose(messages);

  return 0;
}

void* sender(void *nothing){
  int new_dest;

  pthread_mutex_lock(&log_mutex);
  fprintf(logs, "[SENDER] Enviador iniciado!\n");
  pthread_mutex_unlock(&log_mutex);
  while(1){
    pthread_mutex_lock(&out.mutex);
    while(out.begin != out.end){
      package_t *pck = &(out.queue[out.begin]); //Aponta o pacote a ser enviado

      //TO CHECK
      new_dest = routing_table[id][pck->dest].nhop; //Pega o próximo destino (next hop)

      si_send.sin_port = htons(neigh_info[new_dest].port); //Atribui a porta do pacote a ser enviado
      if (inet_aton(neigh_info[new_dest].adress , &si_send.sin_addr) == 0){
        pthread_mutex_lock(&log_mutex);
        fprintf(logs, "[SENDER] Falha ao obter endereco do destinatario\n");
        pthread_mutex_unlock(&log_mutex);
      }
      else{
        //Envia para o socket requisitado(socket, dados, tamanho dos dados, flags, endereço, tamanho do endereço)
        if (sendto(sock, pck, sizeof(*pck), 0, (struct sockaddr*) &si_send, slen) == -1){
          pthread_mutex_lock(&log_mutex);
          fprintf(logs, "[SENDER] Falha ao enviar Pacote de %s ao nó %d\n"
                  ,out.queue[out.begin].control ? "controle" : "dados", out.queue[out.begin].dest);
          pthread_mutex_unlock(&log_mutex);
        }
        else{
          pthread_mutex_lock(&log_mutex);
          fprintf(logs, "[SENDER] Pacote de %s enviado com sucesso para o nó %d\n"
                  ,out.queue[out.begin].control ? "controle" : "dados", out.queue[out.begin].dest);
          pthread_mutex_unlock(&log_mutex);
        }
      }
      out.begin++;
    }
    pthread_mutex_unlock(&out.mutex);
  }
}

void* unpacker(void *nothing){
  int i, retransmit, changed;
  package_t *pck;

  pthread_mutex_lock(&log_mutex);
  fprintf(logs, "[UNPACKER] Desempacotador iniciado!\n");
  pthread_mutex_unlock(&log_mutex);

  while(1){
    pthread_mutex_lock(&in.mutex);
    while(in.begin != in.end){
      pck = &in.queue[in.begin];
      if(pck->dest == id){ //Se o pacote é pra mim
        if(pck->control){
          pthread_mutex_lock(&log_mutex);
          fprintf(logs, "[UNPACKER] Processando pacote de controle vindo de %d\n", pck->orig);
          pthread_mutex_unlock(&log_mutex);
          for(i = retransmit = changed = 0; i < NROUT; i++){
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
                retransmit = 1;
              }
            }
          }
          if(changed){
            pthread_mutex_lock(&log_mutex);
            fprintf(logs, "[UNPACKER] A tabela foi atualizada:\n");
            print_rout_table(routing_table, logs, 1);
            pthread_mutex_unlock(&log_mutex);
          }
          if(retransmit){
            pthread_mutex_lock(&log_mutex);
            fprintf(logs, "[UNPACKER] O vetor de distancias mudou, enfileirando atualização pros vizinhos.\n");
            queue_dist_vec(&out, neigh_list, routing_table, id, neigh_qtty);
            pthread_mutex_unlock(&log_mutex);
          }
        }
      }
      else{ //Se não é pra mim
        pthread_mutex_lock(&log_mutex);
        fprintf(logs, "[UNPACKER] Roteando pacote com origem %d, para %d, via %d\n", pck->orig, pck->dest, routing_table[id][pck->dest].nhop);
        pthread_mutex_unlock(&log_mutex);
        pthread_mutex_lock(&out.mutex);
        copy_package(pck, &out.queue[out.end++]); //Enfilero ele na fila de saida
        pthread_mutex_unlock(&out.mutex);
      }
      in.begin++;
    }
    pthread_mutex_unlock(&in.mutex);
  }
}

void* receiver(void *nothing){
  package_t received;

  pthread_mutex_lock(&log_mutex);
  fprintf(logs, "[RECEIVER] Receptor iniciado!\n");
  pthread_mutex_unlock(&log_mutex);

  while(1){
    if ((recvfrom(sock, &received, sizeof(received), 0, (struct sockaddr *) &si_me,
        (socklen_t * restrict ) &slen)) == -1)
        printf("[RECEIVER] Erro ao receber pacote\n");
    else{
      pthread_mutex_lock(&log_mutex);
      fprintf(logs, "[RECEIVER] Pacote recebido de %d\n", received.orig);
      pthread_mutex_unlock(&log_mutex);
      pthread_mutex_lock(&in.mutex);
      copy_package(&received, &in.queue[in.end++]); //Coloca o pacote no final da fila de recebidos
      pthread_mutex_unlock(&in.mutex);
      //print_pack_queue(&in);
    }
  }
}

void *refresher(void *nothing){
  while(1){
    fprintf(logs, "[REFRESHER] Enfileirando atualizações de vetor de distância\n");
    queue_dist_vec(&out, neigh_list, routing_table, id, neigh_qtty);
    sleep(REFRESH_TIME);
  }
}
