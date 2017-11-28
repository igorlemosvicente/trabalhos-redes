Author: Felipe Chabatura Neto - 1511100016

/*
ESTA VERSAO ESTA INCOMPLETA, NELA, TUDO QUE CADA ROTEADOR FAZ É:
-Carregar informações sobre sua porta e endereço
-Carregar informações sobre portas, endereços, e custos de enlace dos vizinhos imediatos
-Inicializar sua tabela de Roteamento
-Enfileirar um pacote para cada vizinho, contendo seu vetor de distancias
-Enviar os pacotes para os vizinhos (Esses, quando os recebem, nao fazem nada com eles)
*/

-Para compilar use: gcc router.c routing.c -o router -lpthread -Wall -O2

-Cada roteador deve ser instanciado executando ./router ID em um terminal diferente,
onde ID é o numero do roteador instanciado

-A topologia da rede pode ser alterada no arquivo enlaces.config

-Informacoes sobre a porta utilizada por cada roteador, seus endereços, estão no arquivo
roteador.config

-Uma imagem contendo a topologia default da rede acompanha este arquivo

-Configuracoes importantes como numero de roteadores, tamanho maximo da mensagem e tamanho maximo das filas
 podem ser alteradas no arquivo routing.h
