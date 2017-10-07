#include "routing.h"

int main(int argc, char *argv[]){
  int id, sock, i, new = 0, op = -1, snum = 0, qtmsg = 0;
  message_t messages[BOX_SIZE], tmp;
  struct sockaddr_in si_me, si_other; //Endereço do roteador
  hope_t routing_table[NROUT]; //Tabela de roteamento
  router_t routers[NROUT]; //Informações sobre os roteadores

  //Trata erros no argumento
  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado\n");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado\n");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id");

  //Zera a estrutura para envios
  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET; //Familia
  si_other.sin_addr.s_addr = htonl(INADDR_ANY); //Atribui o socket a todo tipo de interface
  initialize(id, &sock, &si_me, routers, routing_table); //Inicializa o roteador
  //printf("\n%d %d %s\n", id, ntohs(si_me.sin_port), routers[id].adress);
  while(1){
    system("clear");
    if(op == -1){
      printf("ROTEADOR %d\n", id);
      printf("------------------------------------------------------\n");
      printf("%d novas mensagens\n\n", new);
      printf("1 - Checar Tabela de Roteamento\n");
      printf("2 - Ler Mensagens\n");
      printf("3 - Escrever Mensagem\n");
      printf("4 - Sair\n\n");
      scanf("%d", &op);
    }
    else if(op == 1){
      print_routing_table(routing_table);
      printf("\nInsira 0 para voltar\n");
      scanf("%d", &op);
      if(op == 0) op = -1;
      else op = 1;
    }
    else if(op == 2){
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
      scanf("%d", &tmp.dest);
      if(tmp.dest < 0 || tmp.dest >= NROUT) printf("\nEsse roteador nao existe!\n");
      else{
        getchar();
        printf("\nInsira a mensagem com no maximo %d caracteres:\n", MESSAGE_SIZE);
        fgets(tmp.msg, MESSAGE_SIZE, stdin);
        tmp.orig = id;

        //Atribui porta e o endereco para qual sera enviada a mensagem
        si_other.sin_port = htons(routers[tmp.dest].port);
        if (inet_aton(routers[tmp.dest].adress , &si_other.sin_addr) == 0)
            printf("Falha ao obter endereco do destinatario\n");
        else{
          //Envia para o socket requisitado(socket, dados, tamanho dos dados, flags, endereço, tamanho do endereço)
          if (sendto(sock, &tmp, sizeof(tmp), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1)
            printf("\nFalha ao enviar mensagem...\n");
          else printf("\nMensagem enviada!\n");
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
