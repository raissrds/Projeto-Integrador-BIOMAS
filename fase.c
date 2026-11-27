#include <stdio.h>

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "fase.h"

// Arredondo as posições para reduzir as tremidas.
static int pixel(float valor) {
    return (int)(valor + 0.5f);
}

// Localizo a parte visível de cada textura.
static int carregar_textura(Textura *t, const char *caminho) {
    t->imagem = al_load_bitmap(caminho);

    if (t->imagem == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
        return 0;
    }

    int largura = al_get_bitmap_width(t->imagem);
    int altura = al_get_bitmap_height(t->imagem);

    int esquerda = largura;
    int direita = -1;
    int cima = altura;
    int baixo = -1;

    if (al_lock_bitmap(
            t->imagem,
            ALLEGRO_PIXEL_FORMAT_ANY,
            ALLEGRO_LOCK_READONLY
        ) == NULL) {
        fprintf(stderr, "Erro ao acessar %s\n", caminho);
        return 0;
    }

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            unsigned char r, g, b, a;

            al_unmap_rgba(
                al_get_pixel(t->imagem, x, y),
                &r, &g, &b, &a
            );

            if (a >= 128) {
                if (x < esquerda) esquerda = x;
                if (x > direita) direita = x;
                if (y < cima) cima = y;
                if (y > baixo) baixo = y;
            }
        }
    }

    al_unlock_bitmap(t->imagem);

    if (direita < esquerda || baixo < cima) {
        fprintf(stderr, "Imagem sem conteudo: %s\n", caminho);
        return 0;
    }

    t->x = esquerda;
    t->y = cima;
    t->largura = direita - esquerda + 1;
    t->altura = baixo - cima + 1;

    return 1;
}

// Desenho a textura dentro das dimensões do objeto.
static void desenhar_textura(
    const Textura *t,
    float x,
    float y,
    float largura,
    float altura,
    ALLEGRO_COLOR cor
) {
    al_draw_tinted_scaled_bitmap(
        t->imagem, cor,
        t->x, t->y,
        t->largura, t->altura,
        x, y, largura, altura,
        0
    );
}

// Mantenho a câmera dentro dos limites do cenário.
static float limitar_camera(
    float camera,
    float largura_fase,
    float largura_visivel
) {
    float limite = largura_fase - largura_visivel;

    if (limite < 0) limite = 0;
    if (camera < 0) camera = 0;
    if (camera > limite) camera = limite;

    return camera;
}

// Adiciono uma moeda sem ultrapassar o tamanho do vetor.
static void adicionar_moeda(
    Fase *f,
    float centro_x,
    float y
) {
    if (f->quantidade_moedas >= MAX_MOEDAS) return;

    Moeda *moeda = &f->moedas[f->quantidade_moedas];

    moeda->x = centro_x - TAMANHO_MOEDA / 2;
    moeda->y = y;
    moeda->tamanho = TAMANHO_MOEDA;
    moeda->coletada = 0;

    f->quantidade_moedas++;
}

// Desenho o contador fixo na tela.
static void desenhar_contador(const Fase *f, float escala) {
    ALLEGRO_TRANSFORM anterior, t;

    al_copy_transform(&anterior, al_get_current_transform());
    al_identity_transform(&t);
    al_scale_transform(&t, 2 * escala, 2 * escala);
    al_use_transform(&t);

    al_draw_filled_rectangle(
        6, 6, 164, 34,
        al_map_rgba(20, 35, 20, 230)
    );

    desenhar_textura(
        &f->textura_moeda,
        10, 11, 18, 18,
        al_map_rgb(255, 255, 255)
    );

    al_draw_textf(
        f->fonte,
        al_map_rgb(255, 230, 130),
        35, 16, 0,
        "Moedas: %d/%d",
        f->moedas_coletadas,
        f->quantidade_moedas
    );

    al_use_transform(&anterior);
}

