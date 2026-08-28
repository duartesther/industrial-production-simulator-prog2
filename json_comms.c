/* src/json_comms.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_comms.h"
#include "cJSON.h"
#include "produto.h"
#include "processo.h"
#include "fila.h"

/**
 * Converte uma estrutura processo_t para um objeto JSON
 * @param p Ponteiro para o processo a ser convertido
 * @return Objeto JSON representando o processo e seus subprocessos
 */
static cJSON* processo_to_json(const processo_t *p) {
    // Cria objeto JSON base
    cJSON *o = cJSON_CreateObject();
    
    // Adiciona propriedades básicas do processo
    cJSON_AddStringToObject(o, "nome", p->nome);
    
    // Calcula progresso percentual (0-100)
    int prog = (p->tempo_total - p->tempo_restante) * 100 / p->tempo_total;
    cJSON_AddNumberToObject(o, "progresso", prog);
    
    // Determina status com base no estado do processo
    const char *st = p->concluido ? "concluido"
                 : (p->tempo_restante < p->tempo_total ? "executando" : "espera");
    cJSON_AddStringToObject(o, "status", st);
    
    // Processa subprocessos recursivamente (se existirem)
    if (p->n_subprocessos) {
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < p->n_subprocessos; i++)
            cJSON_AddItemToArray(arr, processo_to_json(p->subprocessos[i]));
        cJSON_AddItemToObject(o, "filhos", arr);
    }
    return o;
}

/**
 * Exporta o estado atual da simulação para JSON
 * @param rodada Número da rodada atual
 * @param fila Ponteiro para a fila de produtos
 * @return String JSON contendo o estado completo (deve ser liberada pelo caller)
 */
char* exportar_estado_json(int rodada, const fila_t *fila) {
    // Cria objeto JSON raiz
    cJSON *r = cJSON_CreateObject();
    cJSON_AddNumberToObject(r, "rodada", rodada);
    
    // Cria array de produtos
    cJSON *pa = cJSON_CreateArray();
    
    // Percorre todos os produtos na fila
    for (produto_t *p = fila->ini; p; p = p->prox) {
        cJSON *pj = cJSON_CreateObject();
        // Adiciona propriedades básicas do produto
        cJSON_AddStringToObject(pj, "id",   p->id);
        cJSON_AddStringToObject(pj, "tipo", p->tipo);
        
        // Adiciona a árvore de processos do produto
        cJSON_AddItemToObject(pj, "processos", processo_to_json(p->raiz_processo));
        cJSON_AddItemToArray(pa, pj);
    }
    
    // Adiciona o array de produtos ao objeto raiz
    cJSON_AddItemToObject(r, "produtos", pa);
    
    // Converte para string JSON (sem formatação)
    char *out = cJSON_PrintUnformatted(r);
    
    // Libera memória da estrutura JSON
    cJSON_Delete(r);
    return out;
}

/**
 * Importa e processa comandos de um arquivo JSON
 * @param fila Ponteiro para a fila de produtos que será modificada se necessário
 */
void importar_comando(fila_t *fila) {
    // Abre arquivo de comando
    FILE *f = fopen("comando.json","r");
    if (!f) return;  // Nenhum comando se o arquivo não existir
    
    // Lê todo o conteúdo do arquivo
    fseek(f,0,SEEK_END);
    long n = ftell(f); fseek(f,0,SEEK_SET);
    char *b = malloc(n+1);
    fread(b,1,n,f); b[n]=0; fclose(f);
    
    // Parse do JSON
    cJSON *r = cJSON_Parse(b); free(b);
    if (!r) return;  // JSON inválido
    
    // Verifica tipo de comando
    cJSON *a = cJSON_GetObjectItem(r,"acao");
    if (cJSON_IsString(a) && strcmp(a->valuestring,"adicionar_produto")==0) {
        // Comando para adicionar novo produto
        cJSON *t = cJSON_GetObjectItem(r,"tipo");
        if (cJSON_IsString(t)) {
            // Cria novo produto
            produto_t *p = malloc(sizeof(*p));
            strcpy(p->id, gerar_id());  // Gera ID único
            strcpy(p->tipo, t->valuestring);  // Copia tipo
            
            // Cria árvore de processos específica para o tipo
            p->raiz_processo = criar_arvore_por_tipo(t->valuestring);
            p->prox=NULL;
            
            // Adiciona à fila se a árvore foi criada com sucesso
            if (p->raiz_processo) enfileirar(fila,p);
        }
    }
    
    // Libera memória do JSON
    cJSON_Delete(r);
}