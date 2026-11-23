#ifndef MOEDAS_H
#define MOEDAS_H

#include "movimento.h"

#define TAMANHO_MOEDA 24.0f

typedef struct {
    float x;
    float y;
    float tamanho;
    int coletada;
} Moeda;

// Retorno quantas moedas foram coletadas nesta atualização.
int coletar_moedas(
    Personagem personagem,
    Moeda moedas[],
    int quantidade
);

#endif