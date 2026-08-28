#include <stdlib.h>
#include <string.h>
#include "processo.h"

/**
 * Cria um novo processo com nome e tempo de execução especificados
 */
processo_t* criar_processo(const char *nome, int tempo) {
    processo_t *p = malloc(sizeof(*p));
    strcpy(p->nome, nome);
    p->tempo_total = tempo;
    p->tempo_restante = tempo;
    p->n_subprocessos = 0;
    p->subprocessos = NULL;
    p->concluido = false;
    return p;
}

/**
 * Adiciona um subprocesso a um processo pai
 */
void adicionar_subprocesso(processo_t *pai, processo_t *filho) {
    pai->n_subprocessos++;
    pai->subprocessos = realloc(pai->subprocessos, pai->n_subprocessos * sizeof(processo_t*));
    pai->subprocessos[pai->n_subprocessos-1] = filho;
}

/**
 * Executa um ciclo de processamento em um processo e seus subprocessos
 */
/**
 * Executa processamento completo (todas etapas possíveis)
 */
bool processar(processo_t *p, FILE *log) {
    if (p->concluido) return true;
    
    // Processa primeiro todos os subprocessos
    for (int i = 0; i < p->n_subprocessos; i++) {
        if (!processar(p->subprocessos[i], log)) return false;
    }

    // Processa o próprio processo se todos subprocessos estiverem prontos
    if (p->tempo_restante > 0) {
        p->tempo_restante--;
        
        if (p->tempo_restante == 0) {
            p->concluido = true;
            if (log) fprintf(log, "Concluído: %s\n", p->nome);
        } else {
            if (log) fprintf(log, "Processando: %s (%d/%d)\n", p->nome, 
                           p->tempo_total - p->tempo_restante, p->tempo_total);
        }
        return p->concluido;
    }
    
    p->concluido = true;
    if (log) fprintf(log, "Concluído: %s\n", p->nome);
    return true;
}

/**
 * Verifica se um processo e todos seus subprocessos estão concluídos
 */
bool verificar_conclusao(processo_t *p) {
    if (!p->concluido) return false;
    
    for (int i = 0; i < p->n_subprocessos; i++) {
        if (!verificar_conclusao(p->subprocessos[i])) return false;
    }
    return true;
}

/**
 * Libera toda a memória alocada para um processo e seus subprocessos
 */
void liberar_processo(processo_t *p) {
    for (int i = 0; i < p->n_subprocessos; i++) {
        liberar_processo(p->subprocessos[i]);
    }
    free(p->subprocessos);
    free(p);
}

/**
 * Processa apenas uma etapa do processo (a primeira encontrada que pode ser executada)
 * @return true se processou alguma etapa, false caso contrário
 */
bool processar_etapa_unica(processo_t *p, FILE *log) {
    if (p->concluido) return false;

    // Primeiro tenta processar os subprocessos
    for (int i = 0; i < p->n_subprocessos; i++) {
        if (processar_etapa_unica(p->subprocessos[i], log)) {
            return true;
        }
    }

    // Se todos os subprocessos estão concluídos ou não há subprocessos,
    // processa este processo se ainda não estiver concluído
    if (p->tempo_restante > 0) {
        p->tempo_restante--;
        
        if (p->tempo_restante == 0) {
            p->concluido = true;
            if (log) fprintf(log, "Concluído: %s\n", p->nome);
        } else {
            if (log) fprintf(log, "Processando: %s (%d/%d)\n", p->nome, 
                           p->tempo_total - p->tempo_restante, p->tempo_total);
        }
        return true;
    }

    return false;
}

/* ========== FÁBRICAS DE PROCESSOS ========== */

processo_t* criar_ventilador() {
    processo_t *montagem = criar_processo("Montagem Final", 2);
    processo_t *motor = criar_processo("Motor", 2);
    processo_t *bobinagem = criar_processo("Bobinagem", 2);
    processo_t *teste = criar_processo("Teste Eletrico", 1);
    processo_t *helice = criar_processo("Helice", 1);
    processo_t *carcaca = criar_processo("Carcaca", 1);
    processo_t *injecao = criar_processo("Injecao Plastica", 1);

    adicionar_subprocesso(bobinagem, teste);
    adicionar_subprocesso(motor, bobinagem);
    adicionar_subprocesso(carcaca, injecao);
    adicionar_subprocesso(montagem, motor);
    adicionar_subprocesso(montagem, helice);
    adicionar_subprocesso(montagem, carcaca);
    
    return montagem;
}

processo_t* criar_exaustor() {
    processo_t *montagem = criar_processo("Montagem Final", 2);
    processo_t *motor = criar_processo("Motor", 2);
    processo_t *bobinagem = criar_processo("Bobinagem", 2);
    processo_t *teste = criar_processo("Teste Eletrico", 1);
    processo_t *carcaca = criar_processo("Carcaca", 1);
    processo_t *injecao = criar_processo("Injecao Plastica", 1);

    adicionar_subprocesso(bobinagem, teste);
    adicionar_subprocesso(motor, bobinagem);
    adicionar_subprocesso(carcaca, injecao);
    adicionar_subprocesso(montagem, motor);
    adicionar_subprocesso(montagem, carcaca);
    
    return montagem;
}

processo_t* criar_aspirador() {
    processo_t *montagem = criar_processo("Montagem Final", 2);
    processo_t *motor = criar_processo("Motor", 2);
    processo_t *bobinagem = criar_processo("Bobinagem", 2);
    processo_t *teste = criar_processo("Teste Eletrico", 1);
    processo_t *helice = criar_processo("Helice", 1);
    processo_t *carcaca = criar_processo("Carcaca", 1);
    processo_t *injecao = criar_processo("Injecao Plastica", 1);
    processo_t *filtro = criar_processo("Filtro", 1);

    adicionar_subprocesso(bobinagem, teste);
    adicionar_subprocesso(motor, bobinagem);
    adicionar_subprocesso(carcaca, injecao);
    adicionar_subprocesso(carcaca, filtro);
    adicionar_subprocesso(montagem, motor);
    adicionar_subprocesso(montagem, helice);
    adicionar_subprocesso(montagem, carcaca);
    
    return montagem;
}