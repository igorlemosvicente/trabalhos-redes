#include "routing.h"

hope_t routing_table[NROUT]; //Tabela de roteamento
message_t messages[BOX_SIZE], tmps, tmpr; // Caixa de mensagens
router_t routers[NROUT]; //Informações sobre os roteadores
struct sockaddr_in si_me, si_send, si_rerout; //Endereço do roteador e o usado para envios
char logg[100];
pthread_t receiver_id;
int id, sock, i, new = 0, op = -1, snum = 0, qtmsg = 0, slen = sizeof(si_me);

void* receiver(void *nothing);

int main(int argc, char *argv[]){
  strcpy(logg, "Nada aconteceu por enquanto");
  //strcpy(logg, "Nada aconteceu por enquanto");

  //Trata erros no argumento
  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado\n");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado\n");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id");

  //Zera a estrutura para envios
  memset((char *) &si_send, 0, sizeof(si_send));
  memset((char *) &si_rerout, 0, sizeof(si_rerout));

  si_send.sin_family = si_rerout.sin_family = AF_INET; //Familia
  si_send.sin_addr.s_addr = si_rerout.sin_addr.s_addr =  htonl(INADDR_ANY); //Atribui o socket a todo tipo de interface
  initialize(id, &sock, &si_me, routers, routing_table); //Inicializa o roteador
  pthread_create(&receiver_id, NULL, receiver, NULL); //Cria a thread receptora
  //printf("\n%d %d %s\n", id, ntohs(si_me.sin_port), routers[id].adress);
  while(1){
    system("clear");
    if(op == -1){
      printf("ROTEADOR %d| %d novas mensagens\n", id, new);
      printf("LOG: %s\n", logg);
      printf("------------------------------------------------------\n");
      printf("0 - Atualizar\n");
      printf("1 - Checar Tabela de Roteamento\n");
      printf("2 - Ler Mensagens\n");
      printf("3 - Escrever Mensagem\n");
      printf("4 - Sair\n\n");
      scanf("%d", &op);
    }
    else if(op == 0) op = -1;
    else if(op == 1){
      print_routing_table(routing_table);
      printf("\nInsira 0 para voltar\n");
      scanf("%d", &op);
      if(op == 0) op = -1;
      else op = 1;
    }
    else if(op == 2){
      new = 0;
      for(i = 0; i < qtmsg; i++){
        printf("De %d: %s\n", messages[i].orig, messages[i].msg);
      }
      printf("\nInsira 0 para voltar\n");
      scanf("%d", &op);
      if(op == 0) op = -1;
      else op = 1;
    }
    else if(op == 3){
      printf("Insira o numero do roteador que ira receber a mensagem:\n");
      scanf("%d", &tmps.dest);
      if(tmps.dest < 0 || tmps.dest >= NROUT) printf("\nEsse roteador nao existe!\n");
      else{
        getchar();
        printf("\nInsira a mensagem com no maximo %d caracteres:\n", MESSAGE_SIZE);
        fgets(tmps.msg, MESSAGE_SIZE, stdin);
        tmps.orig = id;
        tmps.num = snum++;
        //Atribui porta e o endereco para qual sera enviada a mensagem
        si_send.sin_port = htons(routers[routing_table[tmps.dest].nhope].port);
        if (inet_aton(routers[routing_table[tmps.dest].nhope].adress , &si_send.sin_addr) == 0)
            printf("Falha ao obter endereco do destinatario\n");
        else{
          //Envia para o socket requisitado(socket, dados, tamanho dos dados, flags, endereço, tamanho do endereço)
          if (sendto(sock, &tmps, sizeof(tmps), 0, (struct sockaddr*) &si_send, slen) == -1){
            printf("\nFalha ao enviar mensagem...\n");
            snum--;
          }
          else{
            char c[2];
            c[1] = 0;
            printf("\nMensagem enviada!\n");
            strcpy(logg, "Mensagem #");
            c[0] = (snum - 1) + '0';
            strcat(logg, c);
            strcat(logg, " Enviada para o destino ");
            c[0] = tmps.dest + '0';
            strcat(logg, c);
          }
        }
      }
      sleep(3);
      op = -1;
    }
    if(op == 4){
      system("clear");
      exit(0);
    }
  }
}

void* receiver(void *nothing){
  char c[2];
  c[1] = 0;

  while(1){
    if ((recvfrom(sock, &tmpr, sizeof(tmpr), 0, (struct sockaddr *) &si_me, &slen)) == -1)
        printf("Erro ao receber mensagem\n");
    //Se essa Mensagem eh para mim
    if(tmpr.dest == id){
      new++;
      messages[qtmsg++] = tmpr;
      strcpy(logg, "Mensagem recebida de ");
      c[0] = tmpr.orig + '0';
      strcat(logg, c);
    }
    else{
      //Atribui porta e o endereco para qual sera enviada a mensagem
      si_rerout.sin_port = htons(routers[routing_table[tmpr.dest].nhope].port);
      if (inet_aton(routers[routing_table[tmpr.dest].nhope].adress , &si_rerout.sin_addr) == 0)
          printf("Falha ao obter endereco do destinatario\n");
      else{
        //Envia para o socket requisitado(socket, dados, tamanho dos dados, flags, endereço, tamanho do endereço)
        if (sendto(sock, &tmpr, sizeof(tmpr), 0, (struct sockaddr*) &si_rerout, slen) == -1){
          printf("\nFalha ao enviar mensagem...\n");
        }
        else{
          strcpy(logg, "Mensagem #");
          c[0] = tmpr.num + '0';
          strcat(logg, c);
          strcat(logg, " vinda de ");
          c[0] = tmpr.orig + '0';
          strcat(logg, c);
          strcat(logg, " encaminhada para ");
          c[0] = tmpr.dest + '0';
          strcat(logg, c);
          strcat(logg, " via ");
          c[0] = routing_table[tmpr.dest].nhope + '0';
          strcat(logg, c);
        }
      }
    }
  }
}
