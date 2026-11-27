#include <stdio.h>

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "perguntas.h"

typedef struct {
    float escala;
    float margem_x;
    float margem_y;

    float x;
    float y;
    float largura;
    float altura;

    float largura_origem;
    float altura_origem;

    ALLEGRO_BITMAP *imagem;
} LayoutPergunta;

// Uso as posições das alternativas da imagem da Caatinga.
static const float areas[4][4] = {
    {0.139f, 0.468f, 0.862f, 0.539f},
    {0.139f, 0.545f, 0.862f, 0.645f},
    {0.139f, 0.648f, 0.862f, 0.748f},
    {0.139f, 0.750f, 0.862f, 0.850f}
};

// Guardo as versões vermelha e verde de cada alternativa.
static ALLEGRO_BITMAP *botoes_coloridos[4][2] = {{NULL}};
static int recortes[4][4] = {{0}};

// Limito os componentes das cores ao intervalo permitido.
static unsigned char limitar_cor(float valor) {
    if (valor < 0) return 0;
    if (valor > 255) return 255;
    return (unsigned char)(valor + 0.5f);
}

// Libero os recortes que preparei.
static void liberar_botoes(void) {
    for (int i = 0; i < 4; i++) {
        for (int tipo = 0; tipo < 2; tipo++) {
            if (botoes_coloridos[i][tipo] != NULL) {
                al_destroy_bitmap(botoes_coloridos[i][tipo]);
                botoes_coloridos[i][tipo] = NULL;
            }
        }
    }
}

// Recoloro apenas os pixels verdes, preservando as letras.
static int preparar_botoes(ALLEGRO_BITMAP *imagem) {
    int largura = al_get_bitmap_width(imagem);
    int altura = al_get_bitmap_height(imagem);

    ALLEGRO_BITMAP *alvo_anterior = al_get_target_bitmap();
    int flags_anteriores = al_get_new_bitmap_flags();

    liberar_botoes();

    if (al_lock_bitmap(
            imagem,
            ALLEGRO_PIXEL_FORMAT_ANY,
            ALLEGRO_LOCK_READONLY
        ) == NULL) {
        return 0;
    }

    al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

    for (int i = 0; i < 4; i++) {
        int x1 = (int)(areas[i][0] * largura);
        int y1 = (int)(areas[i][1] * altura);
        int x2 = (int)(areas[i][2] * largura);
        int y2 = (int)(areas[i][3] * altura);

        if (x1 < 0) x1 = 0;
        if (y1 < 0) y1 = 0;
        if (x2 > largura) x2 = largura;
        if (y2 > altura) y2 = altura;

        int w = x2 - x1;
        int h = y2 - y1;

        recortes[i][0] = x1;
        recortes[i][1] = y1;
        recortes[i][2] = w;
        recortes[i][3] = h;

        for (int tipo = 0; tipo < 2; tipo++) {
            ALLEGRO_BITMAP *botao = al_create_bitmap(w, h);

            if (botao == NULL) goto erro;

            botoes_coloridos[i][tipo] = botao;

            if (al_lock_bitmap(
                    botao,
                    ALLEGRO_PIXEL_FORMAT_ANY,
                    ALLEGRO_LOCK_WRITEONLY
                ) == NULL) {
                goto erro;
            }

            al_set_target_bitmap(botao);

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    ALLEGRO_COLOR original =
                        al_get_pixel(imagem, x1 + x, y1 + y);

                    unsigned char r, g, b, a;
                    al_unmap_rgba(original, &r, &g, &b, &a);

                    int pixel_verde =
                        a > 0 &&
                        g > 20 &&
                        g > r * 1.15f &&
                        g > b * 1.15f;

                    if (pixel_verde) {
                        unsigned char novo_r, novo_g, novo_b;

                        if (tipo == 0) {
                            // Transformo o fundo verde em vermelho.
                            novo_r = limitar_cor(g * 1.65f + 30);
                            novo_g = limitar_cor(r * 0.35f);
                            novo_b = limitar_cor(b * 0.45f);
                        } else {
                            // Intensifico o verde para sinalizar o acerto.
                            novo_r = limitar_cor(r * 0.65f);
                            novo_g = limitar_cor(g * 1.45f + 30);
                            novo_b = limitar_cor(b * 0.65f);
                        }

                        al_put_pixel(
                            x, y,
                            al_map_rgba(novo_r, novo_g, novo_b, a)
                        );
                    } else {
                        al_put_pixel(x, y, original);
                    }
                }
            }

            al_unlock_bitmap(botao);
        }
    }

    al_unlock_bitmap(imagem);
    al_set_target_bitmap(alvo_anterior);
    al_set_new_bitmap_flags(flags_anteriores);
    return 1;

