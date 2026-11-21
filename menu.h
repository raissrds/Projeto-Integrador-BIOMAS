#ifndef BIOMAS_MENU_H
#define BIOMAS_MENU_H

#include <allegro5/allegro.h>

// Identifico o botão selecionado.
typedef enum {
    MENU_NENHUMA,
    MENU_JOGAR,
    MENU_INSTRUCOES,
    MENU_SAIR
} MenuAcao;

// Controlo qual tela está aberta.
typedef enum {
    MENU_TELA_INICIAL,
    MENU_TELA_INSTRUCOES
} MenuTela;

typedef struct {
    ALLEGRO_BITMAP *imagem;
    ALLEGRO_BITMAP *imagem_instrucoes;

    float escala;
    float margem_x;
    float margem_y;

    MenuTela tela;
} Menu;

// Declaro as funções que serão utilizadas pelo main.c.
int menu_carregar(
    Menu *menu,
    const char *caminho_menu,
    const char *caminho_instrucoes
);

void menu_desenhar(
    Menu *menu,
    int largura_janela,
    int altura_janela
);

MenuAcao menu_processar_clique(
    Menu *menu,
    int mouse_x,
    int mouse_y,
    int largura_janela,
    int altura_janela
);

int menu_processar_tecla(Menu *menu, int tecla);

void menu_destruir(Menu *menu);

#endif