// Mostro a conclusão do último bioma disponível nesta integração.
static void desenhar_conclusao(
    const Fase *f,
    int largura,
    int altura,
    float escala
) {
    al_draw_filled_rectangle(
        0, 0, largura, altura,
        al_map_rgba(0, 0, 0, 180)
    );

    ALLEGRO_TRANSFORM anterior, t;
    al_copy_transform(&anterior, al_get_current_transform());

    float fator = 2 * escala;
    float cx = largura / fator / 2;
    float cy = altura / fator / 2;

    al_identity_transform(&t);
    al_scale_transform(&t, fator, fator);
    al_use_transform(&t);

    al_draw_filled_rectangle(
        cx - 210, cy - 65,
        cx + 210, cy + 65,
        al_map_rgb(245, 219, 160)
    );

    ALLEGRO_COLOR texto = al_map_rgb(65, 35, 15);

    al_draw_text(
        f->fonte, texto,
        cx, cy - 45,
        ALLEGRO_ALIGN_CENTRE,
        f->bioma == BIOMA_AMAZONIA
            ? "AMAZONIA CONCLUIDA!"
            : "CAATINGA CONCLUIDA!"
    );

    al_draw_text(
        f->fonte, texto,
        cx, cy - 20,
        ALLEGRO_ALIGN_CENTRE,
        "Todas as moedas foram coletadas."
    );

    al_draw_text(
        f->fonte, texto,
        cx, cy - 5,
        ALLEGRO_ALIGN_CENTRE,
        "Desafio respondido corretamente."
    );

    al_draw_textf(
        f->fonte, texto,
        cx, cy + 15,
        ALLEGRO_ALIGN_CENTRE,
        "Moedas: %d/%d",
        f->moedas_coletadas,
        f->quantidade_moedas
    );

    al_use_transform(&anterior);
}

int fase_carregar(
    Fase *f,
    ALLEGRO_FONT *fonte,
    Bioma bioma
) {
    if (bioma != BIOMA_AMAZONIA &&
        bioma != BIOMA_CAATINGA) {
        return 0;
    }

    *f = (Fase){0};

    f->fonte = fonte;
    f->bioma = bioma;
    f->nome = bioma == BIOMA_AMAZONIA
        ? "Amazonia"
        : "Caatinga";

    f->estado = APRESENTACAO;

    const char *pasta = bioma == BIOMA_AMAZONIA
        ? "imagens/amazonia"
        : "imagens/caatinga";

    const char *nomes[2][3] = {
        {"selva.png", "floresta.png", "rio.png"},
        {"caatinga01.png", "caatinga02.png", "caatinga03.png"}
    };

    // Preservo a distribuição própria dos obstáculos de cada bioma.
    const Objeto iniciais[2][12] = {
        {
            {380, CHAO_Y - 60, 70, 60},
            {620, CHAO_Y - 110, 130, 24},
            {840, CHAO_Y - 160, 140, 24},
            {1080, CHAO_Y - 210, 150, 24},
            {1310, CHAO_Y - 90, 80, 90},
            {1510, CHAO_Y - 110, 140, 24},
            {1740, CHAO_Y - 160, 150, 24},
            {1990, CHAO_Y - 210, 160, 24},
            {2230, CHAO_Y - 110, 85, 110},
            {2420, CHAO_Y - 120, 140, 24},
            {2670, CHAO_Y - 180, 150, 24},
            {2920, CHAO_Y - 70, 90, 70}
        },
        {
            {380, CHAO_Y - 55, 85, 55},
            {600, CHAO_Y - 100, 145, 24},
            {850, CHAO_Y - 140, 160, 24},
            {1120, CHAO_Y - 75, 100, 75},
            {1340, CHAO_Y - 115, 155, 24},
            {1590, CHAO_Y - 140, 150, 24},
            {1840, CHAO_Y - 60, 90, 60},
            {2050, CHAO_Y - 100, 160, 24},
            {2310, CHAO_Y - 135, 155, 24},
            {2550, CHAO_Y - 85, 100, 85},
            {2760, CHAO_Y - 120, 150, 24},
            {3000, CHAO_Y - 55, 90, 55}
        }
    };

    f->quantidade_objetos = 12;

    for (int i = 0; i < f->quantidade_objetos; i++) {
        f->objetos[i] = iniciais[bioma][i];
    }

    char caminho[256];

    // Carrego os fundos da pasta do bioma atual.
    for (int i = 0; i < QUANTIDADE_FUNDOS; i++) {
        snprintf(
            caminho, sizeof(caminho),
            "%s/%s", pasta, nomes[bioma][i]
        );

        f->fundos[i] = al_load_bitmap(caminho);

        if (f->fundos[i] == NULL) {
            fprintf(stderr, "Erro ao carregar %s\n", caminho);
            goto erro;
        }

        f->larguras[i] =
            ALTURA_MUNDO *
            (float)al_get_bitmap_width(f->fundos[i]) /
            al_get_bitmap_height(f->fundos[i]);

        f->posicoes[i] = f->largura_fase;
        f->largura_fase += f->larguras[i];
    }

    snprintf(
        caminho, sizeof(caminho),
        "%s/personagem.png", pasta
    );

    f->sprite = al_load_bitmap(caminho);

    if (f->sprite == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
        goto erro;
    }

    snprintf(caminho, sizeof(caminho), "%s/chao.png", pasta);
    f->chao = al_load_bitmap(caminho);

    if (f->chao == NULL) {
        fprintf(stderr, "Erro ao carregar %s\n", caminho);
        goto erro;
    }

    snprintf(caminho, sizeof(caminho), "%s/pedra.png", pasta);
    if (!carregar_textura(&f->pedra, caminho)) goto erro;

    snprintf(
        caminho, sizeof(caminho),
        "%s/plataforma.png", pasta
    );
    if (!carregar_textura(&f->plataforma, caminho)) goto erro;

    snprintf(caminho, sizeof(caminho), "%s/moeda.png", pasta);
    if (!carregar_textura(&f->textura_moeda, caminho)) goto erro;

    snprintf(caminho, sizeof(caminho), "%s/card.png", pasta);
    if (!card_carregar(&f->card, caminho)) goto erro;

    snprintf(caminho, sizeof(caminho), "%s/pergunta.png", pasta);

    // Uso B na Amazônia e D na Caatinga.
    if (!pergunta_carregar(
            &f->pergunta,
            caminho,
            bioma == BIOMA_AMAZONIA ? 1 : 3,
            bioma == BIOMA_AMAZONIA
                ? "O clima equatorial e quente e umido,"
                : "Clima semiarido e vegetacao adaptada a seca.",
            bioma == BIOMA_AMAZONIA
                ? "com chuvas abundantes."
                : "A fauna inclui o tatu e o veado-catingueiro.",
            bioma == BIOMA_CAATINGA
        )) {
        goto erro;
    }

    // Confiro se os obstáculos estão dentro do percurso.
    for (int i = 0; i < f->quantidade_objetos; i++) {
        if (f->objetos[i].x + f->objetos[i].largura >
            f->largura_fase) {
            fprintf(
                stderr,
                "O percurso de %s e curto para os obstaculos.\n",
                f->nome
            );
            goto erro;
        }
    }

    f->largura_sprite = al_get_bitmap_width(f->sprite);
    f->altura_sprite = al_get_bitmap_height(f->sprite);
    f->largura_chao = al_get_bitmap_width(f->chao);
    f->altura_chao = al_get_bitmap_height(f->chao);

    f->largura_peca_chao =
        (ALTURA_MUNDO - CHAO_Y) *
        (float)f->largura_chao / f->altura_chao;

    f->personagem.altura = 80;
    f->personagem.largura =
        f->personagem.altura *
        (float)f->largura_sprite / f->altura_sprite;

    f->personagem.x = 150;
    f->personagem.y = CHAO_Y - f->personagem.altura;
    f->personagem.apoiado = 1;

    adicionar_moeda(
        f, 260,
        CHAO_Y - TAMANHO_MOEDA - 8
    );

    adicionar_moeda(
        f, 320,
        CHAO_Y - TAMANHO_MOEDA - 8
    );

    // Coloco duas moedas sobre cada obstáculo.
    for (int i = 0; i < f->quantidade_objetos; i++) {
        float y = f->objetos[i].y - TAMANHO_MOEDA - 8;

        adicionar_moeda(
            f,
            f->objetos[i].x + f->objetos[i].largura * 0.25f,
            y
        );

        adicionar_moeda(
            f,
            f->objetos[i].x + f->objetos[i].largura * 0.75f,
            y
        );
    }

    return 1;

erro:
    fase_destruir(f);
    return 0;
}

