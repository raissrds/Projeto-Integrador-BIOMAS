#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "menu.h"

int main(void) {
    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;

    Menu menu = {0};

    int rodando = 1;
    int redesenhar = 1;
    int resultado = 1;

    // Uso 1 para começar em tela cheia ou 0 para começar em janela.
    const int iniciar_em_tela_cheia = 1;

    // Mostro as mensagens imediatamente no terminal.
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (!al_init()) {
        fprintf(stderr, "Erro ao inicializar a Allegro.\n");
        return 1;
    }

    if (!al_init_image_addon()) {
        fprintf(stderr, "Erro ao inicializar o addon de imagens.\n");
        al_uninstall_system();
        return 1;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Erro ao inicializar o teclado.\n");
        al_shutdown_image_addon();
        al_uninstall_system();
        return 1;
    }

    if (!al_install_mouse()) {
        fprintf(stderr, "Erro ao inicializar o mouse.\n");
        al_uninstall_keyboard();
        al_shutdown_image_addon();
        al_uninstall_system();
        return 1;
    }

    // Evito a filtragem linear para preservar as bordas do pixel art.
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);

    // Mantenho OpenGL e permito redimensionar a janela.
    al_set_new_display_flags(
        ALLEGRO_WINDOWED |
        ALLEGRO_OPENGL |
        ALLEGRO_RESIZABLE
    );

    janela = al_create_display(1000, 563);

    if (janela == NULL) {
        fprintf(stderr, "Erro ao criar a janela.\n");
        goto finalizar;
    }

    al_set_window_title(janela, "BIOMAS: Desafio Selvagem");

    fila = al_create_event_queue();

    if (fila == NULL) {
        fprintf(stderr, "Erro ao criar a fila de eventos.\n");
        goto finalizar;
    }

    // Carrego as imagens sem utilizar caminhos absolutos.
    if (!menu_carregar(
            &menu,
            "imagens/menu.png",
            "imagens/instrucoes.png"
        )) {
        goto finalizar;
    }

    // Registro os eventos da janela, do teclado e do mouse.
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

    if (iniciar_em_tela_cheia) {
        if (!al_toggle_display_flag(
                janela,
                ALLEGRO_FULLSCREEN_WINDOW,
                true
            )) {
            fprintf(
                stderr,
                "Nao foi possivel ativar tela cheia.\n"
            );
        }
    }

    while (rodando) {
        // Redesenho quando necessário, após tratar os eventos pendentes.
        if (redesenhar && al_is_event_queue_empty(fila)) {
            menu_desenhar(
                &menu,
                al_get_display_width(janela),
                al_get_display_height(janela)
            );

            al_flip_display();
            redesenhar = 0;
        }

        ALLEGRO_EVENT evento;

        // Aguardo um evento para evitar processamento desnecessário.
        al_wait_for_event(fila, &evento);

        switch (evento.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                rodando = 0;
                break;

            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                // Confirmo o novo tamanho antes de redesenhar.
                al_acknowledge_resize(janela);
                redesenhar = 1;
                break;

            case ALLEGRO_EVENT_DISPLAY_EXPOSE:
            case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
                redesenhar = 1;
                break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
                // Encaminho apenas os cliques do botão esquerdo.
                if (evento.mouse.button == 1) {
                    MenuAcao acao = menu_processar_clique(
                        &menu,
                        evento.mouse.x,
                        evento.mouse.y,
                        al_get_display_width(janela),
                        al_get_display_height(janela)
                    );

                    if (acao == MENU_SAIR) {
                        rodando = 0;
                    } else if (acao != MENU_NENHUMA) {
                        redesenhar = 1;
                    }
                }

                break;
            }

            case ALLEGRO_EVENT_KEY_DOWN:
                // Deixo o menu verificar o Enter para voltar.
                if (menu_processar_tecla(
                        &menu,
                        evento.keyboard.keycode
                    )) {
                    redesenhar = 1;
                    break;
                }

                // O teclado envia F11 mesmo quando preciso usar Fn.
                if (evento.keyboard.keycode == ALLEGRO_KEY_F11) {
                    int tela_cheia =
                        (al_get_display_flags(janela) &
                         ALLEGRO_FULLSCREEN_WINDOW) != 0;

                    if (!al_toggle_display_flag(
                            janela,
                            ALLEGRO_FULLSCREEN_WINDOW,
                            !tela_cheia
                        )) {
                        fprintf(
                            stderr,
                            "Erro ao alternar o modo da janela.\n"
                        );
                    }

                    redesenhar = 1;
                }

                // Uso Esc somente para sair da tela cheia.
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    if (al_get_display_flags(janela) &
                        ALLEGRO_FULLSCREEN_WINDOW) {

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
                }
                break;

            default:
                break;
        }
    }

    resultado = 0;

finalizar:
    // Libero os recursos também quando ocorre um erro.
    menu_destruir(&menu);

    if (fila != NULL) {
        al_destroy_event_queue(fila);
    }

    if (janela != NULL) {
        al_destroy_display(janela);
    }

    al_uninstall_mouse();
    al_uninstall_keyboard();
    al_shutdown_image_addon();
    al_uninstall_system();

    return resultado;
}