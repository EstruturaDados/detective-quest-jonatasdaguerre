#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 Helper: strdup alternativa (portável)
*/
char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* -------------------------
   Estruturas de dados
   -------------------------*/

/* Nó da árvore binária de salas (mapa da mansão) */
typedef struct Sala {
    char *nome;
    struct Sala *esq;
    struct Sala *dir;
} Sala;

/* Nó da BST de pistas (armazena strings únicas, ordenadas) */
typedef struct PistaNode {
    char *pista;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

/* Nó para encadeamento na tabela hash (mapeia pista -> suspeito) */
typedef struct HashNode {
    char *pista;
    char *suspeito;
    struct HashNode *proximo;
} HashNode;

/* Tabela hash (encadeamento separado) */
#define HASH_SIZE 101
typedef struct HashTable {
    HashNode *buckets[HASH_SIZE];
} HashTable;

/* -------------------------
   Protótipos (requisitados)
   -------------------------*/

/* criarSala() – cria dinamicamente um cômodo. */
Sala *criarSala(const char *nome);

/* explorarSalas() – navega pela árvore e ativa o sistema de pistas. */
void explorarSalas(Sala *raiz, PistaNode **coletadas, HashTable *ht);

/* inserirPista() / adicionarPista() – insere a pista coletada na árvore de pistas. */
PistaNode *inserirPista(PistaNode *raiz, const char *pista, int *inserido);

/* inserirNaHash() – insere associação pista/suspeito na tabela hash. */
void inserirNaHash(HashTable *ht, const char *pista, const char *suspeito);

/* encontrarSuspeito() – consulta o suspeito correspondente a uma pista. */
char *encontrarSuspeito(HashTable *ht, const char *pista);

/* verificarSuspeitoFinal() – conduz à fase de julgamento final. */
void verificarSuspeitoFinal(PistaNode *coletadas, HashTable *ht);

/* Funções utilitárias adicionais */
const char *pistaPorSala(const char *nome);
unsigned long hash_str(const char *str);
void listarPistasInOrder(PistaNode *raiz);
void liberarPistas(PistaNode *raiz);
void liberarSala(Sala *s);
void liberarHash(HashTable *ht);
int contarPistasQueApontam(PistaNode *raiz, HashTable *ht, const char *suspeito);
int pistaExisteInBST(PistaNode *raiz, const char *pista);

/* -------------------------
   Implementações
   -------------------------*/

/* criarSala() – cria dinamicamente um cômodo. */
Sala *criarSala(const char *nome) {
    Sala *s = malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro de alocacao criarSala\n");
        exit(1);
    }
    s->nome = strdup_safe(nome);
    s->esq = s->dir = NULL;
    return s;
}

/* Função que retorna a pista associada ao nome da sala (regras codificadas). */
const char *pistaPorSala(const char *nome) {
    /* Regras codificadas: retornar NULL se não houver pista nessa sala */
    if (strcmp(nome, "Hall") == 0) return "pegada de lama";
    if (strcmp(nome, "Sala de Estar") == 0) return "lenço rasgado com monograma";
    if (strcmp(nome, "Cozinha") == 0) return "faca com impressao parcial";
    if (strcmp(nome, "Biblioteca") == 0) return "marcador de livro dobrado";
    if (strcmp(nome, "Escritório") == 0) return "nota ameaçadora";
    if (strcmp(nome, "Jardim") == 0) return "fio de tecido azul";
    if (strcmp(nome, "Quarto") == 0) return "guilhotina de cabelo (fiapo)";
    /* outras salas podem não ter pista */
    return NULL;
}

/* BST de pistas: inserir (sem duplicatas) */
PistaNode *inserirPista(PistaNode *raiz, const char *pista, int *inserido) {
    if (!pista) { if (inserido) *inserido = 0; return raiz; }
    if (!raiz) {
        PistaNode *n = malloc(sizeof(PistaNode));
        n->pista = strdup_safe(pista);
        n->esq = n->dir = NULL;
        if (inserido) *inserido = 1;
        return n;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp == 0) {
        if (inserido) *inserido = 0; /* já coletada */
        return raiz;
    } else if (cmp < 0) raiz->esq = inserirPista(raiz->esq, pista, inserido);
    else raiz->dir = inserirPista(raiz->dir, pista, inserido);
    return raiz;
}