void fase_desenhar(
    Fase *f,
    int largura_janela,
    int altura_janela
) {
    if (largura_janela <= 0 || altura_janela <= 0) return;

    float escala = altura_janela / ALTURA_MUNDO;
    float largura_visivel = largura_janela / escala;

    f->camera_x = limitar_camera(
        f->personagem.x + f->personagem.largura / 2
            - largura_visivel / 2,
        f->largura_fase,
        largura_visivel
    );

    al_clear_to_color(al_map_rgb(0, 0, 0));

    int deslocamento = pixel(f->camera_x * escala);

    // Desenho os fundos sem posições fracionadas.
    for (int i = 0; i < QUANTIDADE_FUNDOS; i++) {
        int x1 =
            pixel(f->posicoes[i] * escala) - deslocamento;

        int x2 =
            pixel((f->posicoes[i] + f->larguras[i]) * escala)
            - deslocamento;

        if (x1 < largura_janela && x2 > 0) {
            al_draw_scaled_bitmap(
                f->fundos[i],
                0, 0,
                al_get_bitmap_width(f->fundos[i]),
                al_get_bitmap_height(f->fundos[i]),
                x1, 0,
                x2 - x1, altura_janela,
                0
            );
        }
    }

    int topo_chao = pixel(CHAO_Y * escala);
    int primeira = (int)(f->camera_x / f->largura_peca_chao);

    int ultima = (int)(
        (f->camera_x + largura_visivel) / f->largura_peca_chao
    ) + 1;

    // Repito o chão ao longo do percurso.
    for (int i = primeira; i < ultima; i++) {
        float inicio = i * f->largura_peca_chao;
        float fim = (i + 1) * f->largura_peca_chao;

        if (inicio >= f->largura_fase) break;
        if (fim > f->largura_fase) fim = f->largura_fase;

        int x1 = pixel(inicio * escala) - deslocamento;
        int x2 = pixel(fim * escala) - deslocamento;

        float recorte =
            f->largura_chao * (fim - inicio) /
            f->largura_peca_chao;

        if (x2 > x1) {
            al_draw_scaled_bitmap(
                f->chao,
                0, 0,
                recorte, f->altura_chao,
                x1, topo_chao,
                x2 - x1, altura_janela - topo_chao,
                0
            );
        }
    }

    for (int i = 0; i < f->quantidade_objetos; i++) {
        Objeto objeto = f->objetos[i];

        const Textura *textura =
            objeto.y + objeto.altura >= CHAO_Y - 0.1f
                ? &f->pedra
                : &f->plataforma;

        int x1 = pixel(objeto.x * escala) - deslocamento;
        int x2 =
            pixel((objeto.x + objeto.largura) * escala)
            - deslocamento;

        int y1 = pixel(objeto.y * escala);
        int y2 = pixel((objeto.y + objeto.altura) * escala);

        ALLEGRO_COLOR cor =
            verificar_contato(f->personagem, objeto)
                ? al_map_rgb(255, 180, 180)
                : al_map_rgb(255, 255, 255);

        if (x1 < largura_janela && x2 > 0) {
            desenhar_textura(
                textura,
                x1, y1, x2 - x1, y2 - y1,
                cor
            );
        }
    }

    // Desenho apenas as moedas ainda disponíveis.
    for (int i = 0; i < f->quantidade_moedas; i++) {
        if (f->moedas[i].coletada) continue;

        int x = pixel(f->moedas[i].x * escala) - deslocamento;
        int y = pixel(f->moedas[i].y * escala);
        int tamanho = pixel(f->moedas[i].tamanho * escala);

        if (x < largura_janela && x + tamanho > 0) {
            desenhar_textura(
                &f->textura_moeda,
                x, y, tamanho, tamanho,
                al_map_rgb(255, 255, 255)
            );
        }
    }

    // Inverto a personagem quando caminho para a esquerda.
    al_draw_scaled_bitmap(
        f->sprite,
        0, 0,
        f->largura_sprite, f->altura_sprite,
        pixel(f->personagem.x * escala) - deslocamento,
        pixel(f->personagem.y * escala),
        pixel(f->personagem.largura * escala),
        pixel(f->personagem.altura * escala),
        f->olhando_esquerda ? ALLEGRO_FLIP_HORIZONTAL : 0
    );

    desenhar_contador(f, escala);

    if (f->estado == APRESENTACAO) {
        card_desenhar(
            &f->card,
            largura_janela,
            altura_janela
        );
    } else if (f->estado == DESAFIO) {
        pergunta_desenhar(
            &f->pergunta,
            f->card.imagem,
            f->fonte,
            largura_janela,
            altura_janela,
            al_get_time()
        );
    } else if (f->estado == FASE_CONCLUIDA) {
        desenhar_conclusao(
            f, largura_janela, altura_janela, escala
        );
    }
}

