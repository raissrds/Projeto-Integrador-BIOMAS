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

// Uso as posições proporcionais das quatro alternativas.
static const float areas[4][4] = {
    {0.160f, 0.468f, 0.843f, 0.556f},
    {0.160f, 0.570f, 0.843f, 0.660f},
    {0.160f, 0.672f, 0.843f, 0.759f},
    {0.160f, 0.771f, 0.843f, 0.860f}
};

// Faço o painel aparecer gradualmente.
static float opacidade(const PerguntaFinal *p, double agora) {
    float t = (float)(
        (agora - p->inicio_abertura)
        / PERGUNTA_TEMPO_ABERTURA
    );

    if (t < 0) t = 0;
    if (t > 1) t = 1;

    return t * t * (3 - 2 * t);
}

// Aplico a transparência também aos textos, bordas e botões.
static ALLEGRO_COLOR cor(int r, int g, int b, float a) {
    return al_map_rgba_f(
        r / 255.0f * a,
        g / 255.0f * a,
        b / 255.0f * a,
        a
    );
}

static int calcular_layout(
    const PerguntaFinal *p,
    ALLEGRO_BITMAP *conteudo,
    int largura,
    int altura,
    LayoutPergunta *l
) {
    if (largura <= 0 || altura <= 0) {
        return 0;
    }

    l->imagem =
        p->etapa == LENDO_REVISAO ? conteudo : p->imagem;

    if (l->imagem == NULL) {
        return 0;
    }

    float sx = largura / 500.0f;
    float sy = altura / 350.0f;

    l->escala = sx < sy ? sx : sy;
    l->margem_x = (largura - 500 * l->escala) / 2;
    l->margem_y = (altura - 350 * l->escala) / 2;

    l->largura_origem =
        (float)al_get_bitmap_width(l->imagem);

    l->altura_origem =
        (float)al_get_bitmap_height(l->imagem);

    // Na revisão, retiro a parte que contém Iniciar expedição.
    if (p->etapa == LENDO_REVISAO) {
        l->altura_origem *= 0.78f;
    }

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
    // Limpo as tentativas ao começar um novo desafio.
    for (int i = 0; i < 4; i++) {
        p->errou[i] = 0;
    }

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

    if (correta < 0 || correta >= 4) {
        return 0;
    }

    p->alternativa_correta = correta;
    p->explicacao1 = explicacao1;
    p->explicacao2 = explicacao2;
    p->imagem = al_load_bitmap(caminho);

    if (p->imagem == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
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
    // Exijo a revisão antes de permitir a segunda tentativa.
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
    if (p->etapa == AGUARDANDO_REVISAO) {
        p->etapa = LENDO_REVISAO;
    }
}

void pergunta_voltar(PerguntaFinal *p) {
    if (p->etapa == LENDO_REVISAO) {
        p->etapa = ESCOLHENDO_RESPOSTA;
    }
}

int pergunta_atualizar(PerguntaFinal *p, double agora) {
    int estava_animando = p->animando;

    p->animando =
        agora - p->inicio_abertura < PERGUNTA_TEMPO_ABERTURA;

    // Desenho também o último quadro da abertura.
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

// Uso a mesma posição para os dois tipos de botão.
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
        cor(110, 65, 20, a),
        1
    );

    al_draw_text(
        fonte,
        cor(255, 239, 190, a),
        250, 331,
        ALLEGRO_ALIGN_CENTRE,
        texto
    );
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

    if (!calcular_layout(
            p, conteudo, largura, altura, &l
        )) {
        return;
    }

    float a = opacidade(p, agora);

    // Escureço o cenário aos poucos.
    al_draw_filled_rectangle(
        0, 0,
        (float)largura, (float)altura,
        cor(0, 0, 0, 0.63f * a)
    );

    ALLEGRO_TRANSFORM anterior;
    ALLEGRO_TRANSFORM transformacao;

    al_copy_transform(
        &anterior,
        al_get_current_transform()
    );

    al_identity_transform(&transformacao);
    al_scale_transform(
        &transformacao,
        l.escala, l.escala
    );

    al_translate_transform(
        &transformacao,
        l.margem_x, l.margem_y
    );

    al_use_transform(&transformacao);

    al_draw_tinted_scaled_bitmap(
        l.imagem,
        al_map_rgba_f(a, a, a, a),
        0, 0,
        l.largura_origem,
        l.altura_origem,
        l.x, l.y,
        l.largura, l.altura,
        0
    );

    // Mostro os destaques somente na pergunta.
    if (p->etapa != LENDO_REVISAO) {
        for (int i = 0; i < 4; i++) {
            int correta =
                p->etapa == RESPOSTA_CORRETA &&
                i == p->alternativa_correta;

            if (!correta && !p->errou[i]) {
                continue;
            }

            float x1 = l.x + areas[i][0] * l.largura;
            float y1 = l.y + areas[i][1] * l.altura;
            float x2 = l.x + areas[i][2] * l.largura;
            float y2 = l.y + areas[i][3] * l.altura;

            al_draw_filled_rectangle(
                x1, y1, x2, y2,
                correta
                    ? cor(0, 200, 0, 0.35f * a)
                    : cor(200, 0, 0, 0.35f * a)
            );

            al_draw_rectangle(
                x1, y1, x2, y2,
                correta
                    ? cor(100, 255, 70, a)
                    : cor(255, 65, 65, a),
                2
            );
        }
    }

    al_draw_filled_rectangle(
        35, 280, 465, 349,
        cor(245, 219, 160, a)
    );

    ALLEGRO_COLOR texto = cor(65, 35, 15, a);

    switch (p->etapa) {
        case ESCOLHENDO_RESPOSTA:
            al_draw_text(
                fonte, texto,
                250, 289,
                ALLEGRO_ALIGN_CENTRE,
                p->erros == 0
                    ? "Clique em uma alternativa."
                    : "Segunda e ultima tentativa."
            );
            break;

        case AGUARDANDO_REVISAO:
            al_draw_text(
                fonte,
                cor(150, 25, 20, a),
                250, 285,
                ALLEGRO_ALIGN_CENTRE,
                "Primeira tentativa incorreta."
            );

            al_draw_text(
                fonte, texto,
                250, 302,
                ALLEGRO_ALIGN_CENTRE,
                "Reveja o conteudo para tentar novamente."
            );

            desenhar_botao(fonte, "Rever conteudo", a);
            break;

        case LENDO_REVISAO:
            al_draw_text(
                fonte, texto,
                250, 285,
                ALLEGRO_ALIGN_CENTRE,
                "Releia o conteudo antes de responder."
            );

            al_draw_text(
                fonte, texto,
                250, 302,
                ALLEGRO_ALIGN_CENTRE,
                "Depois, clique em Avancar."
            );

            desenhar_botao(fonte, "Avancar", a);
            break;

        case RESPOSTA_CORRETA:
            al_draw_text(
                fonte,
                cor(25, 100, 25, a),
                250, 284,
                ALLEGRO_ALIGN_CENTRE,
                "Resposta correta!"
            );

            al_draw_text(
                fonte, texto,
                250, 297,
                ALLEGRO_ALIGN_CENTRE,
                p->explicacao1
            );

            al_draw_text(
                fonte, texto,
                250, 309,
                ALLEGRO_ALIGN_CENTRE,
                p->explicacao2
            );

            desenhar_botao(fonte, "Avancar", a);
            break;

        case LIMITE_DE_ERROS:
            al_draw_text(
                fonte,
                cor(150, 25, 20, a),
                250, 286,
                ALLEGRO_ALIGN_CENTRE,
                "Segunda tentativa incorreta."
            );

            al_draw_text(
                fonte, texto,
                250, 303,
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
    // Aguardo a abertura terminar para evitar cliques acidentais.
    if (opacidade(p, agora) < 1) {
        return PERGUNTA_NENHUMA;
    }

    LayoutPergunta l;

    if (!calcular_layout(
            p, conteudo, largura, altura, &l
        )) {
        return PERGUNTA_NENHUMA;
    }

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

    if (p->etapa != ESCOLHENDO_RESPOSTA) {
        return PERGUNTA_NENHUMA;
    }

    float u = (x - l.x) / l.largura;
    float v = (y - l.y) / l.altura;

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
    if (p->imagem != NULL) {
        al_destroy_bitmap(p->imagem);
        p->imagem = NULL;
    }
}