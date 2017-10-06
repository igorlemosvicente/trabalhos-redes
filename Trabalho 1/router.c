#include "routing.h"

void initialize(int id, int *sock, struct sockaddr_in *si_me, router_t routers[NROUT], hope_t routing_table[NROUT]);
void dijkstra(int id, int graph[NROUT][NROUT], hope_t routing_table[NROUT]);

int main(int argc, char *argv[]){
  int id, sock;
  struct sockaddr_in si_me; //Endereço do roteador
  hope_t routing_table[NROUT]; //Tabela de roteamento
  router_t routers[NROUT]; //Informações sobre os roteadores

  //Trata erros no argumento
  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado\n");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado\n");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id");

  initialize(id, &sock, &si_me, routers, routing_table); //Inicializa o roteador
  printf("\n%d %d %s\n", id, ntohs(si_me.sin_port), routers[id].adress);
  //while(1);

  return 0;
}

void initialize(int id, int *sock, struct sockaddr_in *si_me, router_t routers[NROUT], hope_t routing_table[NROUT]){
  int graph[NROUT][NROUT], i, j, u, v, w;

  memset(graph, -1, sizeof(graph));
  for(i = 0, j = 0; i < NROUT; i++, j++) graph[i][j] = 0; //Zera diagonal do grafo
  FILE *links = fopen("enlaces.config", "r");
  if(!links) die("Falha ao abrir o arquivo de enlaces\n");
  while(fscanf(links, "%d %d %d\n", &u, &v, &w) != EOF)
    graph[u][v] = graph[v][u] = w; //Assume-se arestas bidirecionais
  fclose(links);

  dijkstra(id, graph, routing_table);
  printf("TABELA DE ROTEAMENTO PARA O NÓ %d\n", id);
  print_routing_table(routing_table);

  FILE *routers_file = fopen("roteador.config", "r");
  if(!routers_file) die("Falha ao abrir o arquivo de roteadores\n");
  for(i = 0; fscanf(routers_file, "%d %d %s\n", &routers[i].id, &routers[i].port, routers[i].adress) != EOF; i++);
  fclose(routers_file);

  for( i = 0; i < NROUT; i++) printf("%d %d %s\n", routers[i].id, routers[i].port, routers[i].adress);

  //Cria o socket(dominio, tipo, protocolo)
  if((*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    die("Falha ao criar Socket\n");
  //Zera a estrutura
  memset((char *) si_me, 0, sizeof(*si_me));
  si_me->sin_family = AF_INET; //Familia
  si_me->sin_port = htons(routers[id].port); //Porta em ordem de bytes de rede
  si_me->sin_addr.s_addr = htonl(INADDR_ANY); //Atribui o socket a todo tipo de interface

  //Liga o socket a porta (atribui o endereço ao file descriptor)
  if( bind(*sock , (struct sockaddr*) si_me, sizeof(*si_me) ) == -1)
    die("A ligacao do socket com a porta falhou\n");
}

void dijkstra(int id, int graph[NROUT][NROUT], hope_t routing_table[NROUT]){
  edge_t heap[NROUT * NROUT], tmp;
  int v, d, w, last = 0, i, parent[NROUT];

  //Atribui custo e nexthope para ir de um roteador ate ele mesmo
  routing_table[id].nhope = id;
  routing_table[id].dist = 0;
  for(w = 0; w < NROUT; w++) routing_table[w].dist = -1; //Seta todas distancias como -1
  tmp.v = tmp.u = id;
  tmp.dist = 0;
  insert(tmp, ++last, heap); //Insere o no inicial na heap, com distancia 0
  while(last){
    tmp = extract(1, heap, last--); //extrai a aresta com menor custo
    v = tmp.v; d = tmp.dist;
    //printf("%d %d\n", v, d);

    if(routing_table[v].dist != -1) continue;
    routing_table[v].dist = d; //Atribui a distancia ao vértice

    parent[v] = tmp.u; //Atribui antecessor do vértice no caminho
    for(i = v; parent[i] != id; i = parent[i]); //Volta até o primeiro vértice antes da origem
    routing_table[v].nhope = i; //Atribui o nexthope daquele vértice

    for(w = 0; w < NROUT; w++) //Coloca as arestas vizinhas do vertice na heap
      if(graph[v][w] != -1 && routing_table[w].dist == -1){
        tmp.dist = d + graph[v][w];
        tmp.u = v;
        tmp.v = w;
        //printf("Inserindo na heap %d->%d com custo %d\n",tmp.u, tmp.v, tmp.dist);
        insert(tmp, ++last, heap);
      }
  }
}
