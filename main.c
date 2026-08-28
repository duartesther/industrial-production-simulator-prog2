/**
 * main.c - Programa principal do simulador industrial
 *
 * Controle completo da simulação com:
 * - Processamento de produtos (um processo por rodada)
 * - Desenfileiramento com mensagens específicas
 * - Persistência do estado
 * - Modo linha de comando e interativo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "processo.h"
#include "produto.h"
#include "fila.h"
#include "persistencia.h"
#include "serializer.h"

#define CLEAR() printf("\033[H\033[J")

/**
 * Mostra recursivamente um processo e seus subprocessos
 */
static void mostrar_processo_rec(int nivel, const processo_t *pr)
{
    int prog = (pr->tempo_total > 0)
                   ? ((pr->tempo_total - pr->tempo_restante) * 100) / pr->tempo_total
                   : 100;

    for (int i = 0; i < nivel; i++)
        printf("  ");
    const char *cor = pr->concluido ? "\033[32m" : (prog > 0 ? "\033[33m" : "\033[37m");
    printf("%s[%3d%%] %s\033[0m\n", cor, prog, pr->nome);

    for (int i = 0; i < pr->n_subprocessos; i++)
        mostrar_processo_rec(nivel + 1, pr->subprocessos[i]);
}

/**
 * Mostra o andamento atual de todos os produtos na fila
 */
static void mostrar_andamento(const fila_t *fila)
{
    printf("\n=== ANDAMENTO DA SIMULACAO ===\n");
    produto_t *p = fila->ini;
    while (p)
    {
        printf("Produto %s %s\n", p->tipo, p->id);
        mostrar_processo_rec(1, p->raiz_processo);
        printf("\n");
        p = p->prox;
    }
    printf("==============================\n");
}

/**
 * Executa uma rodada de processamento (um processo por vez)
 */
/**
 * Executa uma rodada de processamento (uma etapa por produto)
 */
void executar_rodada(fila_t *fila)
{
    produto_t *cur = fila->ini;
    bool algum_progresso = false;

    // Percorre todos os produtos
    while (cur)
    {
        // Processa apenas uma etapa do produto atual
        if (!verificar_conclusao(cur->raiz_processo))
        {
            processar_etapa_unica(cur->raiz_processo, stdout);
            algum_progresso = true;
        }
        cur = cur->prox;
    }

    if (!algum_progresso)
    {
        printf("Todos os processos estao concluidos!\n");
    }

    mostrar_andamento(fila);
}

/**
 * Remove o primeiro produto concluído da fila
 */
int desenfileirar_produto_concluido(fila_t *fila)
{
    produto_t *p = fila->ini;

    if (!p)
    {
        printf("Fila vazia. Nenhum produto para desenfileirar.\n");
        return 0;
    }

    if (verificar_conclusao(p->raiz_processo))
    {
        produto_t *removido = desenfileirar(fila);
        printf("REMOVER %s %s\n", removido->tipo, removido->id);

        salvar_estado(fila);
        salvar_fila_binaria(fila);
        salvar_estado_json(fila, 0, "estado.json");

        liberar_processo(removido->raiz_processo);
        free(removido);
        return 1;
    }

    printf("Nenhum produto concluido para desenfileirar\n");
    return 0;
}

/**
 * Função principal do programa
 */
