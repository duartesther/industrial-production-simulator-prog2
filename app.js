/* app.js - Aplicação Principal do Simulador Industrial */
/* global io, Vue */

/**
 * Instância Principal da Aplicação Vue.js
 * Controla toda a interface de simulação industrial
 */
new Vue({
    el: '#simulacao',

    data: {
        produtos: [],
        novoTipo: 'ventilador',
        socket: null,
        toasts: [],
        paused: false,
        animationFrame: null,
        showCompletionModal: false,
        productionCount: {},
        productCounter: 1,
        allCompleted: false // Nova flag para controlar se todos estão completos
    },

    created() {
        this.inicializarSocket();
        this.fetchEstado();
        this.startProgressAnimation();
    },

    methods: {
        inicializarSocket() {
            this.socket = io();
            this.socket.on('connect', () => {
                console.log('Conexão WebSocket estabelecida');
            });
            this.socket.on('disconnect', () => {
                console.log('Conexão WebSocket perdida');
            });
            this.socket.on('atualizacao', (data) => {
                if (!this.paused) {
                    this.processarDados(data);
                }
            });
        },

        async fetchEstado() {
            try {
                const res = await fetch('/api/estado');
                const data = await res.json();
                this.processarDados(data);
                this.atualizarContador(data.produtos);
            } catch (error) {
                console.error('Erro ao buscar estado:', error);
                this.toast('⚠️ Erro ao carregar estado');
            }
        },

        processarDados(data) {
            if (data && data.produtos) {
                this.produtos = data.produtos.map(p => ({
                    id: p.id || this.gerarId(),
                    tipo: p.tipo,
                    nome_formatado: this.formatarNomeProduto(p.tipo, p.id || this.gerarId()),
                    processos: this.processarArvore(p.arvore)
                }));
                // Verifica se todos estão completos mas não mostra o modal ainda
                this.allCompleted = this.produtos.every(p =>
                    p.processos.every(et => et.progresso >= 100)
                );
            }
        },

        atualizarContador(produtos) {
            if (produtos && produtos.length > 0) {
                const ids = produtos
                    .map(p => p.id ? parseInt(p.id.replace('P', '')) : 0)
                    .filter(id => !isNaN(id));
                this.productCounter = ids.length > 0 ? Math.max(...ids) + 1 : 1;
            }
        },

        gerarId() {
            return 'P' + this.productCounter.toString().padStart(3, '0');
        },

        async adicionar() {
            const productId = this.gerarId();
            try {
                const response = await fetch('/api/comando', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        acao: 'adicionar',
                        tipo: this.novoTipo,
                        id: productId
                    })
                });

                const result = await response.json();
                if (response.ok) {
                    this.productCounter++;
                    this.toast(`✅ ${this.formatarNomeProduto(this.novoTipo, productId)} adicionado`);
                    await this.fetchEstado();
                } else {
                    this.toast(`⚠️ ${result.status || 'Erro ao adicionar produto'}`);
                }
            } catch (error) {
                console.error('Erro:', error);
                this.toast('⚠️ Erro na comunicação com o servidor');
            }
        },

        async executarRodada() {
            try {
                const response = await fetch('/api/comando', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ acao: 'rodada' })
                });

                if (response.ok) {
                    this.toast('⚙️ Rodada executada');
                    await this.fetchEstado();

                    // Mostra parabéns apenas se já estava tudo completo ANTES desta rodada
                    if (this.allCompleted) {
                        this.calculateProductionCount();
                        this.showCompletionModal = true;
                    }
                } else {
                    const result = await response.json();
                    this.toast(`⚠️ ${result.status || 'Erro ao executar rodada'}`);
                }
            } catch (error) {
                console.error('Erro:', error);
                this.toast('⚠️ Erro na comunicação com o servidor');
            }
        },

        async desenfileirarProduto() {
            try {
                const response = await fetch('/api/comando', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ acao: 'desenfileirar' })
                });

                if (response.ok) {
                    await this.fetchEstado();

                    // Verifica se a fila ficou vazia após desenfileirar
                    if (this.produtos.length === 0) {
                        this.resetarEstado();
                    }
                } else {
                    const result = await response.json();
                    this.toast(`⚠️ ${result.status || 'Erro ao desenfileirar produto'}`);
                }
            } catch (error) {
                console.error('Erro:', error);
                this.toast('⚠️ Erro na comunicação com o servidor');
            }
        },
        // Novo método para resetar o estado quando a fila fica vazia
        resetarEstado() {
            this.showCompletionModal = false;
            this.allCompleted = false;
            this.toast('✅ Todos os produtos foram processados!');
        },
        async gerarRelatorio() {
            try {
                const response = await fetch('/api/comando', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ acao: 'relatorio' })
                });

                if (response.ok) {
                    this.toast('📄 Relatório gerado com sucesso!');
                } else {
                    const result = await response.json();
                    this.toast(`⚠️ ${result.status || 'Erro ao gerar relatório'}`);
                }
            } catch (error) {
                console.error('Erro:', error);
                this.toast('⚠️ Erro na comunicação com o servidor');
            }
        },

        async reiniciar() {
            try {
                const response = await fetch('/api/comando', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ acao: 'reiniciar' })
                });

                if (response.ok) {
                    this.produtos = [];
                    this.productCounter = 1;
                    this.showCompletionModal = false;
                    this.allCompleted = false;
                    this.toast('🔄 Simulação reiniciada');
                } else {
                    const result = await response.json();
                    this.toast(`⚠️ ${result.status || 'Erro ao reiniciar'}`);
                }
            } catch (error) {
                console.error('Erro:', error);
                this.toast('⚠️ Erro na comunicação com o servidor');
            }
        },

        processarArvore(node, nivel = 0) {
            const processos = [];

            const processarNo = (n, nivelAtual) => {
                const progresso = n.progresso || 0;
                let status = 'espera';

                // Modificado para priorizar "concluido" quando progresso = 100%
                if (progresso >= 100 || n.status === 'concluido') {
                    status = 'concluido';
                } else if (progresso >= 50) {
                    status = 'executando-avancado';
                } else if (progresso > 0) {
                    status = 'executando-inicial';
                }

                processos.push({
                    nome: n.nome,
                    progresso: progresso,
                    status: status,
                    nivel: nivelAtual
                });

                if (n.filhos && n.filhos.length) {
                    n.filhos.forEach(filho => processarNo(filho, nivelAtual + 1));
                }
            };

            processarNo(node, nivel);
            return processos;
        },
        formatarNomeProduto(tipo, id) {
            return `${tipo.charAt(0).toUpperCase() + tipo.slice(1)} ${id}`;
        },

        calculateProductionCount() {
            this.productionCount = this.produtos.reduce((acc, produto) => {
                acc[produto.tipo] = (acc[produto.tipo] || 0) + 1;
                return acc;
            }, {});
        },

        startProgressAnimation() {
            const animate = () => {
                if (!this.paused) {
                    this.produtos.forEach(produto => {
                        let processoEmExecucaoEncontrado = false;

                        produto.processos.forEach(etapa => {
                            // Apenas anima o processo em execução
                            if (etapa.status === 'executando' && !processoEmExecucaoEncontrado) {
                                etapa.progresso = Math.min(100, etapa.progresso + 0.5);
                                processoEmExecucaoEncontrado = true;

                                if (etapa.progresso >= 100) {
                                    etapa.status = 'concluido';
                                }
                            }
                        });
                    });
                }
                this.animationFrame = requestAnimationFrame(animate);
            };
            this.animationFrame = requestAnimationFrame(animate);
        },

        toast(msg) {
            this.toasts.push(msg);
            setTimeout(() => this.toasts.shift(), 3500);
        },

        pause() {
            this.paused = true;
            this.toast('⏸ Simulação pausada');
        },

        resume() {
            this.paused = false;
            this.toast('▶ Simulação retomada');
            this.startProgressAnimation();
        }
    }
});