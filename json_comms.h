/* src/json_comms.h */
#ifndef JSON_COMMS_H
#define JSON_COMMS_H

/* 
 * Interface para comunicação via JSON entre o simulador industrial e o front-end
 * 
 * Este header define as funções para:
 * - Exportar o estado atual da simulação para formato JSON
 * - Importar comandos recebidos em formato JSON
 */

#include "fila.h"  // Dependência necessária para o tipo fila_t

/**
 * Exporta o estado atual da simulação para uma string JSON
 * @param rodada Número da rodada atual da simulação
 * @param fila Ponteiro para a fila de produtos (estado atual)
 * @return String JSON alocada dinamicamente contendo:
 *         - Número da rodada
 *         - Lista de produtos com seus processos e subprocessos
 *         - Status e progresso de cada processo
 * @note O caller é responsável por liberar a memória da string retornada
 */
char* exportar_estado_json(int rodada, const fila_t *fila);

/**
 * Importa e executa comandos de um arquivo JSON
 * @param fila Ponteiro para a fila de produtos que será modificada
 * 
 * Lê o arquivo "comando.json" e processa as ações solicitadas:
 * - Atualmente suporta apenas o comando "adicionar_produto"
 * - Cria um novo produto do tipo especificado
 * - Gera automaticamente um ID único
 * - Constrói a árvore de processos apropriada para o tipo
 * - Adiciona o produto à fila de processamento
 * 
 * @note O arquivo deve estar no formato:
 *       {"acao":"adicionar_produto","tipo":"tipo_do_produto"}
 */
void importar_comando(fila_t *fila);

#endif // JSON_COMMS_H