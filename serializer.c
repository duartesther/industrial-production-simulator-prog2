#include <stdio.h>
#include <string.h>
#include "serializer.h"

/**
 * Adiciona indentação ao arquivo JSON baseado no nível hierárquico
 * @param fp Ponteiro para o arquivo de saída
 * @param nivel Nível de indentação (quantidade de espaços)
 */
static void indent(FILE *fp, int nivel) {
    for (int i = 0; i < nivel; i++) fputs("  ", fp);
}

/**
 * Serializa recursivamente um processo e seus subprocessos para formato JSON
 * @param p Processo a ser serializado
 * @param fp Arquivo de saída onde o JSON será escrito
 * @param nivel Nível atual de indentação (para formatação)
 */
/**
 * Serializa recursivamente um processo e seus subprocessos para formato JSON
 * @param p Processo a ser serializado
 * @param fp Arquivo de saída onde o JSON será escrito
 * @param nivel Nível atual de indentação (para formatação)
 */
void serializar_processo(processo_t *p, FILE *fp, int nivel) {
    // Inicia o objeto JSON do processo
    indent(fp, nivel);
    fprintf(fp, "{\n");
    
    // Adiciona o nome do processo
    indent(fp, nivel);
    fprintf(fp, "  \"nome\": \"%s\",\n", p->nome);
    
    // Calcula o progresso percentual (0-100)
    int progresso = (p->tempo_total > 0) ? 
        ((p->tempo_total - p->tempo_restante) * 100) / p->tempo_total : 0;
    
    // Adiciona o progresso
    indent(fp, nivel);
    fprintf(fp, "  \"progresso\": %d,\n", progresso);
    
    // Determina o status do processo - modificado para priorizar "concluido" quando progresso = 100%
    const char *status = (progresso >= 100 || p->concluido) ? "concluido" :
                       (progresso > 0 ? "executando" : "espera");
    
    // Adiciona o status
    indent(fp, nivel);
    fprintf(fp, "  \"status\": \"%s\"", status);
    
    // Se houver subprocessos, serializa-os recursivamente
    if (p->n_subprocessos > 0) {
        fprintf(fp, ",\n");
        indent(fp, nivel);
        fprintf(fp, "  \"filhos\": [\n");
        
        // Serializa cada subprocesso
        for (int i = 0; i < p->n_subprocessos; i++) {
            serializar_processo(p->subprocessos[i], fp, nivel + 1);
            // Adiciona vírgula entre itens (exceto no último)
            fprintf(fp, i + 1 < p->n_subprocessos ? ",\n" : "\n");
        }
        
        // Fecha o array de subprocessos
        indent(fp, nivel);
        fprintf(fp, "  ]");
    }
    
    // Finaliza o objeto do processo
    fprintf(fp, "\n");
    indent(fp, nivel);
    fprintf(fp, "}");
}

/**
 * Exporta o estado completo da simulação para um arquivo JSON
 * @param fila Ponteiro para a fila de produtos
 * @param rodada Número da rodada atual
 * @param filename Nome do arquivo JSON de saída
 */
void exportar_estado(const fila_t *fila, int rodada, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;  // Retorna se não conseguir abrir o arquivo
    
    // Inicia o objeto JSON principal
    fprintf(fp, "{\n");
    fprintf(fp, "  \"rodada\": %d,\n", rodada);  // Adiciona número da rodada
    fprintf(fp, "  \"produtos\": [\n");          // Inicia array de produtos
    
    // Percorre todos os produtos na fila
    produto_t *p = fila->ini;
    while (p) {
        // Inicia o objeto do produto
        indent(fp, 2);
        fprintf(fp, "{\n");
        
        // Adiciona ID do produto
        indent(fp, 2);
        fprintf(fp, "  \"id\": \"%s\",\n", p->id);
        
        // Adiciona tipo do produto
        indent(fp, 2);
        fprintf(fp, "  \"tipo\": \"%s\",\n", p->tipo);
        
        // Adiciona a árvore de processos do produto
        indent(fp, 2);
        fprintf(fp, "  \"arvore\": ");
        serializar_processo(p->raiz_processo, fp, 3);  // Serializa com nível 3 de indentação
        fprintf(fp, "\n");
        
        // Finaliza o objeto do produto
        indent(fp, 2);
        fprintf(fp, "}");
        
        // Adiciona vírgula entre produtos (exceto no último)
        if (p->prox) fprintf(fp, ",\n");
        else fprintf(fp, "\n");
        
        p = p->prox;
    }
    
    // Finaliza o JSON principal
    fprintf(fp, "  ]\n");  // Fecha array de produtos
    fprintf(fp, "}\n");    // Fecha objeto principal
    fclose(fp);            // Fecha o arquivo
}