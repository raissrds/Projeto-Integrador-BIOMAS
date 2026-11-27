#ifndef BIOMAS_FASE_H
#define BIOMAS_FASE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "movimento.h"
#include "moedas.h"
#include "cards.h"
#include "perguntas.h"

#define QUANTIDADE_FUNDOS 3
#define MAX_MOEDAS 64

typedef enum {
    BIOMA_AMAZONIA,
    BIOMA_CAATINGA
} Bioma;

typedef enum {
    APRESENTACAO,
    EXPLORANDO,
    DESAFIO,
    FASE_CONCLUIDA
} EstadoFase;

typedef struct {
    ALLEGRO_BITMAP *imagem;
    int x;
    int y;
    int largura;
    int altura;
} Textura;

// Guardo os recursos e o progresso do bioma atual.
typedef struct {
    Bioma bioma;
    const char *nome;

    ALLEGRO_FONT *fonte;
    ALLEGRO_BITMAP *sprite;
    ALLEGRO_BITMAP *chao;
    ALLEGRO_BITMAP *fundos[QUANTIDADE_FUNDOS];

    Textura pedra;
    Textura plataforma;
    Textura textura_moeda;

    CardInicial card;
    PerguntaFinal pergunta;
    EstadoFase estado;

    float larguras[QUANTIDADE_FUNDOS];
    float posicoes[QUANTIDADE_FUNDOS];

    Objeto objetos[12];
    int quantidade_objetos;

    Personagem personagem;
    Moeda moedas[MAX_MOEDAS];

    int quantidade_moedas;
    int moedas_coletadas;

    float largura_fase;
    float camera_x;
    float largura_peca_chao;

    int largura_sprite;
    int altura_sprite;
    int largura_chao;
    int altura_chao;

    int esquerda;
    int direita;
    int olhando_esquerda;
} Fase;

int fase_carregar(
    Fase *fase,
    ALLEGRO_FONT *fonte,
    Bioma bioma
);

void fase_desenhar(
    Fase *fase,
    int largura,
    int altura
);

void fase_processar_evento(
    Fase *fase,
    const ALLEGRO_EVENT *evento,
    int largura,
    int altura
);

void fase_atualizar(Fase *fase, double agora);
void fase_destruir(Fase *fase);

#endif