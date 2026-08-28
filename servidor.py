"""
servidor.py - Servidor Flask/SocketIO para o Simulador Industrial V2.3.2 07/2025
"""

import os
import json
import subprocess
import threading
import time
from flask import Flask, render_template, jsonify, request
from flask_socketio import SocketIO, emit
import warnings

# ==============================================
# CONFIGURAÇÕES INICIAIS
# ==============================================

# Configuração do Flask
app = Flask(__name__, 
           static_folder="static", 
           template_folder="templates")

# Suprime warnings específicos
warnings.filterwarnings("ignore", message="The async_mode keyword argument is deprecated")
warnings.filterwarnings("ignore", category=DeprecationWarning)

# Configuração do SocketIO
socketio = SocketIO(app,
                   cors_allowed_origins="*",  # Em produção, restrinja às origens necessárias
                   async_mode='threading',
                   logger=True,               # Logs básicos
                   engineio_logger=False)     # Reduz logs detalhados

# ==============================================
# CONSTANTES E CONFIGURAÇÕES
# ==============================================

EXE_PATH = "./simulador.exe"  # Caminho para o executável do simulador
ESTADO_JSON = "estado.json"   # Arquivo de estado em JSON
FILA_BIN = "fila.bin"         # Arquivo binário de persistência

# ==============================================
# FUNÇÕES AUXILIARES
# ==============================================

def monitorar_estado():
    """
    Monitora continuamente o arquivo estado.json e envia atualizações via WebSocket.
    Executada em uma thread separada.
    """
    ultima_modificacao = 0
    
    while True:
        try:
            # Verifica se o arquivo foi modificado
            mod_time = os.path.getmtime(ESTADO_JSON)
            if mod_time != ultima_modificacao:
                with open(ESTADO_JSON, 'r', encoding='utf-8') as f:
                    dados = json.load(f)
                
                # Envia atualização para todos os clientes conectados
                socketio.emit('atualizacao', dados)
                ultima_modificacao = mod_time
                
        except FileNotFoundError:
            print("[MONITOR] Arquivo estado.json não encontrado. Aguardando criação...")
        except json.JSONDecodeError as e:
            print(f"[MONITOR] Erro ao decodificar JSON: {str(e)}")
        except PermissionError as e:
            print(f"[MONITOR] Erro de permissão no arquivo: {str(e)}")
        except OSError as e:
            print(f"[MONITOR] Erro de sistema ao acessar arquivo: {str(e)}")
        
        # Intervalo de verificação
        time.sleep(0.5)

# ==============================================
# ROTAS PRINCIPAIS
# ==============================================

@app.route('/')
def index():
    """Rota principal que serve a interface do simulador"""
    return render_template('index.html')

@app.route('/api/estado')
def obter_estado():
    """
    Endpoint para obter o estado atual da simulação.
    Retorna:
        - JSON com estado atual se bem-sucedido
        - JSON vazio se o arquivo não existir ou for inválido
    """
    try:
        with open(ESTADO_JSON, 'r', encoding='utf-8') as f:
            return jsonify(json.load(f))
    except FileNotFoundError:
        print("[API] Arquivo estado.json não encontrado")
        return jsonify({"rodada": 0, "produtos": []})
    except json.JSONDecodeError as e:
        print(f"[API] Erro ao decodificar JSON: {str(e)}")
        return jsonify({"rodada": 0, "produtos": []})
    except PermissionError as e:
        print(f"[API] Erro de permissão: {str(e)}")
        return jsonify({"status": "erro", "mensagem": "Erro de permissão"}), 500