void fase_atualizar(Fase *f, double agora) {
    if (f->estado == EXPLORANDO) {
        atualizar_movimento(
            &f->personagem,
            f->objetos,
            f->quantidade_objetos,
            f->esquerda,
            f->direita,
            f->largura_fase
        );

        f->moedas_coletadas += coletar_moedas(
            f->personagem,
            f->moedas,
            f->quantidade_moedas
        );

        if (f->quantidade_moedas > 0 &&
            f->moedas_coletadas == f->quantidade_moedas) {
            // Abro a pergunta depois de coletar todas as moedas.
            f->esquerda = 0;
            f->direita = 0;
            f->estado = DESAFIO;

            pergunta_iniciar(&f->pergunta, agora);
        }
    } else if (f->estado == DESAFIO) {
        pergunta_atualizar(&f->pergunta, agora);

        if (pergunta_deve_reiniciar(&f->pergunta, agora)) {
            // Reinicio somente o bioma atual após o segundo erro.
            f->personagem.x = 150;
            f->personagem.y = CHAO_Y - f->personagem.altura;
            f->personagem.velocidade_y = 0;
            f->personagem.apoiado = 1;

            for (int i = 0; i < f->quantidade_moedas; i++) {
                f->moedas[i].coletada = 0;
            }

            f->moedas_coletadas = 0;
            f->camera_x = 0;
            f->esquerda = 0;
            f->direita = 0;
            f->olhando_esquerda = 0;

            f->card.aberto = 1;
            pergunta_iniciar(&f->pergunta, agora);

            f->estado = APRESENTACAO;
        }
    }
}

