#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "movimento.h"

int main(void) {
    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *sprite = NULL;

    Personagem personagem = {0};

    // Organizo os obstáculos: x, y, largura e altura.
    Objeto objetos[] = {
        {330, CHAO_Y - 60, 60, 60},
        {520, CHAO_Y - 80, 80, 80},
        {710, CHAO_Y - 100, 100, 100},
        {900, CHAO_Y - 70, 70, 70}
    };

    // Calculo a quantidade sem precisar contar manualmente.
    const int quantidade_objetos =
        (int)(sizeof(objetos) / sizeof(objetos[0]));

    int resultado = 1;
    int rodando = 1;
    int redesenhar = 1;

    int esquerda = 0;
    int direita = 0;

    int largura_sprite = 0;
    int altura_sprite = 0;

    // Uso 0 se quiser começar em modo janela.
    const int iniciar_em_tela_cheia = 1;

    float largura_mundo = 800;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    // Inicializo os recursos necessários para executar o jogo.
    if (!al_init()) {
        fprintf(stderr, "Erro ao inicializar a Allegro.\n");
        return 1;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Erro ao inicializar o teclado.\n");
        al_uninstall_system();
        return 1;
    }

    if (!al_init_image_addon()) {
        fprintf(stderr, "Erro ao inicializar as imagens.\n");
        al_uninstall_keyboard();
        al_uninstall_system();
        return 1;
    }

    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Erro ao inicializar as primitivas.\n");
        al_shutdown_image_addon();
        al_uninstall_keyboard();
        al_uninstall_system();
        return 1;
    }

    // Preservo as bordas do pixel art sem filtragem linear.
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);

    // Uso OpenGL e permito redimensionar a janela.
    al_set_new_display_flags(
        ALLEGRO_WINDOWED |
        ALLEGRO_OPENGL |
        ALLEGRO_RESIZABLE
    );

    janela = al_create_display(1000, 600);

    if (janela == NULL) {
        fprintf(stderr, "Erro ao criar a janela.\n");
        goto finalizar;
    }

    al_set_window_title(
        janela,
        "BIOMAS - Personagem e obstaculos"
    );

    fila = al_create_event_queue();
    timer = al_create_timer(1.0 / 60.0);

    if (fila == NULL || timer == NULL) {
        fprintf(stderr, "Erro ao criar a fila ou o timer.\n");
        goto finalizar;
    }

    // Carrego a imagem utilizada para desenhar a personagem.
    sprite = al_load_bitmap("imagens/personagem.png");

    if (sprite == NULL) {
        fprintf(
            stderr,
            "Erro ao carregar imagens/personagem.png\n"
        );
        goto finalizar;
    }

    largura_sprite = al_get_bitmap_width(sprite);
    altura_sprite = al_get_bitmap_height(sprite);

    if (largura_sprite <= 0 || altura_sprite <= 0) {
        fprintf(stderr, "Dimensoes da imagem invalidas.\n");
        goto finalizar;
    }

    // Defino a altura e calculo a largura sem deformar a imagem.
    personagem.altura = 80;

    personagem.largura =
        personagem.altura * (float)largura_sprite / altura_sprite;

    personagem.x = 150;
    personagem.y = CHAO_Y - personagem.altura;
    personagem.velocidade_y = 0;
    personagem.apoiado = 1;

    printf(
        "Personagem carregada: %d x %d pixels.\n",
        largura_sprite,
        altura_sprite
    );

    printf("Obstaculos carregados: %d\n", quantidade_objetos);

    // Registro os eventos da janela, do teclado e do timer.
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

    al_start_timer(timer);

    while (rodando) {
        int largura_janela = al_get_display_width(janela);
        int altura_janela = al_get_display_height(janela);

        float escala = 1;

        if (largura_janela > 0 && altura_janela > 0) {
            // Amplio a área horizontal para preencher a janela.
            escala = altura_janela / ALTURA_MUNDO;
            largura_mundo = largura_janela / escala;
        }

        if (redesenhar && al_is_event_queue_empty(fila)) {
            if (largura_janela > 0 && altura_janela > 0) {
                ALLEGRO_TRANSFORM transformacao;

                al_identity_transform(&transformacao);
                al_scale_transform(
                    &transformacao,
                    escala,
                    escala
                );

                al_use_transform(&transformacao);

                al_clear_to_color(al_map_rgb(255, 255, 255));

                // Desenho o chão em toda a largura disponível.
                al_draw_filled_rectangle(
                    0, CHAO_Y,
                    largura_mundo, ALTURA_MUNDO,
                    al_map_rgb(100, 100, 100)
                );

                // Desenho e verifico o contato de cada obstáculo.
                for (int i = 0; i < quantidade_objetos; i++) {
                    int encostando =
                        verificar_contato(personagem, objetos[i]);

                    ALLEGRO_COLOR cor_objeto = encostando
                        ? al_map_rgb(255, 0, 0)
                        : al_map_rgb(0, 180, 0);

                    al_draw_filled_rectangle(
                        objetos[i].x,
                        objetos[i].y,
                        objetos[i].x + objetos[i].largura,
                        objetos[i].y + objetos[i].altura,
                        cor_objeto
                    );
                }

                // Desenho a personagem na posição calculada pela física.
                al_draw_scaled_bitmap(
                    sprite,
                    0, 0,
                    largura_sprite, altura_sprite,
                    personagem.x, personagem.y,
                    personagem.largura, personagem.altura,
                    0
                );

                al_flip_display();
            }

            redesenhar = 0;
        }

        ALLEGRO_EVENT evento;

        // Aguardo eventos para evitar processamento desnecessário.
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
                // Evito manter o movimento ao trocar de janela.
                esquerda = 0;
                direita = 0;
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                switch (evento.keyboard.keycode) {
                    case ALLEGRO_KEY_LEFT:
                        esquerda = 1;
                        break;

                    case ALLEGRO_KEY_RIGHT:
                        direita = 1;
                        break;

                    case ALLEGRO_KEY_UP:
                        // Permito saltar quando a personagem está apoiada.
                        if (personagem.apoiado) {
                            personagem.velocidade_y = -15;
                            personagem.apoiado = 0;
                        }
                        break;

                    case ALLEGRO_KEY_F11: {
                        // Recebo F11 mesmo quando o teclado exige Fn.
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
                                "Erro ao alternar a tela cheia.\n"
                            );
                        }

                        redesenhar = 1;
                        break;
                    }

                    case ALLEGRO_KEY_ESCAPE:
                        // Uso Esc somente para sair da tela cheia.
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
                        break;

                    default:
                        break;
                }
                break;

            case ALLEGRO_EVENT_KEY_UP:
                if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    esquerda = 0;
                }

                if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    direita = 0;
                }
                break;

            case ALLEGRO_EVENT_TIMER:
                // Verifico o movimento considerando todos os obstáculos.
                atualizar_movimento(
                    &personagem,
                    objetos,
                    quantidade_objetos,
                    esquerda,
                    direita,
                    largura_mundo
                );

                redesenhar = 1;
                break;

            default:
                break;
        }
    }

    resultado = 0;

finalizar:
    // Libero os recursos também quando ocorre algum erro.
    if (timer != NULL) {
        al_stop_timer(timer);
    }

    if (fila != NULL) {
        al_destroy_event_queue(fila);
    }

    if (timer != NULL) {
        al_destroy_timer(timer);
    }

    if (sprite != NULL) {
        al_destroy_bitmap(sprite);
    }

    if (janela != NULL) {
        al_destroy_display(janela);
    }

    al_shutdown_primitives_addon();
    al_shutdown_image_addon();
    al_uninstall_keyboard();
    al_uninstall_system();

    return resultado;
}