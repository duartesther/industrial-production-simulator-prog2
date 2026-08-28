/**
 * @file fila.h
 * @brief Interface para fila FIFO de produtos
 * 
 * Define operações para manipulação de uma fila dinâmica de produtos,
 * usada para gerenciar a ordem de processamento na simulação.
 */

#ifndef FILA_H
#define FILA_H

#include "produto.h"

/**
 * @brief Estrutura da fila (FIFO)
 * 
 * Mantém ponteiros para o início e fim da lista encadeada
 */
typedef struct {
    produto_t *ini; ///< Primeiro elemento da fila
    produto_t *fim; ///< Último elemento da fila
} fila_t;

/* ---------- Operações da Fila ---------- */
void inicializar_fila(fila_t *f);
void enfileirar(fila_t *f, produto_t *p);
produto_t* desenfileirar(fila_t *f);
void liberar_fila(fila_t *f);
int contar_produtos(const fila_t *f);

#endif