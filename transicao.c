#include <stdio.h>

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "transicao.h"

static float limitar(float valor) {
    if (valor < 0) return 0;
    if (valor > 1) return 1;

    return valor;
}

// Suavizo o começo e o final dos fades.
static float suavizar(float valor) {
    valor = limitar(valor);

    return valor * valor * (3 - 2 * valor);
}

int transicao_carregar(Transicao *t) {
    *t = (Transicao){0};
    t->etapa = TRANSICAO_FINALIZADA;

    const char *caminhos[2] = {
        "imagens/transicao/transicao_inicio.png",
        "imagens/transicao/transicao_proxima.png"
    };

    // Carrego as duas artes uma única vez.
    for (int i = 0; i < 2; i++) {
        t->imagens[i] = al_load_bitmap(caminhos[i]);

        if (t->imagens[i] == NULL) {
            fprintf(
                stderr,
                "Erro ao carregar %s\n",
                caminhos[i]
            );

            transicao_destruir(t);
            return 0;
        }
    }

    return 1;
}

void transicao_iniciar(
    Transicao *t,
    int primeira,
    double agora
) {
    // Escolho a mensagem conforme a passagem do jogo.
    t->tipo = primeira ? 0 : 1;

    t->etapa = TRANSICAO_ESCURECENDO;
    t->inicio_etapa = agora;
    t->quadro_exibido = 0;
}

void transicao_atualizar(
    Transicao *t,
    double agora
) {
    double decorrido = agora - t->inicio_etapa;

    if (t->etapa == TRANSICAO_ESCURECENDO &&
        decorrido >= TRANSICAO_FADE) {
        t->etapa = TRANSICAO_APRESENTANDO;
        t->inicio_etapa = agora;
    } else if (
        t->etapa == TRANSICAO_ANIMANDO &&
        decorrido >= TRANSICAO_TEMPO_BARRA
    ) {
        t->etapa = TRANSICAO_REVELANDO;
        t->inicio_etapa = agora;
    } else if (
        t->etapa == TRANSICAO_REVELANDO &&
        decorrido >= TRANSICAO_FADE
    ) {
        t->etapa = TRANSICAO_FINALIZADA;
    }
}

// Confirmo que a arte apareceu antes de carregar a fase.
void transicao_confirmar_quadro(
    Transicao *t,
    double agora
) {
    if (t->etapa == TRANSICAO_APRESENTANDO &&
        agora - t->inicio_etapa >= TRANSICAO_FADE) {
        t->quadro_exibido = 1;
    }
}

int transicao_precisa_carregar(const Transicao *t) {
    return
        t->etapa == TRANSICAO_APRESENTANDO &&
        t->quadro_exibido;
}

void transicao_fase_carregada(
    Transicao *t,
    double agora
) {
    // Inicio os dois segundos da barra depois de carregar a fase.
    t->etapa = TRANSICAO_ANIMANDO;
    t->inicio_etapa = agora;
    t->quadro_exibido = 0;
}

void transicao_desenhar(
    const Transicao *t,
    int largura,
    int altura,
    double agora
) {
    if (largura <= 0 ||
        altura <= 0 ||
        t->etapa == TRANSICAO_FINALIZADA) {
        return;
    }

    ALLEGRO_TRANSFORM anterior;
    ALLEGRO_TRANSFORM identidade;

    al_copy_transform(
        &anterior,
        al_get_current_transform()
    );

    al_identity_transform(&identidade);
    al_use_transform(&identidade);

    float tempo = (float)(agora - t->inicio_etapa);

    if (t->etapa == TRANSICAO_ESCURECENDO) {
        float a = suavizar(
            tempo / (float)TRANSICAO_FADE
        );

        al_draw_filled_rectangle(
            0, 0, largura, altura,
            al_map_rgba_f(0, 0, 0, a)
        );

        al_use_transform(&anterior);
        return;
    }

    ALLEGRO_BITMAP *imagem = t->imagens[t->tipo];

    if (imagem == NULL) {
        al_use_transform(&anterior);
        return;
    }

    float a = 1;
    float progresso = 0;

    if (t->etapa == TRANSICAO_APRESENTANDO) {
        a = suavizar(
            tempo / (float)TRANSICAO_FADE
        );
    } else if (t->etapa == TRANSICAO_ANIMANDO) {
        progresso = limitar(
            tempo / (float)TRANSICAO_TEMPO_BARRA
        );
    } else if (t->etapa == TRANSICAO_REVELANDO) {
        a = 1 - suavizar(
            tempo / (float)TRANSICAO_FADE
        );

        progresso = 1;
    }

    float wi = al_get_bitmap_width(imagem);
    float hi = al_get_bitmap_height(imagem);

    float sx = largura / wi;
    float sy = altura / hi;
    float escala = sx < sy ? sx : sy;

    float w = wi * escala;
    float h = hi * escala;

    float x = (largura - w) / 2;
    float y = (altura - h) / 2;

    // Preservo a proporção da arte e escureço as margens.
    al_draw_filled_rectangle(
        0, 0, largura, altura,
        al_map_rgba_f(0, 0, 0, a)
    );

    al_draw_tinted_scaled_bitmap(
        imagem,
        al_map_rgba_f(a, a, a, a),
        0, 0, wi, hi,
        x, y, w, h,
        0
    );

    // Preencho somente o interior da moldura da barra.
    int blocos = (int)(progresso * 40);

    float inicio = x + w * 0.303f;
    float fim = x + w * 0.696f;

    float topo = y + h * 0.665f;
    float base = y + h * 0.706f;

    for (int i = 0; i < blocos; i++) {
        float x1 =
            inicio + (fim - inicio) * i / 40;

        float x2 =
            inicio + (fim - inicio) * (i + 1) / 40;

        ALLEGRO_COLOR verde =
            i % 4 < 2
                ? al_map_rgba_f(
                    0.48f * a,
                    0.74f * a,
                    0,
                    a
                )
                : al_map_rgba_f(
                    0.64f * a,
                    0.86f * a,
                    0.03f * a,
                    a
                );

        al_draw_filled_rectangle(
            (int)(x1 + 0.5f),
            (int)(topo + 0.5f),
            (int)(x2 + 0.5f),
            (int)(base + 0.5f),
            verde
        );

        // Acrescento uma faixa clara para dar volume à barra.
        al_draw_filled_rectangle(
            (int)(x1 + 0.5f),
            (int)(topo + 0.5f),
            (int)(x2 + 0.5f),
            (int)(topo + (base - topo) * 0.18f + 0.5f),
            al_map_rgba_f(
                0.80f * a,
                0.96f * a,
                0.18f * a,
                a
            )
        );
    }

    al_use_transform(&anterior);
}

void transicao_destruir(Transicao *t) {
    // Libero as artes antes de encerrar a janela.
    for (int i = 0; i < 2; i++) {
        if (t->imagens[i] != NULL) {
            al_destroy_bitmap(t->imagens[i]);
        }
    }

    *t = (Transicao){0};
    t->etapa = TRANSICAO_FINALIZADA;
}