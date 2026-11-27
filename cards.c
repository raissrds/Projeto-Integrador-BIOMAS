#include <stdio.h>

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "cards.h"

typedef struct {
    float x;
    float y;
    float largura;
    float altura;
} LayoutCard;

// Calculo a posição e o tamanho do card sem deformar a imagem.
static int calcular_layout(
    const CardInicial *card,
    int largura_janela,
    int altura_janela,
    LayoutCard *layout
) {
    if (card->imagem == NULL ||
        largura_janela <= 0 ||
        altura_janela <= 0) {
        return 0;
    }

    float largura_imagem =
        (float)al_get_bitmap_width(card->imagem);

    float altura_imagem =
        (float)al_get_bitmap_height(card->imagem);

    float escala_x =
        largura_janela * 0.94f / largura_imagem;

    float escala_y =
        altura_janela * 0.94f / altura_imagem;

    float escala =
        escala_x < escala_y ? escala_x : escala_y;

    layout->largura = largura_imagem * escala;
    layout->altura = altura_imagem * escala;

    layout->x =
        (largura_janela - layout->largura) / 2;

    layout->y =
        (altura_janela - layout->altura) / 2;

    return 1;
}

int card_carregar(
    CardInicial *card,
    const char *caminho
) {
    card->imagem = al_load_bitmap(caminho);
    card->aberto = 0;

    if (card->imagem == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
        return 0;
    }

    // Deixo o card aberto quando inicio a fase.
    card->aberto = 1;

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

    LayoutCard layout;

    if (!calcular_layout(
            card,
            largura_janela,
            altura_janela,
            &layout
        )) {
        return;
    }

    ALLEGRO_TRANSFORM anterior;
    ALLEGRO_TRANSFORM identidade;

    al_copy_transform(
        &anterior,
        al_get_current_transform()
    );

    // Desenho o card separado do deslocamento da câmera.
    al_identity_transform(&identidade);
    al_use_transform(&identidade);

    // Escureço o cenário atrás do card.
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
        layout.x,
        layout.y,
        layout.largura,
        layout.altura,
        0
    );

    al_use_transform(&anterior);
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

    LayoutCard layout;

    if (!calcular_layout(
            card,
            largura_janela,
            altura_janela,
            &layout
        )) {
        return 0;
    }

    // Converto o clique para uma posição proporcional na imagem.
    float u = (mouse_x - layout.x) / layout.largura;
    float v = (mouse_y - layout.y) / layout.altura;

    // Verifico o botão Iniciar expedição da imagem da Caatinga.
    if (u >= 0.225f && u <= 0.775f &&
        v >= 0.765f && v <= 0.900f) {
        card->aberto = 0;
        return 1;
    }

    return 0;
}

void card_destruir(CardInicial *card) {
    // Libero a imagem quando encerro a fase.
    if (card->imagem != NULL) {
        al_destroy_bitmap(card->imagem);
        card->imagem = NULL;
    }

    card->aberto = 0;
}