/**
 * @file processo.h
 * @brief Interface para manipulação de processos industriais
 * 
 * Define a estrutura de processos industriais e operações relacionadas,
 * incluindo criação, processamento e liberação de recursos.
 */

#ifndef PROCESSO_H
#define PROCESSO_H

#include <stdio.h>    // Para o tipo FILE usado nas funções
#include <stdbool.h>  // Para o tipo bool

/**
 * @brief Estrutura que representa um processo industrial
 * 
 * Cada processo contém informações sobre uma etapa de produção,
 * incluindo seu estado atual e subprocessos relacionados.
 */
typedef struct processo {
    char nome[50];               ///< Nome descritivo da etapa (ex: "Montagem Final")
    int tempo_total;             ///< Duração total necessária para a etapa (em rodadas)
    int tempo_restante;          ///< Tempo restante para conclusão (em rodadas)
    struct processo **subprocessos; ///< Vetor dinâmico de ponteiros para subprocessos
    int n_subprocessos;          ///< Quantidade de subprocessos diretos
    bool concluido;              ///< Flag indicando se a etapa foi finalizada
} processo_t;

/* ---------- Operações Básicas ---------- */

/**
 * @brief Cria um novo processo industrial
 * @param nome Nome descritivo do processo
 * @param tempo Duração total do processo (em rodadas)
 * @return Ponteiro para o novo processo alocado
 */
processo_t* criar_processo(const char *nome, int tempo);

/**
 * @brief Adiciona um subprocesso a um processo pai
 * @param pai Processo que receberá o subprocesso
 * @param filho Subprocesso a ser adicionado
 */
void adicionar_subprocesso(processo_t *pai, processo_t *filho);

/**
 * @brief Executa processamento completo (todas etapas possíveis)
 * @param p Processo a ser processado
 * @param log Arquivo para registro de mensagens (NULL para não registrar)
 * @return true se o processo e todos subprocessos estão concluídos
 */
bool processar(processo_t *p, FILE *log);

/**
 * @brief Libera todos os recursos alocados para um processo
 * @param p Processo a ser liberado
 */
void liberar_processo(processo_t *p);

/**
 * @brief Verifica se um processo e todos seus subprocessos estão concluídos
 * @param p Processo a verificar
 * @return true se completamente concluído, false caso contrário
 */
bool verificar_conclusao(processo_t *p);

/**
 * @brief Processa apenas uma etapa do processo (a primeira executável)
 * @param p Processo a ser processado
 * @param log Arquivo para registro de mensagens (NULL para não registrar)
 * @return true se processou alguma etapa, false caso contrário
 */
bool processar_etapa_unica(processo_t *p, FILE *log);

/* ---------- Modelos de Produtos ---------- */

/**
 * @brief Cria árvore de processos para um ventilador
 * @return Ponteiro para o processo raiz (Montagem Final)
 */
processo_t* criar_ventilador();

/**
 * @brief Cria árvore de processos para um exaustor
 * @return Ponteiro para o processo raiz (Montagem Final)
 */
processo_t* criar_exaustor();

/**
 * @brief Cria árvore de processos para um aspirador
 * @return Ponteiro para o processo raiz (Montagem Final)
 */
processo_t* criar_aspirador();

#endif // PROCESSO_H   