erro:
    al_unlock_bitmap(imagem);
    al_set_target_bitmap(alvo_anterior);
    al_set_new_bitmap_flags(flags_anteriores);
    liberar_botoes();
    return 0;
}

// Faço o painel aparecer gradualmente.
static float opacidade(const PerguntaFinal *p, double agora) {
    float t = (float)(
        (agora - p->inicio_abertura) / PERGUNTA_TEMPO_ABERTURA
    );

    if (t < 0) t = 0;
    if (t > 1) t = 1;

    return t * t * (3 - 2 * t);
}

// Aplico a transparência durante a abertura.
static ALLEGRO_COLOR cor(int r, int g, int b, float a) {
    return al_map_rgba_f(
        r / 255.0f * a,
        g / 255.0f * a,
        b / 255.0f * a,
        a
    );
}

// Ajusto o painel ao tamanho da janela.
static int calcular_layout(
    const PerguntaFinal *p,
    ALLEGRO_BITMAP *conteudo,
    int largura,
    int altura,
    LayoutPergunta *l
) {
    if (largura <= 0 || altura <= 0) return 0;

    l->imagem =
        p->etapa == LENDO_REVISAO ? conteudo : p->imagem;

    if (l->imagem == NULL) return 0;

    float sx = largura / 500.0f;
    float sy = altura / 350.0f;

    l->escala = sx < sy ? sx : sy;
    l->margem_x = (largura - 500 * l->escala) / 2;
    l->margem_y = (altura - 350 * l->escala) / 2;

    l->largura_origem = al_get_bitmap_width(l->imagem);
    l->altura_origem = al_get_bitmap_height(l->imagem);

    // Retiro o botão inicial durante a revisão.
    if (p->etapa == LENDO_REVISAO)
        l->altura_origem *= 0.78f;

    float ex = 480 / l->largura_origem;
    float ey = 270 / l->altura_origem;
    float escala_imagem = ex < ey ? ex : ey;

    l->largura = l->largura_origem * escala_imagem;
    l->altura = l->altura_origem * escala_imagem;
    l->x = (500 - l->largura) / 2;
    l->y = 4;

    return 1;
}

void pergunta_iniciar(PerguntaFinal *p, double agora) {
    // Limpo as tentativas ao iniciar um novo desafio.
    for (int i = 0; i < 4; i++) p->errou[i] = 0;

    p->erros = 0;
    p->etapa = ESCOLHENDO_RESPOSTA;
    p->inicio_abertura = agora;
    p->instante_erro = 0;
    p->animando = 1;
}

int pergunta_carregar(
    PerguntaFinal *p,
    const char *caminho,
    int correta,
    const char *explicacao1,
    const char *explicacao2
) {
    *p = (PerguntaFinal){0};

    if (correta < 0 || correta >= 4) return 0;

    p->alternativa_correta = correta;
    p->explicacao1 = explicacao1;
    p->explicacao2 = explicacao2;
    p->imagem = al_load_bitmap(caminho);

    if (p->imagem == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
        return 0;
    }

    if (!preparar_botoes(p->imagem)) {
        fprintf(stderr, "Erro ao preparar as alternativas.\n");
        return 0;
    }

    pergunta_iniciar(p, 0);
    return 1;
}

void pergunta_responder(
    PerguntaFinal *p,
    int alternativa,
    double agora
) {
    // Exijo a revisão antes da segunda tentativa.
    if (p->etapa != ESCOLHENDO_RESPOSTA ||
        alternativa < 0 || alternativa >= 4) {
        return;
    }

    if (alternativa == p->alternativa_correta) {
        p->etapa = RESPOSTA_CORRETA;
    } else {
        p->errou[alternativa] = 1;
        p->erros++;

        if (p->erros == 1) {
            p->etapa = AGUARDANDO_REVISAO;
        } else {
            p->etapa = LIMITE_DE_ERROS;
            p->instante_erro = agora;
        }
    }
}

