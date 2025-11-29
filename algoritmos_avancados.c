#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Estruturas --- */

/* Sala: nó da árvore binária que representa cômodos da mansão */
typedef struct Sala {
    char *nome;             /* nome da sala (ex: "Sala de Estar") */
    char *pista;            /* pista opcional; NULL se não houver ou já coletada */
    struct Sala *esquerda;  /* caminho à esquerda */
    struct Sala *direita;   /* caminho à direita */
} Sala;

/* PistaNode: nó da BST que armazena as pistas coletadas */
typedef struct PistaNode {
    char *conteudo;             /* texto da pista */
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

/* --- Protótipos de funções (documentados) --- */

/* criarSala: cria dinamicamente uma sala com nome e pista opcional.
   Recebe strings que serão duplicadas internamente. Retorna ponteiro para Sala. */
Sala* criarSala(const char *nome, const char *pista);

/* inserirPista: insere uma nova pista (string) na BST de pistas.
   Se a pista já existe (comparação string), não insere duplicata. */
void inserirPista(PistaNode **root, const char *pista);

/* explorarSalasComPistas: controla a navegação (do usuário) pela mansão a partir
   da sala atual; toda pista encontrada é automaticamente adicionada à BST.
   Parâmetros: sala atual (inicial) e ponteiro para a raiz da BST de pistas. */
void explorarSalasComPistas(Sala *atual, PistaNode **raizPistas);

/* exibirPistas: imprime as pistas coletadas em ordem alfabética (in-order traversal da BST). */
void exibirPistas(PistaNode *root);

/* Funções auxiliares para liberar memória (boa prática) */
void liberarSalas(Sala *root);
void liberarPistas(PistaNode *root);

/* --- Implementação --- */

Sala* criarSala(const char *nome, const char *pista) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro: sem memória para criar sala.\n");
        exit(EXIT_FAILURE);
    }
    s->nome = nome ? strdup(nome) : NULL;
    s->pista = pista ? strdup(pista) : NULL;
    s->esquerda = NULL;
    s->direita = NULL;
    return s;
}

void inserirPista(PistaNode **root, const char *pista) {
    if (!pista || strlen(pista) == 0) return;
    if (*root == NULL) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) {
            fprintf(stderr, "Erro: sem memória para inserir pista.\n");
            exit(EXIT_FAILURE);
        }
        n->conteudo = strdup(pista);
        n->esq = n->dir = NULL;
        *root = n;
        return;
    }
    int cmp = strcmp(pista, (*root)->conteudo);
    if (cmp == 0) {
        /* já coletada - não inserir duplicata */
        return;
    } else if (cmp < 0) {
        inserirPista(&(*root)->esq, pista);
    } else {
        inserirPista(&(*root)->dir, pista);
    }
}

void explorarSalasComPistas(Sala *atual, PistaNode **raizPistas) {
    if (!atual) return;
    char escolha[10];

    printf("Exploração iniciada. Digite 'e' para esquerda, 'd' para direita, 's' para sair.\n");

    while (1) {
        printf("\nVocê está em: %s\n", atual->nome);

        /* se existir pista na sala atual, coletar automaticamente */
        if (atual->pista && strlen(atual->pista) > 0) {
            printf("Você encontrou uma pista aqui! -> \"%s\"\n", atual->pista);
            inserirPista(raizPistas, atual->pista);
            /* marca como coletada: libera e torna NULL para não coletar novamente */
            free(atual->pista);
            atual->pista = NULL;
        } else {
            printf("Nenhuma pista nova nesta sala.\n");
        }

        /* opções de navegação (apresentar nomes das salas quando existir) */
        printf("Opções: ");
        if (atual->esquerda) {
            printf("[e] Ir para esquerda (%s)  ", atual->esquerda->nome);
        } else {
            printf("[e] Esquerda (não disponível)  ");
        }
        if (atual->direita) {
            printf("[d] Ir para direita (%s)  ", atual->direita->nome);
        } else {
            printf("[d] Direita (não disponível)  ");
        }
        printf("[s] Sair e exibir pistas coletadas\n");

        printf("Escolha: ");
        if (!fgets(escolha, sizeof(escolha), stdin)) {
            /* entrada falhou */
            clearerr(stdin);
            continue;
        }

        /* pegar primeiro caractere útil */
        char c = '\0';
        for (int i = 0; escolha[i] != '\0'; ++i) {
            if (escolha[i] != ' ' && escolha[i] != '\n' && escolha[i] != '\r' && escolha[i] != '\t') {
                c = escolha[i];
                break;
            }
        }
        if (c == 's' || c == 'S') {
            printf("Exploração finalizada pelo jogador.\n");
            break;
        } else if (c == 'e' || c == 'E') {
            if (atual->esquerda) {
                atual = atual->esquerda;
            } else {
                printf("Caminho à esquerda não existe. Escolha outra opção.\n");
            }
        } else if (c == 'd' || c == 'D') {
            if (atual->direita) {
                atual = atual->direita;
            } else {
                printf("Caminho à direita não existe. Escolha outra opção.\n");
            }
        } else {
            printf("Entrada inválida. Use 'e', 'd' ou 's'.\n");
        }
    }
}

