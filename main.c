#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "movimento.h"
#include "moedas.h"
#include "cards.h"
#include "perguntas.h"

#define QUANTIDADE_FUNDOS 3
#define MAX_MOEDAS 64

typedef enum {
    APRESENTACAO,
    EXPLORANDO,
    DESAFIO,
    FASE_CONCLUIDA
} EstadoJogo;

typedef struct {
    ALLEGRO_BITMAP *imagem;
    int x;
    int y;
    int largura;
    int altura;
} Textura;

// Arredondo as posições para desenhar em pixels inteiros.
static int pixel(float valor) {
    return (int)(valor + 0.5f);
}

// Localizo a parte visível das texturas.
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
        fprintf(stderr, "Imagem sem conteudo visivel: %s\n", caminho);
        return 0;
    }

    t->x = esquerda;
    t->y = cima;
    t->largura = direita - esquerda + 1;
    t->altura = baixo - cima + 1;

    return 1;
}

// Desenho a parte visível da textura no tamanho do objeto.
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

// Limito a câmera para não ultrapassar as bordas da fase.
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

// Adiciono cada moeda respeitando o limite do vetor.
static void adicionar_moeda(
    Moeda moedas[],
    int *quantidade,
    float centro_x,
    float y
) {
    if (*quantidade >= MAX_MOEDAS) {
        return;
    }

    moedas[*quantidade].x = centro_x - TAMANHO_MOEDA / 2;
    moedas[*quantidade].y = y;
    moedas[*quantidade].tamanho = TAMANHO_MOEDA;
    moedas[*quantidade].coletada = 0;

    (*quantidade)++;
}

// Desenho o contador fixo, separado da câmera.
static void desenhar_contador(
    ALLEGRO_FONT *fonte,
    const Textura *moeda,
    int coletadas,
    int quantidade,
    float escala
) {
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
        moeda,
        10, 11, 18, 18,
        al_map_rgb(255, 255, 255)
    );

    al_draw_textf(
        fonte,
        al_map_rgb(255, 230, 130),
        35, 16, 0,
        "Moedas: %d/%d",
        coletadas, quantidade
    );

    al_use_transform(&anterior);
}

// Exibo a confirmação enquanto a próxima fase ainda não está integrada.
static void desenhar_conclusao(
    ALLEGRO_FONT *fonte,
    int largura,
    int altura,
    float escala,
    int coletadas,
    int quantidade
) {
    al_draw_filled_rectangle(
        0, 0,
        (float)largura, (float)altura,
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
        fonte, texto,
        cx, cy - 45,
        ALLEGRO_ALIGN_CENTRE,
        "CAATINGA CONCLUIDA!"
    );

    al_draw_text(
        fonte, texto,
        cx, cy - 20,
        ALLEGRO_ALIGN_CENTRE,
        "Todas as moedas foram coletadas."
    );

    al_draw_text(
        fonte, texto,
        cx, cy - 5,
        ALLEGRO_ALIGN_CENTRE,
        "Desafio respondido corretamente."
    );

    al_draw_textf(
        fonte, texto,
        cx, cy + 15,
        ALLEGRO_ALIGN_CENTRE,
        "Moedas: %d/%d",
        coletadas, quantidade
    );

    al_use_transform(&anterior);
}