// Reconheço o botão inicial conforme a imagem de cada bioma.
static int iniciar_expedicao(
    Fase *f,
    int mouse_x,
    int mouse_y,
    int largura,
    int altura
) {
    if (f->bioma == BIOMA_AMAZONIA) {
        return card_processar_clique(
            &f->card,
            mouse_x, mouse_y,
            largura, altura
        );
    }

    if (!f->card.aberto || largura <= 0 || altura <= 0) {
        return 0;
    }

    float w = al_get_bitmap_width(f->card.imagem);
    float h = al_get_bitmap_height(f->card.imagem);

    float sx = largura / w;
    float sy = altura / h;
    float escala = (sx < sy ? sx : sy) * 0.94f;

    float x = (largura - w * escala) / 2;
    float y = (altura - h * escala) / 2;

    float u = (mouse_x - x) / (w * escala);
    float v = (mouse_y - y) / (h * escala);

    if (u >= 0.225f && u <= 0.775f &&
        v >= 0.765f && v <= 0.900f) {
        f->card.aberto = 0;
        return 1;
    }

    return 0;
}

void fase_processar_evento(
    Fase *f,
    const ALLEGRO_EVENT *e,
    int largura,
    int altura
) {
    if (e->type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
        f->esquerda = 0;
        f->direita = 0;
    } else if (
        e->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        e->mouse.button == 1
    ) {
        if (f->estado == APRESENTACAO) {
            if (iniciar_expedicao(
                    f,
                    e->mouse.x, e->mouse.y,
                    largura, altura
                )) {
                f->esquerda = 0;
                f->direita = 0;
                f->estado = EXPLORANDO;
            }
        } else if (f->estado == DESAFIO) {
            AcaoPergunta acao = pergunta_processar_clique(
                &f->pergunta,
                f->card.imagem,
                e->mouse.x, e->mouse.y,
                largura, altura,
                al_get_time()
            );

            if (acao == PERGUNTA_CONTINUAR) {
                f->estado = FASE_CONCLUIDA;
            }
        }
    } else if (
        e->type == ALLEGRO_EVENT_KEY_DOWN &&
        f->estado == EXPLORANDO
    ) {
        switch (e->keyboard.keycode) {
            case ALLEGRO_KEY_LEFT:
                f->esquerda = 1;
                f->olhando_esquerda = 1;
                break;

            case ALLEGRO_KEY_RIGHT:
                f->direita = 1;
                f->olhando_esquerda = 0;
                break;

            case ALLEGRO_KEY_UP:
                if (f->personagem.apoiado) {
                    f->personagem.velocidade_y = -15;
                    f->personagem.apoiado = 0;
                }
                break;

            default:
                break;
        }
    } else if (e->type == ALLEGRO_EVENT_KEY_UP) {
        if (e->keyboard.keycode == ALLEGRO_KEY_LEFT)
            f->esquerda = 0;

        if (e->keyboard.keycode == ALLEGRO_KEY_RIGHT)
            f->direita = 0;
    }
}

void fase_destruir(Fase *f) {
    // Libero os recursos do bioma, preservando a janela e a fonte.
    pergunta_destruir(&f->pergunta);
    card_destruir(&f->card);

    if (f->sprite != NULL) al_destroy_bitmap(f->sprite);
    if (f->chao != NULL) al_destroy_bitmap(f->chao);

    if (f->pedra.imagem != NULL)
        al_destroy_bitmap(f->pedra.imagem);

    if (f->plataforma.imagem != NULL)
        al_destroy_bitmap(f->plataforma.imagem);

    if (f->textura_moeda.imagem != NULL)
        al_destroy_bitmap(f->textura_moeda.imagem);

    for (int i = 0; i < QUANTIDADE_FUNDOS; i++) {
        if (f->fundos[i] != NULL)
            al_destroy_bitmap(f->fundos[i]);
    }

    *f = (Fase){0};
}