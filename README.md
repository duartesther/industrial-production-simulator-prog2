# Industrial Production Simulator

A simulation engine for an industrial production line, built in **C**, with a **Python (Flask + Socket.IO)** web dashboard for real-time visualization. Developed as a Data Structures course project at UFSC (in a pair).

## What it does

- Simulates a production line for multiple products (e.g. fan, exhaust fan, vacuum cleaner), each with its own sequence of manufacturing subprocesses.
- Uses a **FIFO queue** to schedule products for production in order.
- Uses a **tree structure** to represent the multi-stage subprocesses each product goes through.
- Persists simulation state to disk in **binary format**, and exchanges data with the web dashboard via **JSON** (using a custom cJSON-based serializer).
- The Python web server reads the simulation state and pushes **real-time updates** to the browser via WebSocket (Socket.IO), so production status can be watched live.

## Tech stack

- **C** — core simulation engine (queue, tree, persistence, serialization)
- **Python (Flask, Flask-SocketIO)** — web server and real-time communication
- **HTML/CSS/JS** — dashboard front-end

## Project structure

```
src/            C source code (queue, tree/process logic, persistence, JSON serialization)
static/         CSS and JS for the dashboard
templates/       HTML template for the dashboard
servidor.py     Python/Flask server that runs the dashboard
Makefile        Build script for the C simulation engine
requirements.txt Python dependencies
```

## How to run

```bash
# Build the C simulation engine
make

# Install Python dependencies
pip install -r requirements.txt

# Run the web dashboard (calls the compiled simulator behind the scenes)
python servidor.py
```

Then open `http://localhost:5000` in your browser.

## Author

Esther Duarte dos Reis — Mechatronics Engineering student, UFSC
