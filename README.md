# FRC_ChatRoom

ChatRoom é um projeto em C que implementa uma aplicação de chat baseada em cliente e servidor utilizando a arquitetura TCP/IP e sockets de rede.

## Rodar projeto

Para que você consigar executar os comandos de inicialização, você precisa estar dentro da pasta main ("FRC_ChatRoom/src/main")

1 - Inicialmente você deve começar o servidor com:

```
$make server
```

2 - Após o servidor começar, você deve rodar o cliente para cada um dos usuários:

```
$make client
```

## Comandos para o cliente

Para executar os comandos a seguir, você precisa estar na sala e enviar como mensagem o comando.

- Obter lista de usuários na sala:

```
/list
```

- Sair da sala:

```
/exit
```

- Entrar em outra sala após sair:

```
NumeroDaSala
```

## Documentos

O relatório do projeto e os slides podem ser encontrados no diretorio `/docs`.
