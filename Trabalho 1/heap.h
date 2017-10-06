#define leftson(x) (2 * x) //Filho a esquerda
#define rightson(x) (2 * x + 1) //Filho a direita
#define dad(x) (x % 2 ? (x-1)/2 : x/2) //Pai (Se for par ou ímpar)
#define valido(x, last) (x <= last)

struct proc *extract(int target, struct proc* heap[], int last);
void checaHeapFilho(int i, struct proc* heap[], int last);
int checaHeapPai(int i, struct proc* heap[]);
