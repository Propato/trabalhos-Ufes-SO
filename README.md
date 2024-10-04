# Trabalhos Ufes: Sistemas Operacionais

Este é o trabalho desenvolvido na disciplina de Sistemas Operacionais.

O objetivo é o desenvolvimento de uma shell própria. Executando comandos e funções, em foreground e background.

Comandos em background são chamados com # e geram filhos, também em background, que executam o mesmo comando.

Funções são executadas pela própria shell e chamadas no terminal por `die` e `waitall`.

Se um processo filho da shell receber um sinal, os demais processos chamados na mesma linha de comando devem receber o mesmo sinal.

### List

Foi desenvolvida uma lista encadeada com sentinela para armazenar as informações (Ids) dos processos. Cada célula contém os ids dos processos que foram criado juntos (pelos comandos chamados na mesma linha).

Usa-se isso para quando um filho receber um sinal, seu Id é buscado na lista e é retornado todos os Ids dos demais processos que também deverão receber o sinal.

## Running

Inicialmente, é necessário executar o `make all` para compilar e gerar o arquivo executável.

```shell
make all
./SO
```

Agora, basta inserir qualquer comando que a shell executará. Ela chamará os programas igual a shell real.

## Ending & Cleaning

A melhor maneira de encerrar é com o comando: `die`. Para encerrar todos os filhos depois a própria shell, pois caso contrário, a shell pode ser encerrada mas ios filhos em background continuarão existindo.

Mas caso queira, é possível encerrar com `Ctrl + C`.

Após encerrar a shell, usa-se o comando: `make clean` para remover os arquivos de compilação e o executável.

## Conclusions

Assim, foi possível desenvolver uma shell funcional, que executa comandos e funções próprias, tratando erros para comandos inválidos e mantendo um histórico dos Ids dos processos filhos que ela cria e executa.

<h6 align="center">David Propato <a href="https://github.com/Propato">@Propato</a> </h6>