void exibirPistas(PistaNode *root) {
    if (!root) return;
    exibirPistas(root->esq);
    printf("- %s\n", root->conteudo);
    exibirPistas(root->dir);
}

void liberarSalas(Sala *root) {
    if (!root) return;
    liberarSalas(root->esquerda);
    liberarSalas(root->direita);
    free(root->nome);
    if (root->pista) free(root->pista);
    free(root);
}

void liberarPistas(PistaNode *root) {
    if (!root) return;
    liberarPistas(root->esq);
    liberarPistas(root->dir);
    free(root->conteudo);
    free(root);
}

/* --- main: constrói o mapa fixo e inicia exploração --- */
int main(void) {
    /* Construção fixa do mapa da mansão (árvore binária de salas).
       Estrutura de exemplo simples e clara para o desafio:
                   Hall de Entrada
                   /            \
           Sala de Estar      Corredor
             /      \         /     \
        Biblioteca  Jardim  Escritório Cozinha
       (tem pista)        (tem pista)   (tem pista)
    */

    Sala *hall = criarSala("Hall de Entrada", "Pegada molhada perto do tapete");
    Sala *salaEstar = criarSala("Sala de Estar", "Vidro quebrado com marcas");
    Sala *biblioteca = criarSala("Biblioteca", "Livro aberto na página do contrato");
    Sala *jardim = criarSala("Jardim", NULL); /* sem pista */
    Sala *corredor = criarSala("Corredor", NULL);
    Sala *escritorio = criarSala("Escritório", "Carta rasgada com iniciais 'M.S.'");
    Sala *cozinha = criarSala("Cozinha", "Panela com vestígios de veneno");

    /* ligar nós (fixo, sem inserções dinâmicas posteriores) */
    hall->esquerda = salaEstar;
    hall->direita  = corredor;

    salaEstar->esquerda = biblioteca;
    salaEstar->direita  = jardim;

    corredor->esquerda = escritorio;
    corredor->direita  = cozinha;

    /* Raiz da BST de pistas (inicialmente vazia) */
    PistaNode *raizPistas = NULL;

    printf("=== Detective Quest: Coleta de Pistas (nível aventureiro) ===\n");
    printf("Você começará pelo 'Hall de Entrada'. Explore (e/d) e colecione pistas; digite 's' para terminar.\n\n");

    /* explorar a partir do hall */
    explorarSalasComPistas(hall, &raizPistas);

    /* Exibir todas as pistas coletadas em ordem alfabética */
    printf("\n--- Pistas coletadas (em ordem alfabética) ---\n");
    if (!raizPistas) {
        printf("Nenhuma pista foi coletada.\n");
    } else {
        exibirPistas(raizPistas);
    }

    /* liberar memória alocada */
    liberarPistas(raizPistas);
    liberarSalas(hall);

    printf("\nObrigado por jogar! Boa investigação.\n");
    return 0;
}
