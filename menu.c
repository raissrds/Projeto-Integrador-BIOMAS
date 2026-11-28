#include <stdio.h>

#include <allegro5/allegro_image.h>

#include "menu.h"

// Guardo os limites da área clicável de cada botão.
typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;

    MenuAcao acao;
} AreaBotao;

// Uso as posições dos botões na imagem original do menu.
static const AreaBotao botoes[] = {
    {570, 425, 1100, 545, MENU_JOGAR},
    {570, 575, 1100, 690, MENU_INSTRUCOES}, 
    {570, 720, 1100, 835, MENU_SAIR}
};

static int menu_atualizar_layout(
    Menu *menu,
    ALLEGRO_BITMAP *imagem,
    int largura_janela,
    int altura_janela
) {
    if (imagem == NULL ||
        largura_janela <= 0 ||
        altura_janela <= 0) {
        return 0;
    }

    // Consulto as dimensões da imagem que será exibida.
    int largura_imagem = al_get_bitmap_width(imagem);
    int altura_imagem = al_get_bitmap_height(imagem);

    float escala_x = (float)largura_janela / largura_imagem;
    float escala_y = (float)altura_janela / altura_imagem;

    // Escolho a menor escala para evitar cortes e distorções.
    menu->escala = escala_x < escala_y ? escala_x : escala_y;

    float largura_desenho = largura_imagem * menu->escala;
    float altura_desenho = altura_imagem * menu->escala;

    // Centralizo a imagem no espaço disponível.
    menu->margem_x = (largura_janela - largura_desenho) / 2.0f;
    menu->margem_y = (altura_janela - altura_desenho) / 2.0f;

    return 1;
}

int menu_carregar(
    Menu *menu,
    const char *caminho_menu,
    const char *caminho_instrucoes
) {
    menu->imagem = NULL;
    menu->imagem_instrucoes = NULL;

    menu->escala = 1;
    menu->margem_x = 0;
    menu->margem_y = 0;
    menu->tela = MENU_TELA_INICIAL;

    menu->imagem = al_load_bitmap(caminho_menu);

    if (menu->imagem == NULL) {
        fprintf(
            stderr,
            "Erro ao carregar a imagem: %s\n",
            caminho_menu
        );
        return 0;
    }

    menu->imagem_instrucoes = al_load_bitmap(caminho_instrucoes);

    if (menu->imagem_instrucoes == NULL) {
        fprintf(
            stderr,
            "Erro ao carregar a imagem: %s\n",
            caminho_instrucoes
        );

        // Libero a primeira imagem se a segunda não carregar.
        menu_destruir(menu);
        return 0;
    }

    return 1;
}

void menu_desenhar(
    Menu *menu,
    int largura_janela,
    int altura_janela
) {
    // Escolho a imagem de acordo com a tela atual.
    ALLEGRO_BITMAP *imagem = menu->imagem;

    if (menu->tela == MENU_TELA_INSTRUCOES) {
        imagem = menu->imagem_instrucoes;
    }

    if (!menu_atualizar_layout(
            menu,
            imagem,
            largura_janela,
            altura_janela
        )) {
        return;
    }

    int largura_imagem = al_get_bitmap_width(imagem);
    int altura_imagem = al_get_bitmap_height(imagem);

    al_clear_to_color(al_map_rgb(20, 25, 20));

    // Desenho a imagem inteira, mantendo sua proporção.
    al_draw_scaled_bitmap(
        imagem,
        0, 0,
        largura_imagem, altura_imagem,
        menu->margem_x, menu->margem_y,
        largura_imagem * menu->escala,
        altura_imagem * menu->escala,
        0
    );
}

MenuAcao menu_processar_clique(
    Menu *menu,
    int mouse_x,
    int mouse_y,
    int largura_janela,
    int altura_janela
) {
    // Só verifico os botões enquanto o menu inicial está aberto.
    if (menu->tela != MENU_TELA_INICIAL) {
        return MENU_NENHUMA;
    }

    if (!menu_atualizar_layout(
            menu,
            menu->imagem,
            largura_janela,
            altura_janela
        )) {
        return MENU_NENHUMA;
    }

    // Converto o clique para as coordenadas da imagem original.
    float x = (mouse_x - menu->margem_x) / menu->escala;
    float y = (mouse_y - menu->margem_y) / menu->escala;

    for (int i = 0; i < 3; i++) {
        if (x >= botoes[i].x1 &&
            x <= botoes[i].x2 &&
            y >= botoes[i].y1 &&
            y <= botoes[i].y2) {

            switch (botoes[i].acao) {
                case MENU_JOGAR:
                    // Confirmo o clique sem iniciar uma fase.
                    printf("Botao JOGAR selecionado.\n");
                    break;

                case MENU_INSTRUCOES:
                    menu->tela = MENU_TELA_INSTRUCOES;
                    break;

                case MENU_SAIR:
                    printf("Botao SAIR selecionado.\n");
                    break;

                default:
                    break;
            }

            return botoes[i].acao;
        }
    }

    return MENU_NENHUMA;
}

int menu_processar_tecla(Menu *menu, int tecla) {
    // Uso somente Enter para voltar das instruções ao menu.
    if (menu->tela == MENU_TELA_INSTRUCOES &&
        tecla == ALLEGRO_KEY_ENTER) {

        menu->tela = MENU_TELA_INICIAL;
        return 1;
    }

    return 0;
}

void menu_destruir(Menu *menu) {
    // Libero as duas imagens antes de encerrar o programa.
    if (menu->imagem_instrucoes != NULL) {
        al_destroy_bitmap(menu->imagem_instrucoes);
        menu->imagem_instrucoes = NULL;
    }

    if (menu->imagem != NULL) {
        al_destroy_bitmap(menu->imagem);
        menu->imagem = NULL;
    }
}