@app.route('/api/comando', methods=['POST'])
def executar_comando():
    """
    Processa comandos do frontend e executa no simulador.
    Comandos suportados:
        - adicionar: Adiciona novo produto
        - rodada: Executa uma rodada de processamento
        - desenfileirar: Remove produto concluído
        - relatorio: Gera relatório textual
        - reiniciar: Reinicia a simulação
    """
    dados = request.get_json()
    comando = dados.get('acao')
    
    try:
        if comando == 'adicionar':
            # Adiciona novo produto à simulação
            subprocess.run([
                EXE_PATH, 
                "adicionar", 
                dados.get('tipo', ''), 
                dados.get('id', 'P000')
            ], check=True)
            
        elif comando == 'rodada':
            # Executa uma rodada de processamento
            subprocess.run([EXE_PATH, "rodada"], check=True)
            
        elif comando == 'desenfileirar':
            # Remove produto concluído e captura saída
            resultado = subprocess.run(
                [EXE_PATH, "desenfileirar"],
                capture_output=True,
                text=True,
                check=True
            )
            mensagem = resultado.stdout.strip()
            time.sleep(0.3)  # Garante tempo para atualização dos arquivos
            
            # Extrai informações do produto removido
            partes = mensagem.split()
            produto_removido = {
                'tipo': partes[1],
                'id': partes[2]
            } if len(partes) >= 3 else None
            
            # Envia atualização imediata
            with open(ESTADO_JSON, 'r', encoding='utf-8') as f:
                estado = json.load(f)
            socketio.emit('atualizacao', estado)
            
            return jsonify({
                "status": "sucesso",
                "mensagem": mensagem,
                "produto_removido": produto_removido
            })
            
        elif comando == 'relatorio':
            # Gera relatório textual
            subprocess.run([EXE_PATH, "relatorio"], check=True)
            
        elif comando == 'reiniciar':
            # Reinicia completamente a simulação
            subprocess.run([EXE_PATH, "reiniciar"], check=True)
            
        # Atualização geral após comando
        if os.path.exists(ESTADO_JSON):
            with open(ESTADO_JSON, 'r', encoding='utf-8') as f:
                socketio.emit('atualizacao', json.load(f))
                
        return jsonify({"status": "sucesso"})
        
    except subprocess.CalledProcessError as e:
        print(f"[COMANDO] Erro ao executar comando: {str(e)}")
        return jsonify({"status": "erro", "mensagem": str(e)}), 500
    except FileNotFoundError as e:
        print(f"[COMANDO] Arquivo não encontrado: {str(e)}")
        return jsonify({"status": "erro", "mensagem": "Arquivo não encontrado"}), 500
    except json.JSONDecodeError as e:
        print(f"[COMANDO] Erro no JSON: {str(e)}")
        return jsonify({"status": "erro", "mensagem": "Erro no formato do arquivo"}), 500
    except PermissionError as e:
        print(f"[COMANDO] Erro de permissão: {str(e)}")
        return jsonify({"status": "erro", "mensagem": "Erro de permissão"}), 500
    except OSError as e:
        print(f"[COMANDO] Erro de sistema: {str(e)}")
        return jsonify({"status": "erro", "mensagem": "Erro de sistema"}), 500

# ==============================================
# WEBSOCKETS
# ==============================================

@socketio.on('connect')
def handle_connect():
    """Handler para novas conexões WebSocket"""
    try:
        with open(ESTADO_JSON, 'r', encoding='utf-8') as f:
            emit('atualizacao', json.load(f))
    except FileNotFoundError:
        emit('atualizacao', {"rodada": 0, "produtos": []})
    except json.JSONDecodeError as e:
        print(f"[WS] Erro no JSON ao conectar: {str(e)}")
        emit('atualizacao', {"rodada": 0, "produtos": []})

# ==============================================
# INICIALIZAÇÃO
# ==============================================

if __name__ == '__main__':
    # Inicia thread de monitoramento em segundo plano
    threading.Thread(target=monitorar_estado, daemon=True).start()
    
    # Inicia o servidor web
    socketio.run(app, 
                host='0.0.0.0', 
                port=5000, 
                debug=True,
                allow_unsafe_werkzeug=True)  # Para desenvolvimento apenas