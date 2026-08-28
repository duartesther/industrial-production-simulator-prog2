#include <stdlib.h>
#include "fila.h"

/* Inicializa uma fila vazia, definindo os ponteiros de início e fim como NULL */
void inicializar_fila(fila_t *f) {
    f->ini = f->fim = NULL;
}

/* Adiciona um novo produto ao final da fila */
void enfileirar(fila_t *f, produto_t *p) {
    p->prox = NULL;  // Garante que o novo produto não aponta para outro
    
    // Se a fila está vazia, o novo produto será o primeiro e o último
    if (f->fim == NULL) {
        f->ini = f->fim = p;
    } else {
        // Se não está vazia, adiciona no final e atualiza o ponteiro de fim
        f->fim->prox = p;
        f->fim = p;
    }
}

/* Remove e retorna o produto no início da fila (política FIFO) */
produto_t* desenfileirar(fila_t *f) {
    produto_t *p = f->ini;  // Pega o primeiro produto da fila
    
    // Se a fila está vazia, retorna NULL
    if (p == NULL) return NULL;
    
    // Atualiza o início da fila para o próximo produto
    f->ini = p->prox;
    
    // Se a fila ficou vazia após a remoção, atualiza o ponteiro de fim
    if (f->ini == NULL) {
        f->fim = NULL;
    }
    
    return p;  // Retorna o produto removido
}

/* Libera toda a memória alocada para a fila e seus produtos */
void liberar_fila(fila_t *f) {
    produto_t *p = f->ini;
    
    // Percorre todos os produtos da fila
    while (p) {
        produto_t *temp = p;  // Guarda referência ao produto atual
        p = p->prox;         // Avança para o próximo produto
        
        // Libera a árvore de processos do produto e depois o próprio produto
        liberar_processo(temp->raiz_processo);
        free(temp);
    }
    
    // Garante que os ponteiros da fila estão NULL após a liberação
    f->ini = f->fim = NULL;
}

/* Conta quantos produtos existem na fila */
int contar_produtos(const fila_t *f) {
    int count = 0;
    
    // Percorre a fila incrementando o contador para cada produto
    for (produto_t *p = f->ini; p; p = p->prox) count++;
    
    return count;  // Retorna o total de produtos
}