/* Verifica existência de uma pista na BST */
int pistaExisteInBST(PistaNode *raiz, const char *pista) {
    if (!raiz) return 0;
    int cmp = strcmp(pista, raiz->pista);
    if (cmp == 0) return 1;
    if (cmp < 0) return pistaExisteInBST(raiz->esq, pista);
    return pistaExisteInBST(raiz->dir, pista);
}

/* Impressão in-order das pistas coletadas */
void listarPistasInOrder(PistaNode *raiz) {
    if (!raiz) return;
    listarPistasInOrder(raiz->esq);
    printf(" - %s\n", raiz->pista);
    listarPistasInOrder(raiz->dir);
}

/* Hash: djb2 */
unsigned long hash_str(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + (unsigned char)c;
    return hash;
}

/* inserirNaHash() – insere associação pista/suspeito na tabela hash. */
void inserirNaHash(HashTable *ht, const char *pista, const char *suspeito) {
    if (!pista || !suspeito) return;
    unsigned long h = hash_str(pista) % HASH_SIZE;
    HashNode *head = ht->buckets[h];
    /* se já existe a mesma pista -> atualizar suspeito (opcional) */
    for (HashNode *cur = head; cur; cur = cur->proximo) {
        if (strcmp(cur->pista, pista) == 0) {
            free(cur->suspeito);
            cur->suspeito = strdup_safe(suspeito);
            return;
        }
    }
    HashNode *n = malloc(sizeof(HashNode));
    n->pista = strdup_safe(pista);
    n->suspeito = strdup_safe(suspeito);
    n->proximo = head;
    ht->buckets[h] = n;
}

/* encontrarSuspeito() – consulta o suspeito correspondente a uma pista. */
char *encontrarSuspeito(HashTable *ht, const char *pista) {
    if (!pista) return NULL;
    unsigned long h = hash_str(pista) % HASH_SIZE;
    for (HashNode *cur = ht->buckets[h]; cur; cur = cur->proximo) {
        if (strcmp(cur->pista, pista) == 0) return cur->suspeito;
    }
    return NULL;
}

/* Conta quantas pistas na BST apontam para o suspeito dado */
int contarPistasQueApontam(PistaNode *raiz, HashTable *ht, const char *suspeito) {
    if (!raiz) return 0;
    int contador = 0;
    char *s = encontrarSuspeito(ht, raiz->pista);
    if (s && strcmp(s, suspeito) == 0) contador++;
    contador += contarPistasQueApontam(raiz->esq, ht, suspeito);
    contador += contarPistasQueApontam(raiz->dir, ht, suspeito);
    return contador;
}

/* verificarSuspeitoFinal() – conduz à fase de julgamento final. */
void verificarSuspeitoFinal(PistaNode *coletadas, HashTable *ht) {
    printf("\n--- Fase de Acusacao ---\n");
    printf("Pistas coletadas:\n");
    if (!coletadas) {
        printf("Nenhuma pista coletada. Nao ha evidencias suficientes.\n");
        return;
    }
    listarPistasInOrder(coletadas);

    char buf[128];
    printf("\nIndique o nome do suspeito que deseja acusar: ");
    if (!fgets(buf, sizeof(buf), stdin)) return;
    buf[strcspn(buf, "\n")] = '\0';
    /* trim espaços em excesso */
    char *p = buf;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') {
        printf("Acusacao vazia. Encerrando.\n");
        return;
    }
    /* Normaliza: deixar a primeira letra maiuscula? Comparações serão exatas */
    char suspeito_acusado[128];
    strncpy(suspeito_acusado, p, sizeof(suspeito_acusado)-1);
    suspeito_acusado[sizeof(suspeito_acusado)-1] = '\0';

    int cont = contarPistasQueApontam(coletadas, ht, suspeito_acusado);
    printf("\nO acusado: %s\n", suspeito_acusado);
    printf("Pistas coletadas que apontam para ele: %d\n", cont);
    if (cont >= 2) {
        printf("VEREDICTO: Ha evidencias suficientes. A acusacao foi bem-sucedida!\n");
    } else {
        printf("VEREDICTO: Evidencias insuficientes. Ninguem prende.\n");
    }
}

