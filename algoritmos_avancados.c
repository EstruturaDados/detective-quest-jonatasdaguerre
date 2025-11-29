#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Estrutura que representa uma sala da mansão */
typedef struct Sala {
    char nome[50];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

/* Função que cria uma nova sala dinamicamente */
Sala* criarSala(char nome[]) {
    Sala *novaSala = (Sala*) malloc(sizeof(Sala));
    
    if (novaSala == NULL) {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }

    strcpy(novaSala->nome, nome);
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;

    return novaSala;
}

/* Função que permite a navegação pela mansão */
void explorarSalas(Sala *salaAtual) {
    char opcao;

    while (salaAtual != NULL) {
        printf("\nVocê está em: %s\n", salaAtual->nome);

        /* Se for nó folha, encerra a exploração */
        if (salaAtual->esquerda == NULL && salaAtual->direita == NULL) {
            printf("Você chegou a um cômodo final. Fim da exploração!\n");
            break;
        }

        printf("Escolha um caminho:\n");
        printf("e - Ir para a esquerda\n");
        printf("d - Ir para a direita\n");
        printf("s - Sair do jogo\n");
        printf("Opção: ");
        scanf(" %c", &opcao);

        if (opcao == 'e') {
            if (salaAtual->esquerda != NULL) {
                salaAtual = salaAtual->esquerda;
            } else {
                printf("Não há sala à esquerda!\n");
            }
        } 
        else if (opcao == 'd') {
            if (salaAtual->direita != NULL) {
                salaAtual = salaAtual->direita;
            } else {
                printf("Não há sala à direita!\n");
            }
        }
        else if (opcao == 's') {
            printf("Você saiu da mansão.\n");
            break;
        }
        else {
            printf("Opção inválida!\n");
        }
    }
}

/* Função principal */
int main() {

    /* Montagem da árvore da mansão */
    Sala *hall = criarSala("Hall de Entrada");

    Sala *salaEstar = criarSala("Sala de Estar");
    Sala *biblioteca = criarSala("Biblioteca");

    Sala *cozinha = criarSala("Cozinha");
    Sala *jardim = criarSala("Jardim");

    Sala *escritorio = criarSala("Escritório");
    Sala *porão = criarSala("Porão Secreto");

    /* Ligando os nós da árvore */
    hall->esquerda = salaEstar;
    hall->direita = biblioteca;

    salaEstar->esquerda = cozinha;
    salaEstar->direita = jardim;

    biblioteca->esquerda = escritorio;
    biblioteca->direita = porão;

    /* Início da exploração */
    printf("=== Bem-vindo ao Detective Quest ===\n");
    printf("Explore a mansão escolhendo os caminhos.\n");

    explorarSalas(hall);

    return 0;
}