int main(void) {
    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_FONT *fonte = NULL;

    ALLEGRO_BITMAP *sprite = NULL;
    ALLEGRO_BITMAP *chao = NULL;
    ALLEGRO_BITMAP *fundos[QUANTIDADE_FUNDOS] = {NULL};

    Textura pedra = {0};
    Textura plataforma = {0};
    Textura textura_moeda = {0};

    CardInicial card = {0};
    PerguntaFinal pergunta = {0};

    EstadoJogo estado = APRESENTACAO;

    // Defino a ordem dos três cenários da Caatinga.
    const char *caminhos[QUANTIDADE_FUNDOS] = {
        "imagem/caatinga01.png",
        "imagem/caatinga02.png",
        "imagem/caatinga03.png"
    };

    float larguras[QUANTIDADE_FUNDOS] = {0};
    float posicoes[QUANTIDADE_FUNDOS] = {0};

    // Distribuo pedras e plataformas respeitando o alcance do pulo.
    // Mantenho as plataformas até 140 unidades acima do chão.
    Objeto objetos[] = {
        {380,  CHAO_Y - 55,  85,  55},
        {600,  CHAO_Y - 100, 145, 24},
        {850,  CHAO_Y - 140, 160, 24},
        {1120, CHAO_Y - 75,  100, 75},
        {1340, CHAO_Y - 115, 155, 24},
        {1590, CHAO_Y - 140, 150, 24},
        {1840, CHAO_Y - 60,  90,  60},
        {2050, CHAO_Y - 100, 160, 24},
        {2310, CHAO_Y - 135, 155, 24},
        {2550, CHAO_Y - 85,  100, 85},
        {2760, CHAO_Y - 120, 150, 24},
        {3000, CHAO_Y - 55,  90,  55}
    };

    const int quantidade_objetos =
        (int)(sizeof(objetos) / sizeof(objetos[0]));

    Personagem personagem = {0};

    Moeda moedas[MAX_MOEDAS] = {0};
    int quantidade_moedas = 0;
    int moedas_coletadas = 0;

    float largura_fase = 0;
    float camera_x = 0;
    float largura_peca_chao = 0;

    int largura_sprite = 0;
    int altura_sprite = 0;
    int largura_chao = 0;
    int altura_chao = 0;

    int esquerda = 0;
    int direita = 0;
    int olhando_esquerda = 0;

    int rodando = 1;
    int redesenhar = 1;
    int resultado = 1;

    int imagens_iniciadas = 0;
    int teclado_iniciado = 0;
    int mouse_iniciado = 0;
    int primitivas_iniciadas = 0;
    int fontes_iniciadas = 0;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (!al_init()) {
        fprintf(stderr, "Erro ao inicializar a Allegro.\n");
        return 1;
    }

    imagens_iniciadas = al_init_image_addon();

    if (!imagens_iniciadas) {
        fprintf(stderr, "Erro ao inicializar as imagens.\n");
        goto finalizar;
    }

    teclado_iniciado = al_install_keyboard();

    if (!teclado_iniciado) {
        fprintf(stderr, "Erro ao inicializar o teclado.\n");
        goto finalizar;
    }

    mouse_iniciado = al_install_mouse();

    if (!mouse_iniciado) {
        fprintf(stderr, "Erro ao inicializar o mouse.\n");
        goto finalizar;
    }

    primitivas_iniciadas = al_init_primitives_addon();

    if (!primitivas_iniciadas) {
        fprintf(stderr, "Erro ao inicializar as primitivas.\n");
        goto finalizar;
    }

    fontes_iniciadas = al_init_font_addon();

    if (!fontes_iniciadas) {
        fprintf(stderr, "Erro ao inicializar as fontes.\n");
        goto finalizar;
    }

    // Mantenho os pixels sem filtro de suavização.
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);

    // Uso OpenGL e permito redimensionar a janela.
    al_set_new_display_flags(
        ALLEGRO_WINDOWED |
        ALLEGRO_OPENGL |
        ALLEGRO_RESIZABLE
    );

    al_set_new_display_option(
        ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST
    );

    janela = al_create_display(1000, 600);

    if (janela == NULL) {
        fprintf(stderr, "Erro ao criar a janela.\n");
        goto finalizar;
    }

    al_set_window_title(janela, "BIOMAS - Caatinga");

    fila = al_create_event_queue();
    timer = al_create_timer(1.0 / 60.0);
    fonte = al_create_builtin_font();

    if (fila == NULL || timer == NULL || fonte == NULL) {
        fprintf(stderr, "Erro ao criar fila, timer ou fonte.\n");
        goto finalizar;
    }

    // Carrego as imagens uma única vez e calculo a largura do percurso.
    for (int i = 0; i < QUANTIDADE_FUNDOS; i++) {
        fundos[i] = al_load_bitmap(caminhos[i]);

        if (fundos[i] == NULL) {
            fprintf(stderr, "Erro ao carregar %s\n", caminhos[i]);
            goto finalizar;
        }

        larguras[i] =
            ALTURA_MUNDO * (float)al_get_bitmap_width(fundos[i])
            / al_get_bitmap_height(fundos[i]);

        posicoes[i] = largura_fase;
        largura_fase += larguras[i];
    }

    // Confiro se os fundos comportam todos os obstáculos.
    for (int i = 0; i < quantidade_objetos; i++) {
        if (objetos[i].x + objetos[i].largura + 40 > largura_fase) {
            fprintf(
                stderr,
                "Os fundos sao estreitos para a distribuicao dos obstaculos.\n"
                "Largura da fase: %.0f; minimo necessario: %.0f.\n",
                largura_fase,
                objetos[i].x + objetos[i].largura + 40
            );

            goto finalizar;
        }
    }

    sprite = al_load_bitmap("imagem/personagem.png");
    chao = al_load_bitmap("imagem/chao.png");

    if (sprite == NULL || chao == NULL) {
        fprintf(
            stderr,
            "Confira imagem/personagem.png e imagem/chao.png\n"
        );

        goto finalizar;
    }

    if (!carregar_textura(&pedra, "imagem/pedra.png"))
        goto finalizar;

    if (!carregar_textura(&plataforma, "imagem/plataforma.png"))
        goto finalizar;

    if (!carregar_textura(&textura_moeda, "imagem/moeda.png"))
        goto finalizar;

    if (!card_carregar(&card, "imagem/card.png"))
        goto finalizar;

    // Defino a alternativa D como correta usando o índice 3.
    if (!pergunta_carregar(
            &pergunta,
            "imagem/pergunta.png",
            3,
            "Clima semiarido e vegetacao adaptada a seca.",
            "A fauna inclui o tatu e o veado-catingueiro."
        )) {
        goto finalizar;
    }

    largura_sprite = al_get_bitmap_width(sprite);
    altura_sprite = al_get_bitmap_height(sprite);
    largura_chao = al_get_bitmap_width(chao);
    altura_chao = al_get_bitmap_height(chao);

    largura_peca_chao =
        (ALTURA_MUNDO - CHAO_Y)
        * (float)largura_chao / altura_chao;

    // Mantenho a proporção da imagem da personagem.
    personagem.altura = 80;
    personagem.largura =
        personagem.altura * (float)largura_sprite / altura_sprite;

    personagem.x = 150;
    personagem.y = CHAO_Y - personagem.altura;
    personagem.apoiado = 1;

    // Coloco duas moedas no início para apresentar a coleta.
    adicionar_moeda(
        moedas, &quantidade_moedas,
        260, CHAO_Y - TAMANHO_MOEDA - 8
    );

    adicionar_moeda(
        moedas, &quantidade_moedas,
        320, CHAO_Y - TAMANHO_MOEDA - 8
    );

    // Distribuo mais duas moedas sobre cada obstáculo.
    for (int i = 0; i < quantidade_objetos; i++) {
        float y = objetos[i].y - TAMANHO_MOEDA - 8;

        adicionar_moeda(
            moedas, &quantidade_moedas,
            objetos[i].x + objetos[i].largura * 0.25f,
            y
        );

        adicionar_moeda(
            moedas, &quantidade_moedas,
            objetos[i].x + objetos[i].largura * 0.75f,
            y
        );
    }

    al_register_event_source(
        fila, al_get_display_event_source(janela)
    );

    al_register_event_source(
        fila, al_get_keyboard_event_source()
    );

    al_register_event_source(
        fila, al_get_mouse_event_source()
    );

    al_register_event_source(
        fila, al_get_timer_event_source(timer)
    );

    // Abro a fase em tela cheia.
    if (!al_toggle_display_flag(
            janela, ALLEGRO_FULLSCREEN_WINDOW, true
        )) {
        fprintf(stderr, "Continuando em modo janela.\n");
    }

    while (rodando) {
        int largura_janela = al_get_display_width(janela);
        int altura_janela = al_get_display_height(janela);

        float escala = 1;
        float largura_visivel = 1000;

        if (largura_janela > 0 && altura_janela > 0) {
            escala = altura_janela / ALTURA_MUNDO;
            largura_visivel = largura_janela / escala;
        }

        // Acompanho a personagem sem ultrapassar os limites do cenário.
        camera_x = limitar_camera(
            personagem.x + personagem.largura / 2
                - largura_visivel / 2,
            largura_fase,
            largura_visivel
        );

        if (redesenhar && al_is_event_queue_empty(fila)) {
            if (largura_janela > 0 && altura_janela > 0) {
                al_clear_to_color(al_map_rgb(0, 0, 0));

                int deslocamento = pixel(camera_x * escala);

                // Desenho os fundos alinhados em pixels inteiros.
                for (int i = 0; i < QUANTIDADE_FUNDOS; i++) {
                    int x1 =
                        pixel(posicoes[i] * escala) - deslocamento;

                    int x2 =
                        pixel((posicoes[i] + larguras[i]) * escala)
                        - deslocamento;

                    if (x1 < largura_janela && x2 > 0) {
                        al_draw_scaled_bitmap(
                            fundos[i], 0, 0,
                            al_get_bitmap_width(fundos[i]),
                            al_get_bitmap_height(fundos[i]),
                            (float)x1, 0,
                            (float)(x2 - x1),
                            (float)altura_janela,
                            0
                        );
                    }
                }

                int topo_chao = pixel(CHAO_Y * escala);

                int primeira =
                    (int)(camera_x / largura_peca_chao);

                int ultima =
                    (int)(
                        (camera_x + largura_visivel)
                        / largura_peca_chao
                    ) + 1;

                // Repito o chão ao longo do percurso.
                for (int i = primeira; i < ultima; i++) {
                    float inicio = i * largura_peca_chao;
                    float fim = (i + 1) * largura_peca_chao;

                    if (inicio >= largura_fase) break;
                    if (fim > largura_fase) fim = largura_fase;

                    int x1 = pixel(inicio * escala) - deslocamento;
                    int x2 = pixel(fim * escala) - deslocamento;

                    float recorte =
                        largura_chao * (fim - inicio)
                        / largura_peca_chao;

                    if (x2 > x1) {
                        al_draw_scaled_bitmap(
                            chao, 0, 0,
                            recorte, (float)altura_chao,
                            (float)x1, (float)topo_chao,
                            (float)(x2 - x1),
                            (float)(altura_janela - topo_chao),
                            0
                        );
                    }
                }

                // Escolho pedra para os blocos e plataforma para os suspensos.
                for (int i = 0; i < quantidade_objetos; i++) {
                    float base = objetos[i].y + objetos[i].altura;

                    Textura *textura =
                        base >= CHAO_Y - 0.1f
                        ? &pedra : &plataforma;

                    int x1 =
                        pixel(objetos[i].x * escala) - deslocamento;

                    int x2 =
                        pixel(
                            (objetos[i].x + objetos[i].largura)
                            * escala
                        ) - deslocamento;

                    int y1 = pixel(objetos[i].y * escala);

                    int y2 = pixel(
                        (objetos[i].y + objetos[i].altura)
                        * escala
                    );

                    ALLEGRO_COLOR cor =
                        verificar_contato(personagem, objetos[i])
                        ? al_map_rgb(255, 180, 180)
                        : al_map_rgb(255, 255, 255);

                    if (x1 < largura_janela && x2 > 0) {
                        desenhar_textura(
                            textura,
                            (float)x1, (float)y1,
                            (float)(x2 - x1), (float)(y2 - y1),
                            cor
                        );
                    }
                }

                // Desenho somente as moedas que ainda não coletei.
                for (int i = 0; i < quantidade_moedas; i++) {
                    if (moedas[i].coletada) continue;

                    int x =
                        pixel(moedas[i].x * escala) - deslocamento;

                    int y = pixel(moedas[i].y * escala);

                    int tamanho = pixel(moedas[i].tamanho * escala);

                    if (x < largura_janela && x + tamanho > 0) {
                        desenhar_textura(
                            &textura_moeda,
                            (float)x, (float)y,
                            (float)tamanho, (float)tamanho,
                            al_map_rgb(255, 255, 255)
                        );
                    }
                }

                // Inverto a imagem quando caminho para a esquerda.
                al_draw_scaled_bitmap(
                    sprite, 0, 0,
                    largura_sprite, altura_sprite,
                    (float)(
                        pixel(personagem.x * escala)
                        - deslocamento
                    ),
                    (float)pixel(personagem.y * escala),
                    (float)pixel(personagem.largura * escala),
                    (float)pixel(personagem.altura * escala),
                    olhando_esquerda ? ALLEGRO_FLIP_HORIZONTAL : 0
                );

                desenhar_contador(
                    fonte, &textura_moeda,
                    moedas_coletadas, quantidade_moedas,
                    escala
                );

                // Exibo o card correspondente ao estado atual da fase.
                if (estado == APRESENTACAO) {
                    card_desenhar(
                        &card,
                        largura_janela, altura_janela
                    );
                } else if (estado == DESAFIO) {
                    pergunta_desenhar(
                        &pergunta,
                        card.imagem,
                        fonte,
                        largura_janela,
                        altura_janela,
                        al_get_time()
                    );
                } else if (estado == FASE_CONCLUIDA) {
                    desenhar_conclusao(
                        fonte,
                        largura_janela, altura_janela,
                        escala,
                        moedas_coletadas, quantidade_moedas
                    );
                }

                al_flip_display();
            }

            redesenhar = 0;
        }

        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        switch (evento.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                rodando = 0;
                break;

            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                al_acknowledge_resize(janela);
                redesenhar = 1;
                break;

            case ALLEGRO_EVENT_DISPLAY_EXPOSE:
            case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
                redesenhar = 1;
                break;

            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                // Limpo as direções quando saio da janela.
                esquerda = 0;
                direita = 0;
                break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (evento.mouse.button != 1) break;

                if (estado == APRESENTACAO) {
                    if (card_processar_clique(
                            &card,
                            evento.mouse.x, evento.mouse.y,
                            al_get_display_width(janela),
                            al_get_display_height(janela)
                        )) {
                        // Inicio a exploração após clicar no botão do card.
                        esquerda = 0;
                        direita = 0;
                        estado = EXPLORANDO;
                        al_start_timer(timer);
                        redesenhar = 1;
                    }
                } else if (estado == DESAFIO) {
                    AcaoPergunta acao = pergunta_processar_clique(
                        &pergunta,
                        card.imagem,
                        evento.mouse.x,
                        evento.mouse.y,
                        al_get_display_width(janela),
                        al_get_display_height(janela),
                        al_get_time()
                    );

                    if (acao == PERGUNTA_CONTINUAR) {
                        estado = FASE_CONCLUIDA;
                        al_stop_timer(timer);
                    }

                    redesenhar = 1;
                }
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                // Permito apenas os controles da janela fora da exploração.
                if (estado != EXPLORANDO &&
                    evento.keyboard.keycode != ALLEGRO_KEY_F11 &&
                    evento.keyboard.keycode != ALLEGRO_KEY_ESCAPE) {
                    break;
                }

                switch (evento.keyboard.keycode) {
                    case ALLEGRO_KEY_LEFT:
                        esquerda = 1;
                        olhando_esquerda = 1;
                        break;

                    case ALLEGRO_KEY_RIGHT:
                        direita = 1;
                        olhando_esquerda = 0;
                        break;

                    case ALLEGRO_KEY_UP:
                        // Permito pular apenas quando estou apoiada.
                        if (personagem.apoiado) {
                            personagem.velocidade_y = -15;
                            personagem.apoiado = 0;
                        }
                        break;

                    case ALLEGRO_KEY_F11: {
                        // Alterno entre tela cheia e modo janela.
                        int tela_cheia =
                            (al_get_display_flags(janela)
                             & ALLEGRO_FULLSCREEN_WINDOW) != 0;

                        if (!al_toggle_display_flag(
                                janela,
                                ALLEGRO_FULLSCREEN_WINDOW,
                                !tela_cheia
                            )) {
                            fprintf(stderr, "Erro ao alternar a tela.\n");
                        }

                        redesenhar = 1;
                        break;
                    }

                    case ALLEGRO_KEY_ESCAPE:
                        // Uso Esc apenas para sair da tela cheia.
                        if (al_get_display_flags(janela)
                            & ALLEGRO_FULLSCREEN_WINDOW) {

                            if (!al_toggle_display_flag(
                                    janela,
                                    ALLEGRO_FULLSCREEN_WINDOW,
                                    false
                                )) {
                                fprintf(
                                    stderr,
                                    "Erro ao sair da tela cheia.\n"
                                );
                            }

                            redesenhar = 1;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case ALLEGRO_EVENT_KEY_UP:
                if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
                    esquerda = 0;

                if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                    direita = 0;
                break;

            case ALLEGRO_EVENT_TIMER: {
                double agora = al_get_time();

                if (estado == EXPLORANDO) {
                    atualizar_movimento(
                        &personagem,
                        objetos,
                        quantidade_objetos,
                        esquerda,
                        direita,
                        largura_fase
                    );

                    moedas_coletadas += coletar_moedas(
                        personagem,
                        moedas,
                        quantidade_moedas
                    );

                    if (quantidade_moedas > 0 &&
                        moedas_coletadas == quantidade_moedas) {
                        // Pauso a exploração e inicio a abertura suave.
                        esquerda = 0;
                        direita = 0;
                        estado = DESAFIO;
                        pergunta_iniciar(&pergunta, agora);
                    }

                    redesenhar = 1;
                } else if (estado == DESAFIO) {
                    if (pergunta_atualizar(&pergunta, agora)) {
                        redesenhar = 1;
                    }

                    if (pergunta_deve_reiniciar(&pergunta, agora)) {
                        // Restauro a fase sem recarregar as imagens.
                        personagem.x = 150;
                        personagem.y = CHAO_Y - personagem.altura;
                        personagem.velocidade_y = 0;
                        personagem.apoiado = 1;

                        for (int i = 0; i < quantidade_moedas; i++) {
                            moedas[i].coletada = 0;
                        }

                        moedas_coletadas = 0;
                        camera_x = 0;
                        esquerda = 0;
                        direita = 0;
                        olhando_esquerda = 0;

                        card.aberto = 1;
                        pergunta_iniciar(&pergunta, agora);

                        estado = APRESENTACAO;
                        al_stop_timer(timer);
                        redesenhar = 1;
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    resultado = 0;

finalizar:
    // Libero todos os recursos, inclusive em caso de erro.
    if (timer != NULL) al_stop_timer(timer);
    if (fila != NULL) al_destroy_event_queue(fila);
    if (timer != NULL) al_destroy_timer(timer);

    pergunta_destruir(&pergunta);
    card_destruir(&card);

    if (fonte != NULL) al_destroy_font(fonte);
    if (sprite != NULL) al_destroy_bitmap(sprite);
    if (chao != NULL) al_destroy_bitmap(chao);
    if (pedra.imagem != NULL) al_destroy_bitmap(pedra.imagem);

    if (plataforma.imagem != NULL)
        al_destroy_bitmap(plataforma.imagem);

    if (textura_moeda.imagem != NULL)
        al_destroy_bitmap(textura_moeda.imagem);

    for (int i = 0; i < QUANTIDADE_FUNDOS; i++) {
        if (fundos[i] != NULL) al_destroy_bitmap(fundos[i]);
    }

    if (janela != NULL) al_destroy_display(janela);

    if (fontes_iniciadas) al_shutdown_font_addon();
    if (primitivas_iniciadas) al_shutdown_primitives_addon();
    if (mouse_iniciado) al_uninstall_mouse();
    if (teclado_iniciado) al_uninstall_keyboard();
    if (imagens_iniciadas) al_shutdown_image_addon();

    al_uninstall_system();

    return resultado;
}