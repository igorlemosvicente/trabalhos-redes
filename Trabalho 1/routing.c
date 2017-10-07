#include "routing.h"

//Funcao para inicializar o roteador
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

  FILE *routers_file = fopen("roteador.config", "r");
  if(!routers_file) die("Falha ao abrir o arquivo de roteadores\n");
  for(i = 0; fscanf(routers_file, "%d %d %s\n", &routers[i].id, &routers[i].port, routers[i].adress) != EOF; i++);
  fclose(routers_file);

  //for( i = 0; i < NROUT; i++) printf("%d %d %s\n", routers[i].id, routers[i].port, routers[i].adress);

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

//Preenche o grafo e a tabela de roteamento
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

//Funcao para imprimir mensagens de erro e encerrar o programa
void die(char* msg){
  printf("%s\n", msg);
  exit(1);
};

//Funcao para converter uma string para inteiro
int toint(char *str){
  int i, pot, ans;
  ans = 0;
  for(i = strlen(str) - 1, pot = 1; i >= 0; i--, pot *= 10)
    ans += pot * (str[i] - '0');
  return ans;
}

//Funcao para imprimir o grafo da topologia
void print_graph(int graph[NROUT][NROUT]){
  int i, j;

  for(i = 0; i < NROUT; i++){
    for(j = 0; j < NROUT; j++) printf("%d ", graph[i][j]);
    printf("\n");
  }
}

//Funcao para imprimir tabela de roteamento de um roteador
void print_routing_table(hope_t routing_table[NROUT]){
  int w;
  printf("Destino|Proximo Salto|Custo\n");
  for(w = 0; w < NROUT; w++)
    printf("%d      |%d            |%d\n",w, routing_table[w].nhope, routing_table[w].dist);
}


//Funcao para inserir arestas na heap
void insert(edge_t e, int last, edge_t heap[]){ //Insere um novo elemento na heap
  heap[last] = e;
  checaHeapPai(last, heap);
  return;
}

//Funcao para extrair arestas da heap
edge_t extract(int target, edge_t heap[], int last){
  edge_t ans;
  ans = heap[target];
  heap[target] = heap[last];
  checaHeapFilho(target, heap, last);
  return ans;
}

//Corrige a heap do elemento i até o nível necessário
void checaHeapFilho(int i, edge_t heap[], int last){
  int smallest = i;
  edge_t tmp;
  if(valido(leftson(i), last) && heap[leftson(i)].dist < heap[smallest].dist) smallest = leftson(i);
  if(valido(rightson(i), last) && heap[rightson(i)].dist < heap[smallest].dist) smallest = rightson(i);

  if(smallest != i){
    tmp = heap[smallest];
    heap[smallest] = heap[i];
    heap[i] = tmp;
    checaHeapFilho(smallest, heap, last);
  }
  return;
}

int checaHeapPai(int i, edge_t heap[]){
  edge_t tmp;

  if(i == 1) return i;
  //Se o pai for maior que o elemento considerado, troca e chama para o pai
  if(heap[dad(i)].dist > heap[i].dist){
    tmp = heap[dad(i)];
    heap[dad(i)] = heap[i];
    heap[i] = tmp;
  }
  checaHeapPai(dad(i), heap);
  return i;
}
