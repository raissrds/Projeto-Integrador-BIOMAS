
#include <stdio.h>

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "cards.h"

// Uso o mesmo cálculo para desenhar o card e interpretar os cliques.
static int calcular_area(
    const CardInicial *card,
    int largura_janela,
    int altura_janela,
    float *x,
    float *y,
    float *largura,
    float *altura
) {
    if (card->imagem == NULL ||
        largura_janela <= 0 ||
        altura_janela <= 0) {
        return 0;
    }

    int largura_imagem = al_get_bitmap_width(card->imagem);
    int altura_imagem = al_get_bitmap_height(card->imagem);

    float escala_x = (float)largura_janela / largura_imagem;
    float escala_y = (float)altura_janela / altura_imagem;

    float escala =
        escala_x < escala_y ? escala_x : escala_y;

    // Deixo uma pequena margem e preservo a imagem inteira.
    escala *= 0.94f;

    *largura = largura_imagem * escala;
    *altura = altura_imagem * escala;

    *x = (largura_janela - *largura) / 2.0f;
    *y = (altura_janela - *altura) / 2.0f;

    return 1;
}

int card_carregar(CardInicial *card, const char *caminho) {
    card->imagem = al_load_bitmap(caminho);
    card->aberto = 1;

    if (card->imagem == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
        return 0;
    }

    return 1;
}

void card_desenhar(
    const CardInicial *card,
    int largura_janela,
    int altura_janela
) {
    if (!card->aberto) {
        return;
    }

    float x, y, largura, altura;

    if (!calcular_area(
            card,
            largura_janela,
            altura_janela,
            &x, &y, &largura, &altura
        )) {
        return;
    }

    // Escureço o cenário antes de desenhar o card.
    al_draw_filled_rectangle(
        0, 0,
        (float)largura_janela,
        (float)altura_janela,
        al_map_rgba(0, 0, 0, 160)
    );

    al_draw_scaled_bitmap(
        card->imagem,
        0, 0,
        al_get_bitmap_width(card->imagem),
        al_get_bitmap_height(card->imagem),
        x, y,
        largura, altura,
        0
    );
}

int card_processar_clique(
    CardInicial *card,
    int mouse_x,
    int mouse_y,
    int largura_janela,
    int altura_janela
) {
    if (!card->aberto) {
        return 0;
    }

    float x, y, largura, altura;

    if (!calcular_area(
            card,
            largura_janela,
            altura_janela,
            &x, &y, &largura, &altura
        )) {
        return 0;
    }

    // Converto o clique para uma posição proporcional dentro da imagem.
    float proporcao_x = (mouse_x - x) / largura;
    float proporcao_y = (mouse_y - y) / altura;

    // Uso a região interna do botão desenhado no card enviado.
    if (proporcao_x >= 0.27f &&
        proporcao_x <= 0.74f &&
        proporcao_y >= 0.80f &&
        proporcao_y <= 0.915f) {

        card->aberto = 0;
        return 1;
    }

    return 0;
}

void card_destruir(CardInicial *card) {
    if (card->imagem != NULL) {
        al_destroy_bitmap(card->imagem);
        card->imagem = NULL;
    }

    card->aberto = 0;
}