/* explorarSalas() – navega pela árvore e ativa o sistema de pistas. */
void explorarSalas(Sala *raiz, PistaNode **coletadas, HashTable *ht) {
    if (!raiz) {
        printf("Mapa vazio.\n");
        return;
    }
    Sala *atual = raiz;
    char buf[32];

    printf("Iniciando exploracao da mansao. Comandos: (e) esquerda, (d) direita, (s) sair\n");

    while (1) {
        printf("\nVoce esta na sala: %s\n", atual->nome);
        const char *pista = pistaPorSala(atual->nome);
        if (pista) {
            if (!pistaExisteInBST(*coletadas, pista)) {
                int inserido = 0;
                *coletadas = inserirPista(*coletadas, pista, &inserido);
                if (inserido) {
                    printf("Voce encontrou uma pista: \"%s\" (coletada automaticamente)\n", pista);
                } else {
                    printf("Pista encontrada \"%s\" (ja coletada anteriormente)\n", pista);
                }
            } else {
                printf("Nesta sala ha uma pista: \"%s\" (ja coletada)\n", pista);
            }
        } else {
            printf("Nesta sala nao ha pista.\n");
        }

        printf("Escolha proxima acao [(e) esquerda | (d) direita | (s) sair]: ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        buf[strcspn(buf, "\n")] = '\0';
        char cmd = tolower((unsigned char)buf[0]);
        if (cmd == 's') {
            printf("Exploracao finalizada pelo jogador.\n");
            break;
        } else if (cmd == 'e') {
            if (atual->esq) atual = atual->esq;
            else printf("Nao ha sala a esquerda daqui.\n");
        } else if (cmd == 'd') {
            if (atual->dir) atual = atual->dir;
            else printf("Nao ha sala a direita daqui.\n");
        } else {
            printf("Comando desconhecido. Use e, d ou s.\n");
        }
    }
}

/* Liberar memória (pistas BST) */
void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz->pista);
    free(raiz);
}

/* Liberar salas */
void liberarSala(Sala *s) {
    if (!s) return;
    liberarSala(s->esq);
    liberarSala(s->dir);
    free(s->nome);
    free(s);
}

/* Liberar hash */
void liberarHash(HashTable *ht) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashNode *cur = ht->buckets[i];
        while (cur) {
            HashNode *tmp = cur->proximo;
            free(cur->pista);
            free(cur->suspeito);
            free(cur);
            cur = tmp;
        }
        ht->buckets[i] = NULL;
    }
}

/* -------------------------
   Main: monta mapa, popula hash e inicia jogo
   -------------------------*/
int main(void) {
    /* Montagem manual do mapa da mansao (arvore binaria) */
    /* Exemplo de estrutura:
              Hall
             /    \
       Sala de Estar  Cozinha
         /     \        \
     Biblioteca Escritório Jardim
                  /
                Quarto
    */
    Sala *hall = criarSala("Hall");
    Sala *salaestar = criarSala("Sala de Estar");
    Sala *cozinha = criarSala("Cozinha");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *escritorio = criarSala("Escritório");
    Sala *jardim = criarSala("Jardim");
    Sala *quarto = criarSala("Quarto");

    /* ligar nós */
    hall->esq = salaestar;
    hall->dir = cozinha;
    salaestar->esq = biblioteca;
    salaestar->dir = escritorio;
    cozinha->dir = jardim;
    escritorio->esq = quarto;

    /* Criar e popular a tabela hash (pista -> suspeito).
       Essas associações seguem a "historia" do desafio. */
    HashTable ht = {0};

    /* Definimos as pistas (mesmas strings que pistaPorSala retorna)
       e associamos suspeitos (nomes). */
    inserirNaHash(&ht, "pegada de lama", "Carlos");
    inserirNaHash(&ht, "lenço rasgado com monograma", "Ana");
    inserirNaHash(&ht, "faca com impressao parcial", "Carlos");
    inserirNaHash(&ht, "marcador de livro dobrado", "Beatriz");
    inserirNaHash(&ht, "nota ameaçadora", "Daniel");
    inserirNaHash(&ht, "fio de tecido azul", "Ana");
    inserirNaHash(&ht, "guilhotina de cabelo (fiapo)", "Beatriz");

    printf("Bem-vindo(a) a Detective Quest - Capitulo Mestre!\n");
    printf("Objetivo: explore a mansao, colete pistas e acuse o culpado.\n");

    PistaNode *coletadas = NULL;

    /* Começa a exploracao interativa a partir do Hall */
    explorarSalas(hall, &coletadas, &ht);

    /* Fase final: listagem e acusacao */
    verificarSuspeitoFinal(coletadas, &ht);

    /* Liberar memoria e encerrar */
    liberarPistas(coletadas);
    liberarSala(hall);
    liberarHash(&ht);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}
