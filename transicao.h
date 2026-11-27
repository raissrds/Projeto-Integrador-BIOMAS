#ifndef BIOMAS_TRANSICAO_H
#define BIOMAS_TRANSICAO_H

#include <allegro5/allegro.h>

#define TRANSICAO_FADE 0.25
#define TRANSICAO_TEMPO_BARRA 2.0

typedef enum {
    TRANSICAO_ESCURECENDO,
    TRANSICAO_APRESENTANDO,
    TRANSICAO_ANIMANDO,
    TRANSICAO_REVELANDO,
    TRANSICAO_FINALIZADA
} EtapaTransicao;

typedef struct {
    ALLEGRO_BITMAP *imagens[2];

    EtapaTransicao etapa;

    int tipo;
    int quadro_exibido;

    double inicio_etapa;
} Transicao;

int transicao_carregar(Transicao *t);

void transicao_iniciar(
    Transicao *t,
    int primeira,
    double agora
);

void transicao_atualizar(
    Transicao *t,
    double agora
);

void transicao_desenhar(
    const Transicao *t,
    int largura,
    int altura,
    double agora
);

void transicao_confirmar_quadro(
    Transicao *t,
    double agora
);

int transicao_precisa_carregar(const Transicao *t);

void transicao_fase_carregada(
    Transicao *t,
    double agora
);

void transicao_destruir(Transicao *t);

#endif