#ifndef CARDS_H
#define CARDS_H

#include <allegro5/allegro.h>

typedef struct {
    ALLEGRO_BITMAP *imagem;
    int aberto;
} CardInicial;

int card_carregar(
    CardInicial *card,
    const char *caminho
);

void card_desenhar(
    const CardInicial *card,
    int largura_janela,
    int altura_janela
);

int card_processar_clique(
    CardInicial *card,
    int mouse_x,
    int mouse_y,
    int largura_janela,
    int altura_janela
);

void card_destruir(CardInicial *card);

#endif