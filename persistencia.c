#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "persistencia.h"
#include "serializer.h"

/**
 * Salva recursivamente um processo e seus subprocessos em formato binário
 * @param out Arquivo de saída onde os dados serão escritos
 * @param p Processo a ser salvo
 */
static void salvar_processo_rec(FILE *out, const processo_t *p) {
    // Salva o tamanho e o conteúdo do nome do processo
    int nome_len = strlen(p->nome) + 1;  // +1 para o caractere nulo
    fwrite(&nome_len, sizeof(int), 1, out);
    fwrite(p->nome, sizeof(char), nome_len, out);
    
    // Salva os tempos e quantidade de subprocessos
    fwrite(&p->tempo_total, sizeof(int), 1, out);
    fwrite(&p->tempo_restante, sizeof(int), 1, out);
    fwrite(&p->n_subprocessos, sizeof(int), 1, out);
    
    // Salva recursivamente cada subprocesso
    for (int i = 0; i < p->n_subprocessos; i++)
        salvar_processo_rec(out, p->subprocessos[i]);
    
    // Salva o status de conclusão
    fwrite(&p->concluido, sizeof(bool), 1, out);
}

/**
 * Carrega recursivamente um processo e seus subprocessos de arquivo binário
 * @param in Arquivo de entrada contendo os dados
 * @return Ponteiro para o processo carregado ou NULL em caso de erro
 */
static processo_t* carregar_processo_rec(FILE *in) {
    int nome_len;
    // Lê o tamanho do nome do processo
    if (fread(&nome_len, sizeof(int), 1, in) != 1) return NULL;
    
    // Lê o nome do processo
    char nome[50];
    fread(nome, sizeof(char), nome_len, in);
    
    // Lê os tempos e quantidade de subprocessos
    int tempo_total, tempo_restante, n_sub;
    fread(&tempo_total, sizeof(int), 1, in);
    fread(&tempo_restante, sizeof(int), 1, in);
    fread(&n_sub, sizeof(int), 1, in);
    
    // Cria o processo base
    processo_t *p = criar_processo(nome, tempo_total);
    p->tempo_restante = tempo_restante;
    
    // Carrega recursivamente cada subprocesso
    for (int i = 0; i < n_sub; i++) {
        processo_t *c = carregar_processo_rec(in);
        if (c) adicionar_subprocesso(p, c);
    }
    
    // Lê o status de conclusão
    bool concluido;
    fread(&concluido, sizeof(bool), 1, in);
    p->concluido = concluido;
    
    return p;
}

/**
 * Salva toda a fila de produtos em arquivo binário
 * @param f Ponteiro para a fila a ser salva
 */
void salvar_fila_binaria(const fila_t *f) {
    FILE *out = fopen("fila.bin", "wb");
    if (!out) return;
    
    // Salva a quantidade total de produtos
    int count = contar_produtos(f);
    fwrite(&count, sizeof(int), 1, out);
    
    // Para cada produto na fila...
    for (produto_t *p = f->ini; p; p = p->prox) {
        // Salva ID do produto
        int id_len = strlen(p->id) + 1;
        fwrite(&id_len, sizeof(int), 1, out);
        fwrite(p->id, sizeof(char), id_len, out);
        
        // Salva tipo do produto
        int tipo_len = strlen(p->tipo) + 1;
        fwrite(&tipo_len, sizeof(int), 1, out);
        fwrite(p->tipo, sizeof(char), tipo_len, out);
        
        // Salva a árvore de processos do produto
        salvar_processo_rec(out, p->raiz_processo);
    }
    fclose(out);
}

/**
 * Carrega uma fila de produtos de arquivo binário
 * @param f Ponteiro para a fila onde os dados serão carregados
 */
void carregar_fila_binaria(fila_t *f) {
    FILE *in = fopen("fila.bin", "rb");
    if (!in) return;
    
    // Lê a quantidade total de produtos
    int count;
    if (fread(&count, sizeof(int), 1, in) != 1) { fclose(in); return; }
    
    // Para cada produto no arquivo...
    for (int i = 0; i < count; i++) {
        // Lê ID do produto
        int id_len; fread(&id_len, sizeof(int), 1, in);
        char id[64]; fread(id, sizeof(char), id_len, in);
        
        // Lê tipo do produto
        int tipo_len; fread(&tipo_len, sizeof(int), 1, in);
        char tipo[64]; fread(tipo, sizeof(char), tipo_len, in);
        
        // Carrega a árvore de processos do produto
        processo_t *raiz = carregar_processo_rec(in);
        if (!raiz) break;
        
        // Cria e enfileira o produto
        produto_t *p = malloc(sizeof(*p));
        strcpy(p->id, id);
        strcpy(p->tipo, tipo);
        p->raiz_processo = raiz;
        p->prox = NULL;
        enfileirar(f, p);
    }
    fclose(in);
}

/**
 * Imprime recursivamente os processos em formato de texto
 * @param out Arquivo de saída
 * @param p Processo a ser impresso
 * @param nivel Nível de indentação (para hierarquia)
 */
static void imprimir_processos(FILE *out, const processo_t *p, int nivel) {
    // Aplica indentação conforme o nível
    for (int i = 0; i < nivel; i++) fprintf(out, "  ");
    
    // Calcula progresso e determina status
    int progresso = (p->tempo_total > 0) ?
        ((p->tempo_total - p->tempo_restante) * 100) / p->tempo_total : 0;
    
    const char *status = p->concluido ? "concluido" :
                       (p->tempo_restante < p->tempo_total ? "executando" : "espera");
    
    // Imprime informações do processo
    fprintf(out, "- %s: %d%% (%s)\n", p->nome, progresso, status);
    
    // Imprime recursivamente os subprocessos
    for (int i = 0; i < p->n_subprocessos; i++)
        imprimir_processos(out, p->subprocessos[i], nivel + 1);
}

/**
 * Salva um relatório textual do estado atual da simulação
 * @param f Ponteiro para a fila de produtos
 */
void salvar_estado(const fila_t *f) {
    FILE *out = fopen("relatorio.txt", "w");
    if (!out) return;
    
    // Cabeçalho do relatório
    fprintf(out, "=== Relatório da Simulação ===\n");
    fprintf(out, "Produtos na fila: %d\n\n", contar_produtos(f));
    
    // Para cada produto na fila...
    for (produto_t *p = f->ini; p; p = p->prox) {
        // Imprime informações básicas do produto
        fprintf(out, "Produto %s (%s):\n", p->id, p->tipo);
        // Imprime sua árvore de processos
        imprimir_processos(out, p->raiz_processo, 1);
        fprintf(out, "\n");
    }
    fclose(out);
}

/**
 * Salva o estado atual em formato JSON
 * @param f Ponteiro para a fila de produtos
 * @param rodada Número da rodada atual
 * @param filename Nome do arquivo de saída
 */
void salvar_estado_json(const fila_t *f, int rodada, const char *filename) {
    // Delega a operação para a função exportar_estado do serializer
    exportar_estado(f, rodada, filename);
}