#ifndef SERIALIZER_H  // Header guard para evitar inclusão múltipla
#define SERIALIZER_H

/* Inclusão das dependências necessárias */
#include "fila.h"     // Para o tipo fila_t
#include "processo.h" // Para o tipo processo_t
#include "produto.h"  // Para o tipo produto_t

/**
 * @brief Serializa um processo e seus subprocessos em formato JSON
 * 
 * @param p Ponteiro para o processo a ser serializado
 * @param fp Ponteiro para o arquivo onde o JSON será escrito
 * @param nivel Nível de indentação atual (para formatação)
 * 
 * A função gera uma representação JSON completa do processo, incluindo:
 * - Nome do processo
 * - Progresso atual (0-100%)
 * - Status (espera, executando ou concluido)
 * - Subprocessos (se existirem)
 */
void serializar_processo(processo_t *p, FILE *fp, int nivel);

/**
 * @brief Exporta o estado completo da simulação para um arquivo JSON
 * 
 * @param fila Ponteiro para a fila de produtos a ser serializada
 * @param rodada Número da rodada atual da simulação
 * @param filename Nome do arquivo de saída JSON
 * 
 * A função gera um arquivo JSON contendo:
 * - Número da rodada atual
 * - Lista de todos os produtos na fila
 * - Para cada produto: ID, tipo e árvore de processos completa
 * 
 * O formato do JSON segue o padrão:
 * {
 *   "rodada": 1,
 *   "produtos": [
 *     {
 *       "id": "P001",
 *       "tipo": "ventilador",
 *       "arvore": { ... }
 *     },
 *     ...
 *   ]
 * }
 */
void exportar_estado(const fila_t *fila, int rodada, const char *filename);

#endif // SERIALIZER_H