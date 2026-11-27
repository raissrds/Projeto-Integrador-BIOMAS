#include "moedas.h"

int coletar_moedas(
    Personagem personagem,
    Moeda moedas[],
    int quantidade
) {
    int coletadas = 0;

    for (int i = 0; i < quantidade; i++) {
        // Ignoro as moedas que já coletei.
        if (moedas[i].coletada) {
            continue;
        }

        // Verifico se a personagem encostou na moeda.
        int contato =
            personagem.x < moedas[i].x + moedas[i].tamanho &&
            personagem.x + personagem.largura > moedas[i].x &&
            personagem.y < moedas[i].y + moedas[i].tamanho &&
            personagem.y + personagem.altura > moedas[i].y;

        if (contato) {
            moedas[i].coletada = 1;
            coletadas++;
        }
    }

    return coletadas;
}