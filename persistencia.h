/**
 * @file persistencia.h
 * @brief Interface para persistência do estado da simulação
 * 
 * Define funções para salvar/carregar o estado em diferentes formatos:
 * - Binário (fila.bin): Para reconstrução exata do estado
 * - JSON (estado.json): Para integração com frontend
 * - Texto (relatorio.txt): Para inspeção humana
 */

#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "fila.h"

/**
 * @brief Salva relatório textual do estado atual
 * @param f Ponteiro para a fila de produtos
 */
void salvar_estado(const fila_t *f);

/**
 * @brief Persiste o estado completo em arquivo binário
 * @param f Ponteiro para a fila de produtos
 */
void salvar_fila_binaria(const fila_t *f);

/**
 * @brief Carrega estado a partir de arquivo binário
 * @param f Ponteiro para a fila onde os dados serão carregados
 */
void carregar_fila_binaria(fila_t *f);

/**
 * @brief Exporta estado para arquivo JSON formatado
 * @param f Ponteiro para a fila de produtos
 * @param rodada Número da rodada atual
 * @param filename Nome do arquivo de saída
 */
void salvar_estado_json(const fila_t *f, int rodada, const char *filename);

#endif