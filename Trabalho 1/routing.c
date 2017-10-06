#include "routing.h"

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

  for(w = 0; w < NROUT; w++)
    printf("%d| %d| %d\n",w, routing_table[w].nhope, routing_table[w].dist);
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