void pergunta_revisar(PerguntaFinal *p) {
    if (p->etapa == AGUARDANDO_REVISAO)
        p->etapa = LENDO_REVISAO;
}

void pergunta_voltar(PerguntaFinal *p) {
    if (p->etapa == LENDO_REVISAO)
        p->etapa = ESCOLHENDO_RESPOSTA;
}

int pergunta_atualizar(PerguntaFinal *p, double agora) {
    int estava_animando = p->animando;

    p->animando =
        agora - p->inicio_abertura < PERGUNTA_TEMPO_ABERTURA;

    return estava_animando || p->animando;
}

int pergunta_deve_reiniciar(
    const PerguntaFinal *p,
    double agora
) {
    return
        p->etapa == LIMITE_DE_ERROS &&
        agora - p->instante_erro >= PERGUNTA_TEMPO_REINICIO;
}

// Mantenho os botões de revisão e avanço na mesma posição.
static void desenhar_botao(
    ALLEGRO_FONT *fonte,
    const char *texto,
    float a
) {
    al_draw_filled_rectangle(
        170, 325, 330, 345,
        cor(65, 85, 25, a)
    );

    al_draw_rectangle(
        170, 325, 330, 345,
        cor(110, 65, 20, a), 1
    );

    al_draw_text(
        fonte, cor(255, 239, 190, a),
        250, 331, ALLEGRO_ALIGN_CENTRE, texto
    );
}

// Desenho o recorte colorido sobre a alternativa escolhida.
static void desenhar_respostas(
    const PerguntaFinal *p,
    const LayoutPergunta *l
) {
    if (p->etapa == LENDO_REVISAO) return;

    float escala_x = l->largura / l->largura_origem;
    float escala_y = l->altura / l->altura_origem;

    for (int i = 0; i < 4; i++) {
        int correta =
            p->etapa == RESPOSTA_CORRETA &&
            i == p->alternativa_correta;

        if (!correta && !p->errou[i]) continue;

        ALLEGRO_BITMAP *botao =
            botoes_coloridos[i][correta ? 1 : 0];

        if (botao == NULL) continue;

        al_draw_scaled_bitmap(
            botao, 0, 0,
            recortes[i][2], recortes[i][3],
            l->x + recortes[i][0] * escala_x,
            l->y + recortes[i][1] * escala_y,
            recortes[i][2] * escala_x,
            recortes[i][3] * escala_y,
            0
        );
    }
}

