/**
 * produto.c - Implementação das operações de produtos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "produto.h"

static int contador_id = 1; // Contador interno para geração de IDs

/**
 * Gera um ID único no formato PXXX
 */
char* gerar_id(void) {
    static char id[10];
    snprintf(id, sizeof(id), "P%03d", contador_id++);
    return id;
}

/**
 * Reinicia o contador de IDs
 */
void resetar_contador_id(void) {
    contador_id = 1;
}

/**
 * Factory method para criar árvore de processos
 */
processo_t* criar_arvore_por_tipo(const char *tipo) {
    if (strcmp(tipo, "ventilador") == 0) return criar_ventilador();
    if (strcmp(tipo, "exaustor") == 0) return criar_exaustor();
    if (strcmp(tipo, "aspirador") == 0) return criar_aspirador();
    return NULL;
}