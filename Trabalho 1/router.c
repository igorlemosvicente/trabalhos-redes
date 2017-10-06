#include "routing.h"

void initialize(int id, hope_t routing_table[NROUT]);
void dijkstra(int id, int graph[NROUT][NROUT], hope_t routing_table[NROUT]);

int main(int argc, char *argv[]){
  int id;
  hope_t routing_table[NROUT]; //Tabela de roteamento

  //Trata erros no argumento
  if(argc < 2)
    die("Argumentos insuficientes, informe o ID do roteador a ser instanciado\n");
  if(argc > 2)
    die("Argumentos demais, informe apenas o ID do roteador a ser instanciado\n");
  id = toint(argv[1]);
  if(id < 0 || id >= NROUT) die("Não existe um roteador com este id");

  initialize(id, routing_table); //Inicializa o roteador

  //while(1);

  return 0;
}

void initialize(int id, hope_t routing_table[NROUT]){
  int graph[NROUT][NROUT], i, j, u, v, w;

  memset(graph, -1, sizeof(graph));
  for(i = 0, j = 0; i < NROUT; i++, j++) graph[i][j] = 0; //Zera diagonal do grafo
  FILE *routers = fopen("enlaces.config", "r");
  if(!routers) die("Falha ao abrir o arquivo de enlaces\n");
  while(fscanf(routers, "%d %d %d\n", &u, &v, &w) != EOF)
    graph[u][v] = graph[v][u] = w; //Assume-se arestas bidirecionais
  fclose(routers);

  //print_graph(graph);
  dijkstra(id, graph, routing_table);




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
  print_routing_table(routing_table);
}
