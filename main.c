
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include "movimento.h"

int main(void) {

    // Inicializacao do Allegro
    if (!al_init()) {
        return 1;
    }

    if (!al_install_keyboard()) {
        return 1;
    }

    if (!al_init_primitives_addon()) {
        return 1;
    }

    // Configuracao da janela
    al_set_new_display_flags(
        ALLEGRO_WINDOWED |
        ALLEGRO_OPENGL |
        ALLEGRO_RESIZABLE
    );

    ALLEGRO_DISPLAY *janela =
        al_create_display(800, 600);

    if (janela == NULL) {
        return 1;
    }

    al_set_window_title(
        janela,
        "Movimento, salto e colisao"
    );

    // Eventos e timer
    ALLEGRO_EVENT_QUEUE *fila =
        al_create_event_queue();

    ALLEGRO_TIMER *timer =
        al_create_timer(1.0 / 60.0);

    if (fila == NULL || timer == NULL) {

        if (timer != NULL) {
            al_destroy_timer(timer);
        }

        if (fila != NULL) {
            al_destroy_event_queue(fila);
        }

        al_destroy_display(janela);
        return 1;
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
        al_get_timer_event_source(timer)
    );

    // Coordenadas do mundo
    const float largura_mundo = 800;
    const float altura_mundo = 600;
    const float chao_y = 550;

    // Personagem: quadrado azul
    Personagem personagem = {
        150, 500, 50, 0, 1
    };

    // Objeto fixo
    Objeto objeto = {
        550, 470, 80, 80
    };

    int esquerda = 0;
    int direita = 0;
    int rodando = 1;

    al_start_timer(timer);

    while (rodando) {

        ALLEGRO_EVENT evento;

        al_wait_for_event(fila, &evento);

        // Fechar janela
        if (evento.type ==
            ALLEGRO_EVENT_DISPLAY_CLOSE) {

            rodando = 0;
        }

        // Redimensionamento
        if (evento.type ==
            ALLEGRO_EVENT_DISPLAY_RESIZE) {

            al_acknowledge_resize(janela);
        }

        // Perda de foco da janela
        if (evento.type ==
            ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {

            esquerda = 0;
            direita = 0;
        }

        // Teclas pressionadas
        if (evento.type ==
            ALLEGRO_EVENT_KEY_DOWN) {

            switch (evento.keyboard.keycode) {

                case ALLEGRO_KEY_LEFT:
                    esquerda = 1;
                    break;

                case ALLEGRO_KEY_RIGHT:
                    direita = 1;
                    break;

                case ALLEGRO_KEY_UP:

                    if (personagem.apoiado) {
                        personagem.velocidade_y = -15;
                        personagem.apoiado = 0;
                    }

                    break;

                case ALLEGRO_KEY_F11: {

                    int tela_cheia =
                        al_get_display_flags(janela) &
                        ALLEGRO_FULLSCREEN_WINDOW;

                    al_toggle_display_flag(
                        janela,
                        ALLEGRO_FULLSCREEN_WINDOW,
                        !tela_cheia
                    );

                    break;
                }

                case ALLEGRO_KEY_ESCAPE:

                    if (al_get_display_flags(janela) &
                        ALLEGRO_FULLSCREEN_WINDOW) {

                        al_toggle_display_flag(
                            janela,
                            ALLEGRO_FULLSCREEN_WINDOW,
                            false
                        );
                    }
                    else {
                        rodando = 0;
                    }

                    break;

                default:
                    break;
            }
        }

        // Teclas soltas
        if (evento.type ==
            ALLEGRO_EVENT_KEY_UP) {

            if (evento.keyboard.keycode ==
                ALLEGRO_KEY_LEFT) {

                esquerda = 0;
            }

            if (evento.keyboard.keycode ==
                ALLEGRO_KEY_RIGHT) {

                direita = 0;
            }
        }

        // Atualizacao do jogo
        if (evento.type ==
            ALLEGRO_EVENT_TIMER && rodando) {

            atualizar_movimento(
                &personagem,
                objeto,
                esquerda,
                direita
            );

            // Detectar contato com o objeto
            int encostando =
                verificar_contato(personagem, objeto);

            ALLEGRO_COLOR cor_objeto = encostando
                ? al_map_rgb(255, 0, 0)
                : al_map_rgb(0, 180, 0);

            // Dimensoes atuais da janela
            int largura_janela =
                al_get_display_width(janela);

            int altura_janela =
                al_get_display_height(janela);

            if (largura_janela <= 0 ||
                altura_janela <= 0) {

                continue;
            }

            // Escala proporcional
            float escala_x =
                largura_janela / largura_mundo;

            float escala_y =
                altura_janela / altura_mundo;

            float escala =
                escala_x < escala_y
                ? escala_x : escala_y;

            float margem_x =
                (largura_janela -
                largura_mundo * escala) / 2;

            float margem_y =
                (altura_janela -
                altura_mundo * escala) / 2;

            ALLEGRO_TRANSFORM transformacao;

            al_identity_transform(&transformacao);

            al_scale_transform(
                &transformacao,
                escala,
                escala
            );

            al_translate_transform(
                &transformacao,
                margem_x,
                margem_y
            );

            al_use_transform(&transformacao);

            // Fundo
            al_clear_to_color(
                al_map_rgb(30, 30, 30)
            );

            al_draw_filled_rectangle(
                0, 0,
                largura_mundo,
                altura_mundo,
                al_map_rgb(255, 255, 255)
            );

            // Chao
            al_draw_filled_rectangle(
                0, chao_y,
                largura_mundo,
                altura_mundo,
                al_map_rgb(100, 100, 100)
            );

            // Objeto fixo
            al_draw_filled_rectangle(
                objeto.x,
                objeto.y,
                objeto.x + objeto.largura,
                objeto.y + objeto.altura,
                cor_objeto
            );

            // Personagem azul
            al_draw_filled_rectangle(
                personagem.x,
                personagem.y,
                personagem.x + personagem.tamanho,
                personagem.y + personagem.tamanho,
                al_map_rgb(0, 0, 255)
            );

            al_flip_display();
        }
    }

    // Liberacao de recursos
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    al_shutdown_primitives_addon();
    al_uninstall_keyboard();

    return 0;
}