void pergunta_desenhar(
    const PerguntaFinal *p,
    ALLEGRO_BITMAP *conteudo,
    ALLEGRO_FONT *fonte,
    int largura,
    int altura,
    double agora
) {
    LayoutPergunta l;

    if (!calcular_layout(p, conteudo, largura, altura, &l))
        return;

    float a = opacidade(p, agora);

    // Escureço o cenário atrás da pergunta.
    al_draw_filled_rectangle(
        0, 0, largura, altura,
        cor(0, 0, 0, 0.63f * a)
    );

    ALLEGRO_TRANSFORM anterior, t;

    al_copy_transform(&anterior, al_get_current_transform());
    al_identity_transform(&t);
    al_scale_transform(&t, l.escala, l.escala);
    al_translate_transform(&t, l.margem_x, l.margem_y);
    al_use_transform(&t);

    al_draw_tinted_scaled_bitmap(
        l.imagem, al_map_rgba_f(a, a, a, a),
        0, 0, l.largura_origem, l.altura_origem,
        l.x, l.y, l.largura, l.altura, 0
    );

    if (a >= 1) desenhar_respostas(p, &l);

    al_draw_filled_rectangle(
        35, 280, 465, 349,
        cor(245, 219, 160, a)
    );

    ALLEGRO_COLOR texto = cor(65, 35, 15, a);

    switch (p->etapa) {
        case ESCOLHENDO_RESPOSTA:
            al_draw_text(
                fonte, texto, 250, 289,
                ALLEGRO_ALIGN_CENTRE,
                p->erros == 0
                    ? "Clique em uma alternativa."
                    : "Segunda e ultima tentativa."
            );
            break;

        case AGUARDANDO_REVISAO:
            al_draw_text(
                fonte, cor(150, 25, 20, a),
                250, 285, ALLEGRO_ALIGN_CENTRE,
                "Primeira tentativa incorreta."
            );
            al_draw_text(
                fonte, texto, 250, 302,
                ALLEGRO_ALIGN_CENTRE,
                "Reveja o conteudo para tentar novamente."
            );
            desenhar_botao(fonte, "Rever conteudo", a);
            break;

        case LENDO_REVISAO:
            al_draw_text(
                fonte, texto, 250, 285,
                ALLEGRO_ALIGN_CENTRE,
                "Releia o conteudo antes de responder."
            );
            al_draw_text(
                fonte, texto, 250, 302,
                ALLEGRO_ALIGN_CENTRE,
                "Depois, clique em Avancar."
            );
            desenhar_botao(fonte, "Avancar", a);
            break;

        case RESPOSTA_CORRETA:
            al_draw_text(
                fonte, cor(25, 100, 25, a),
                250, 284, ALLEGRO_ALIGN_CENTRE,
                "Resposta correta!"
            );
            al_draw_text(
                fonte, texto, 250, 297,
                ALLEGRO_ALIGN_CENTRE, p->explicacao1
            );
            al_draw_text(
                fonte, texto, 250, 309,
                ALLEGRO_ALIGN_CENTRE, p->explicacao2
            );
            desenhar_botao(fonte, "Avancar", a);
            break;

        case LIMITE_DE_ERROS:
            al_draw_text(
                fonte, cor(150, 25, 20, a),
                250, 286, ALLEGRO_ALIGN_CENTRE,
                "Segunda tentativa incorreta."
            );
            al_draw_text(
                fonte, texto, 250, 303,
                ALLEGRO_ALIGN_CENTRE,
                "A fase sera reiniciada."
            );
            break;
    }

    al_use_transform(&anterior);
}

AcaoPergunta pergunta_processar_clique(
    PerguntaFinal *p,
    ALLEGRO_BITMAP *conteudo,
    int mouse_x,
    int mouse_y,
    int largura,
    int altura,
    double agora
) {
    // Aguardo a abertura terminar antes de aceitar cliques.
    if (opacidade(p, agora) < 1)
        return PERGUNTA_NENHUMA;

    LayoutPergunta l;

    if (!calcular_layout(p, conteudo, largura, altura, &l))
        return PERGUNTA_NENHUMA;

    float x = (mouse_x - l.margem_x) / l.escala;
    float y = (mouse_y - l.margem_y) / l.escala;

    int clicou_botao =
        x >= 170 && x <= 330 &&
        y >= 325 && y <= 345;

    if (clicou_botao) {
        if (p->etapa == AGUARDANDO_REVISAO) {
            pergunta_revisar(p);
        } else if (p->etapa == LENDO_REVISAO) {
            pergunta_voltar(p);
        } else if (p->etapa == RESPOSTA_CORRETA) {
            return PERGUNTA_CONTINUAR;
        }

        return PERGUNTA_NENHUMA;
    }

    if (p->etapa != ESCOLHENDO_RESPOSTA)
        return PERGUNTA_NENHUMA;

    float u = (x - l.x) / l.largura;
    float v = (y - l.y) / l.altura;

    // Uso as mesmas áreas para desenhar e reconhecer os cliques.
    for (int i = 0; i < 4; i++) {
        if (u >= areas[i][0] && u <= areas[i][2] &&
            v >= areas[i][1] && v <= areas[i][3]) {
            pergunta_responder(p, i, agora);
            break;
        }
    }

    return PERGUNTA_NENHUMA;
}

void pergunta_destruir(PerguntaFinal *p) {
    // Libero os recortes e a imagem do desafio.
    liberar_botoes();

    if (p->imagem != NULL) {
        al_destroy_bitmap(p->imagem);
        p->imagem = NULL;
    }
}