CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -g
SRCD    := src

SRC     := $(SRCD)/processo.c  \
           $(SRCD)/produto.c   \
           $(SRCD)/fila.c      \
           $(SRCD)/persistencia.c \
           $(SRCD)/serializer.c   \
           $(SRCD)/json_comms.c   \
           $(SRCD)/main.c       \
           $(SRCD)/cJSON.c
TARGET  := simulador.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) relatorio.txt estado.txt estado.json fila.bin comando.json

.PHONY: all clean
