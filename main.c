#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "menu.h"
#include "fase.h"
#include "transicao.h"

typedef enum {
    TELA_MENU,
    TELA_FASE,
    TELA_TRANSICAO
} TelaJogo;

int main(void) {
    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_FONT *fonte = NULL;

    Menu menu = {0};
    Fase fase = {0};
    Transicao transicao = {0};

    TelaJogo tela = TELA_MENU;
    Bioma destino = BIOMA_AMAZONIA;

    int origem_menu = 1;

    int rodando = 1;
    int redesenhar = 1;
    int resultado = 1;

    int imagens = 0;
    int primitivas = 0;
    int fontes = 0;
    int teclado = 0;
    int mouse = 0;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (!al_init()) {
        fprintf(stderr, "Erro ao inicializar a Allegro.\n");
        return 1;
    }

    imagens = al_init_image_addon();
    primitivas = al_init_primitives_addon();
    fontes = al_init_font_addon();
    teclado = al_install_keyboard();
    mouse = al_install_mouse();

    if (!imagens || !primitivas || !fontes || !teclado || !mouse) {
        fprintf(stderr, "Erro ao inicializar os recursos.\n");
        goto finalizar;
    }

    // Preservo a mesma janela e os pixels sem suavização.
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);

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

    al_set_window_title(
        janela,
        "BIOMAS - Desafio Selvagem"
    );

    fila = al_create_event_queue();
    timer = al_create_timer(1.0 / 60.0);
    fonte = al_create_builtin_font();

    if (fila == NULL || timer == NULL || fonte == NULL) {
        fprintf(stderr, "Erro ao criar fila, timer ou fonte.\n");
        goto finalizar;
    }

    if (!menu_carregar(
            &menu,
            "imagens/menu/menu.png",
            "imagens/menu/instrucoes.png"
        )) {
        goto finalizar;
    }

    if (!transicao_carregar(&transicao)) {
        goto finalizar;
    }

    al_register_event_source(
        fila,
        al_get_display_event_source(janela)
    );

    al_register_event_source(
        fila,
        al_get_keyboard_event_source()
    );

    al_register_event_source(
        fila,
        al_get_mouse_event_source()
    );

    al_register_event_source(
        fila,
        al_get_timer_event_source(timer)
    );

    if (!al_toggle_display_flag(
            janela,
            ALLEGRO_FULLSCREEN_WINDOW,
            true
        )) {
        fprintf(stderr, "Continuando em modo janela.\n");
    }

    al_start_timer(timer);

    while (rodando) {
        if (redesenhar && al_is_event_queue_empty(fila)) {
            int largura = al_get_display_width(janela);
            int altura = al_get_display_height(janela);

            if (largura > 0 && altura > 0) {
                if (tela == TELA_MENU) {
                    menu_desenhar(
                        &menu,
                        largura,
                        altura
                    );
                } else if (tela == TELA_FASE) {
                    fase_desenhar(
                        &fase,
                        largura,
                        altura
                    );
                } else {
                    // Escureço a origem e revelo o card de destino no final.
                    if (
                        transicao.etapa ==
                        TRANSICAO_ESCURECENDO
                    ) {
                        if (origem_menu) {
                            menu_desenhar(
                                &menu,
                                largura,
                                altura
                            );
                        } else {
                            fase_desenhar(
                                &fase,
                                largura,
                                altura
                            );
                        }
                    } else if (
                        transicao.etapa ==
                        TRANSICAO_REVELANDO
                    ) {
                        fase_desenhar(
                            &fase,
                            largura,
                            altura
                        );
                    } else {
                        al_clear_to_color(
                            al_map_rgb(0, 0, 0)
                        );
                    }

                    transicao_desenhar(
                        &transicao,
                        largura,
                        altura,
                        al_get_time()
                    );
                }

                al_flip_display();

                if (tela == TELA_TRANSICAO) {
                    transicao_confirmar_quadro(
                        &transicao,
                        al_get_time()
                    );
                }
            }

            redesenhar = 0;
        }

        // Mostro a arte antes de carregar os recursos da próxima fase.
        if (tela == TELA_TRANSICAO &&
            transicao_precisa_carregar(&transicao)) {
            fase_destruir(&fase);

            if (!fase_carregar(
                    &fase,
                    fonte,
                    destino
                )) {
                fprintf(
                    stderr,
                    "Nao consegui carregar a proxima fase.\n"
                    "Confira os caminhos das imagens.\n"
                );
                goto finalizar;
            }

            al_set_window_title(
                janela,
                destino == BIOMA_AMAZONIA
                    ? "BIOMAS - Amazonia"
                    : "BIOMAS - Caatinga"
            );

            transicao_fase_carregada(
                &transicao,
                al_get_time()
            );

            redesenhar = 1;
        }

        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        int largura = al_get_display_width(janela);
        int altura = al_get_display_height(janela);

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

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (tela == TELA_MENU &&
                    evento.mouse.button == 1) {
                    MenuAcao acao = menu_processar_clique(
                        &menu,
                        evento.mouse.x,
                        evento.mouse.y,
                        largura,
                        altura
                    );

                    if (acao == MENU_SAIR) {
                        rodando = 0;
                    } else if (acao == MENU_JOGAR) {
                        // Inicio a primeira transição antes da Amazônia.
                        origem_menu = 1;
                        destino = BIOMA_AMAZONIA;

                        transicao_iniciar(
                            &transicao,
                            1,
                            al_get_time()
                        );

                        tela = TELA_TRANSICAO;
                    }
                } else if (tela == TELA_FASE) {
                    fase_processar_evento(
                        &fase,
                        &evento,
                        largura,
                        altura
                    );
                }

                // Ignoro os cliques enquanto a transição está ativa.
                redesenhar = 1;
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                if (evento.keyboard.keycode == ALLEGRO_KEY_F11) {
                    int cheia =
                        (al_get_display_flags(janela)
                         & ALLEGRO_FULLSCREEN_WINDOW) != 0;

                    if (!al_toggle_display_flag(
                            janela,
                            ALLEGRO_FULLSCREEN_WINDOW,
                            !cheia
                        )) {
                        fprintf(
                            stderr,
                            "Erro ao alternar a tela cheia.\n"
                        );
                    }
                } else if (
                    evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE
                ) {
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
                    }
                } else if (tela == TELA_MENU) {
                    menu_processar_tecla(
                        &menu,
                        evento.keyboard.keycode
                    );
                } else if (tela == TELA_FASE) {
                    fase_processar_evento(
                        &fase,
                        &evento,
                        largura,
                        altura
                    );
                }

                redesenhar = 1;
                break;

            case ALLEGRO_EVENT_KEY_UP:
            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                if (tela == TELA_FASE) {
                    fase_processar_evento(
                        &fase,
                        &evento,
                        largura,
                        altura
                    );
                }
                break;

            case ALLEGRO_EVENT_TIMER:
                if (tela == TELA_FASE) {
                    fase_atualizar(
                        &fase,
                        al_get_time()
                    );
                } else if (tela == TELA_TRANSICAO) {
                    transicao_atualizar(
                        &transicao,
                        al_get_time()
                    );

                    if (
                        transicao.etapa ==
                        TRANSICAO_FINALIZADA
                    ) {
                        tela = TELA_FASE;
                    }
                }

                redesenhar = 1;
                break;

            default:
                break;
        }

        if (rodando &&
            tela == TELA_FASE &&
            fase.bioma == BIOMA_AMAZONIA &&
            fase.estado == FASE_CONCLUIDA) {
            // Preservo a pergunta durante o fade de saída.
            fase.estado = DESAFIO;

            origem_menu = 0;
            destino = BIOMA_CAATINGA;

            transicao_iniciar(
                &transicao,
                0,
                al_get_time()
            );

            tela = TELA_TRANSICAO;
            redesenhar = 1;
        }
    }

    resultado = 0;

finalizar:
    // Libero os recursos antes da fonte e da janela compartilhadas.
    if (timer != NULL) al_stop_timer(timer);
    if (fila != NULL) al_destroy_event_queue(fila);
    if (timer != NULL) al_destroy_timer(timer);

    transicao_destruir(&transicao);
    fase_destruir(&fase);
    menu_destruir(&menu);

    if (fonte != NULL) al_destroy_font(fonte);
    if (janela != NULL) al_destroy_display(janela);

    if (mouse) al_uninstall_mouse();
    if (teclado) al_uninstall_keyboard();
    if (fontes) al_shutdown_font_addon();
    if (primitivas) al_shutdown_primitives_addon();
    if (imagens) al_shutdown_image_addon();

    al_uninstall_system();

    return resultado;
}