#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

int main() {

    al_init();
    al_install_keyboard();
    al_init_primitives_addon();

    // Usa janela normal com OpenGL
    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL);

    ALLEGRO_DISPLAY *janela = al_create_display(800, 600);

    if (janela == NULL) {
        return 1;
    }

    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

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

    // Posição do personagem
    float x = 375;
    float y = 275;

    // Velocidade
    float velocidade = 5;

    // Guarda se as teclas estão pressionadas
    int esquerda = 0;
    int direita = 0;

    int rodando = 1;

    al_start_timer(timer);

    while (rodando) {

        ALLEGRO_EVENT evento;

        al_wait_for_event(fila, &evento);

        // Fechar a janela
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = 0;
        }

        // Quando uma tecla é pressionada
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {

            if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                esquerda = 1;
            }

            if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                direita = 1;
            }

            if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                rodando = 0;
            }
        }

        // Quando uma tecla é solta
        if (evento.type == ALLEGRO_EVENT_KEY_UP) {

            if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                esquerda = 0;
            }

            if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                direita = 0;
            }
        }

        // Atualiza o personagem
        if (evento.type == ALLEGRO_EVENT_TIMER) {

            if (esquerda) {
                x -= velocidade;
            }

            if (direita) {
                x += velocidade;
            }

            // Limita o personagem dentro da tela
            if (x < 0) {
                x = 0;
            }

            if (x > 750) {
                x = 750;
            }

            // Limpa a tela
            al_clear_to_color(al_map_rgb(255, 255, 255));

            // Desenha o personagem
            al_draw_filled_rectangle(
                x,
                y,
                x + 50,
                y + 50,
                al_map_rgb(0, 0, 255)
            );

            // Atualiza a tela
            al_flip_display();
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    return 0;
}