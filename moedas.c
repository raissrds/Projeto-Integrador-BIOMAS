#include "moedas.h"

int coletar_moedas(
    Personagem personagem,
    Moeda moedas[],
    int quantidade
) {
    int novas = 0;

    // Verifico somente as moedas que ainda estão disponíveis.
    for (int i = 0; i < quantidade; i++) {
        if (moedas[i].coletada) {
            continue;
        }

        int encostando =
            personagem.x < moedas[i].x + moedas[i].tamanho &&
            personagem.x + personagem.largura > moedas[i].x &&
            personagem.y < moedas[i].y + moedas[i].tamanho &&
            personagem.y + personagem.altura > moedas[i].y;

        if (encostando) {
            // Marco a coleta para não contar a mesma moeda novamente.
            moedas[i].coletada = 1;
            novas++;
        }
    }

    return novas;
}