# TrabSOEtapa2
Enunciado dos trabalho

Desenvolver um sistema de arquivos distribuídos baseado no padrão EXT3 da família de Sistemas Operacionais Linux. O sistema deve permitir que arquivos locais sejam acessados por usuários remotos simultaneamente. As operações permitidas pelo sistema são: criar arquivo ou diretório, remover arquivo ou diretório, copiar dados de um diretório para outro e listar conteúdo de diretórios. 2.

Etapa 1

Desenvolver a estrutura de acesso do servidor de arquivos. Ele será acessado via socket TCP. Cada conexão será gerida por uma thread. Não é necessário autenticação dos usuários. O acesso concorrente ao mesmo arquivo deve ser garantido com o uso de semáforos/mutexes. Simule o sistema de arquivos utilizando o sistema de arquivos do sistema operacional que você está usando. Para demonstrar o funcionamento, não é necessário criar um aplicativo cliente. Você pode usar o aplicativo netcat disponível para Linux e Windows.

Recomenda-se que o servidor imprima mensagens na tela para demonstrar o funcionamento ao professor. 3.

Etapa 2

Estenda o trabalho desenvolvendo a estrutura de alocação e organização de arquivos do servidor de arquivos. O servidor de arquivos deve seguir as mesmas diretrizes do sistema de arquivos EXT3. O disco do servidor deve ser simulado através de um único arquivo no sistema de arquivos do sistemas operacional onde o servidor está sendo executado. A thread no servidor deve receber uma solicitação do cliente e então repassá-la a um novo processo via pipe nomeado e este deverá executar a ação solicitada e devolver ao servidor uma mensagem de status, informando se a solicitação foi realizada com sucesso ou com erro.

Informações

O trabalho deverá ser feito em dupla ou individualmente. No dia da apresentação, data que deverá ser marcada com antecedência com o professor, o aluno ou alunos irão apresentar o programa em execução mostrando as funcionalidades do mesmo, bem como apresentar o código fonte. As notas serão atribuídas de maneira individual, ou seja, caso o trabalho tenha sido feito em dupla cada um dos autores receberá uma nota. Esta nota dependerá do desempenho durante apresentação do trabalho onde o professor arguirá o aluno. O trabalho deverá ser desenvolvido na linguagem de programação C usando as bibliotecas de semáforos, threads e comunicação entre processos (socket local ou pipe nomeado). 

Ponto extra em cada etapa (nota máxima é 10)

+ 0.5 se usar controle de versão durante todo o desenvolvimento (ex. Github ou Bitbucket)

+ 0.5 se disponibilizar makefile para compilar o projeto
Última atualização: sexta, 25 Mai 2018, 18:03
