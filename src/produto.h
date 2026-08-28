/**
 * produto.h - Definição de produtos industriais
 * Contém a estrutura de produtos e funções relacionadas
 */

#ifndef PRODUTO_H
#define PRODUTO_H

#include "processo.h"

typedef struct produto {
    char id[10];               // Identificador único (P001, P002, etc)
    char tipo[20];             // Tipo do produto (ventilador, exaustor, etc)
    processo_t *raiz_processo; // Árvore de processos do produto
    struct produto *prox;      // Próximo produto na fila
} produto_t;

// Gera ID sequencial para novos produtos
char* gerar_id(void);

// Reinicia o contador de IDs
void resetar_contador_id(void);

// Cria árvore de processos específica para cada tipo
processo_t* criar_arvore_por_tipo(const char *tipo);

#endif