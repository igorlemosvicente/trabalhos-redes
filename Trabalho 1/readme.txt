-Para compilar use: gcc router.c routing.c -o router -lpthread

-Cada roteador deve ser instanciado executando ./router ID em um terminal diferente,
onde ID é o numero do roteador instanciado

-A topologia da redepode ser alterada no arquivo enlaces.config

-Informacoes sobre a porta utilizada por cada roteador, seus endereços, estamo no arquivo
roteador.config

-Depois de executar os roteadores, basta seguir as opções indicadas nos menus.

-O campo de Log só mostra a ultima operação envolvendo aquele roteador

-Para atualizar o campo de log e a quantidade de novas mensagens, a opção atualizar do
menu deve ser escolhida

-Uma imagem contendo a topologia default da rede acompanha este arquivo

-Configuracoes importantes como numero de roteadores, tamanho maximo da mensagem e capacidade
maxima de armazenamento de mensagens podem ser alteradas no arquivo config.h
