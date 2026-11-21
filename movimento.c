
#include "movimento.h"

void atualizar_movimento(
    Personagem *p,
    Objeto objeto,
    int esquerda,
    int direita
) {
    const float velocidade = 5;
    const float gravidade = 0.65f;
    const float largura_mundo = 800;
    const float chao_y = 550;

    // Movimento horizontal
    float movimento_x =
        (direita - esquerda) * velocidade;

    p->x += movimento_x;

    // Colisao horizontal com o objeto
    int sobreposicao_vertical =
        p->y < objeto.y + objeto.altura &&
        p->y + p->tamanho > objeto.y;

    if (sobreposicao_vertical &&
        p->x < objeto.x + objeto.largura &&
        p->x + p->tamanho > objeto.x) {

        if (movimento_x > 0) {
            p->x = objeto.x - p->tamanho;
        }
        else if (movimento_x < 0) {
            p->x = objeto.x + objeto.largura;
        }
    }

    // Limites laterais
    if (p->x < 0) {
        p->x = 0;
    }

    if (p->x > largura_mundo - p->tamanho) {
        p->x = largura_mundo - p->tamanho;
    }

    // Gravidade e movimento vertical
    float y_anterior = p->y;

    p->velocidade_y += gravidade;
    p->y += p->velocidade_y;
    p->apoiado = 0;

    int sobreposicao_horizontal =
        p->x < objeto.x + objeto.largura &&
        p->x + p->tamanho > objeto.x;

    if (sobreposicao_horizontal) {

        // Personagem caiu sobre o objeto
        if (p->velocidade_y >= 0 &&
            y_anterior + p->tamanho <= objeto.y &&
            p->y + p->tamanho >= objeto.y) {

            p->y = objeto.y - p->tamanho;
            p->velocidade_y = 0;
            p->apoiado = 1;
        }

        // Personagem bateu embaixo do objeto
        else if (p->velocidade_y < 0 &&
                 y_anterior >= objeto.y + objeto.altura &&
                 p->y <= objeto.y + objeto.altura) {

            p->y = objeto.y + objeto.altura;
            p->velocidade_y = 0;
        }
    }

    // Colisao com o chao
    if (p->y + p->tamanho >= chao_y) {
        p->y = chao_y - p->tamanho;
        p->velocidade_y = 0;
        p->apoiado = 1;
    }

    // Limite superior
    if (p->y < 0) {
        p->y = 0;
        p->velocidade_y = 0;
    }
}

// Verifica se o personagem encostou no objeto
int verificar_contato(Personagem p, Objeto objeto) {
    const float tolerancia = 0.1f;

    return
        p.x + p.tamanho >= objeto.x - tolerancia &&
        p.x <= objeto.x + objeto.largura + tolerancia &&
        p.y + p.tamanho >= objeto.y - tolerancia &&
        p.y <= objeto.y + objeto.altura + tolerancia;
}