int main(int argc, char *argv[])
{
    fila_t fila;
    inicializar_fila(&fila);
    carregar_fila_binaria(&fila);

    // Modo linha de comando
    if (argc >= 2)
    {
        if (strcmp(argv[1], "adicionar") == 0 && argc == 4)
        {
            produto_t *p = malloc(sizeof(*p));
            strcpy(p->id, argv[3]);
            strcpy(p->tipo, argv[2]);
            p->raiz_processo = criar_arvore_por_tipo(argv[2]);
            p->prox = NULL;

            if (p->raiz_processo)
            {
                enfileirar(&fila, p);
                salvar_fila_binaria(&fila);
                salvar_estado_json(&fila, 0, "estado.json");
            }
            else
            {
                free(p);
            }

            liberar_fila(&fila);
            return 0;
        }
        else if (strcmp(argv[1], "rodada") == 0)
        {
            executar_rodada(&fila);
            salvar_estado_json(&fila, 0, "estado.json");
            salvar_fila_binaria(&fila);
            liberar_fila(&fila);
            return 0;
        }
        else if (strcmp(argv[1], "desenfileirar") == 0)
        {
            int removido = desenfileirar_produto_concluido(&fila);
            if (removido)
            {
                salvar_estado(&fila);
                salvar_fila_binaria(&fila);
                salvar_estado_json(&fila, 0, "estado.json");
            }
            liberar_fila(&fila);
            return 0;
        }
        else if (strcmp(argv[1], "relatorio") == 0)
        {
            salvar_estado(&fila);
            liberar_fila(&fila);
            return 0;
        }
        else if (strcmp(argv[1], "reiniciar") == 0)
        {
            liberar_fila(&fila);
            inicializar_fila(&fila);
            remove("fila.bin");
            remove("estado.json");
            resetar_contador_id();
            return 0;
        }
        else
        {
            fprintf(stderr, "Comando invalido\n");
            liberar_fila(&fila);
            return 1;
        }
    }

    // Modo interativo
    int rodada = 1, opcao;
    do
    {
        CLEAR();
        printf("==== SIMULADOR INDUSTRIAL ====\n");
        printf("Rodada %d\n\n", rodada);

        produto_t *p = fila.ini;
        if (!p)
        {
            printf("  (nenhum produto na fila)\n");
        }
        else
        {
            while (p)
            {
                printf(" - %s %s\n", p->tipo, p->id);
                p = p->prox;
            }
        }

        printf("\n1) Ventilador    2) Exaustor    3) Aspirador\n");
        printf("4) Executar Rodada    5) Desenfileirar Produto Concluido\n");
        printf("6) Gerar Relatorio    7) Reiniciar Simulacao\n");
        printf("0) Sair\nEscolha: ");

        if (scanf("%d", &opcao) != 1)
        {
            opcao = 0;
        }
        getchar();

        switch (opcao)
        {
        case 1:
        case 2:
        case 3:
        {
            const char *tipos[] = {"ventilador", "exaustor", "aspirador"};
            produto_t *novo = malloc(sizeof(*novo));

            strcpy(novo->id, gerar_id());
            strcpy(novo->tipo, tipos[opcao - 1]);
            novo->raiz_processo = criar_arvore_por_tipo(tipos[opcao - 1]);
            novo->prox = NULL;

            if (novo->raiz_processo)
            {
                enfileirar(&fila, novo);
            }
            else
            {
                free(novo);
                printf("Erro ao criar arvore de processos!\n");
            }
            break;
        }
        case 4:
            executar_rodada(&fila);
            salvar_estado_json(&fila, rodada, "estado.json");
            salvar_fila_binaria(&fila);
            rodada++;
            printf("Pressione ENTER...");
            getchar();
            break;
        case 5:
            if (desenfileirar_produto_concluido(&fila))
            {
                salvar_estado(&fila);
                salvar_fila_binaria(&fila);
                salvar_estado_json(&fila, rodada, "estado.json");
            }
            printf("Pressione ENTER...");
            getchar();
            break;
        case 6:
            salvar_estado(&fila);
            printf("Relatorio gerado em relatorio.txt\nPressione ENTER...");
            getchar();
            break;
        case 7:
            liberar_fila(&fila);
            inicializar_fila(&fila);
            remove("fila.bin");
            remove("estado.json");
            resetar_contador_id();
            rodada = 1;
            printf("Simulacao reiniciada.\nPressione ENTER...");
            getchar();
            break;
        }
    } while (opcao != 0);

    salvar_fila_binaria(&fila);
    liberar_fila(&fila);
    return 0;
}