#include "movimento.h"

void atualizar_movimento(
    Personagem *p,
    const Objeto objetos[],
    int quantidade_objetos,
    int esquerda,
    int direita,
    float largura_mundo
) {
    const float velocidade = 5;
    const float gravidade = 0.65f;

    float movimento_x = (direita - esquerda) * velocidade;
    p->x += movimento_x;

    // Verifico as laterais dos obstáculos recebidos.
    for (int i = 0; i < quantidade_objetos; i++) {
        Objeto objeto = objetos[i];

        int sobreposicao_vertical =
            p->y < objeto.y + objeto.altura &&
            p->y + p->altura > objeto.y;

        if (sobreposicao_vertical &&
            p->x < objeto.x + objeto.largura &&
            p->x + p->largura > objeto.x) {

            if (movimento_x > 0) {
                p->x = objeto.x - p->largura;
            } else if (movimento_x < 0) {
                p->x = objeto.x + objeto.largura;
            }
        }
    }

    // Limito o movimento ao percurso completo.
    float limite_direito = largura_mundo - p->largura;

    if (limite_direito < 0) {
        limite_direito = 0;
    }

    if (p->x < 0) {
        p->x = 0;
    }

    if (p->x > limite_direito) {
        p->x = limite_direito;
    }

    float y_anterior = p->y;

    p->velocidade_y += gravidade;
    p->y += p->velocidade_y;
    p->apoiado = 0;

    // Verifico o apoio e as batidas por baixo dos obstáculos.
    for (int i = 0; i < quantidade_objetos; i++) {
        Objeto objeto = objetos[i];

        int sobreposicao_horizontal =
            p->x < objeto.x + objeto.largura &&
            p->x + p->largura > objeto.x;

        if (sobreposicao_horizontal) {
            if (p->velocidade_y >= 0 &&
                y_anterior + p->altura <= objeto.y &&
                p->y + p->altura >= objeto.y) {

                p->y = objeto.y - p->altura;
                p->velocidade_y = 0;
                p->apoiado = 1;
            } else if (p->velocidade_y < 0 &&
                       y_anterior >= objeto.y + objeto.altura &&
                       p->y <= objeto.y + objeto.altura) {

                p->y = objeto.y + objeto.altura;
                p->velocidade_y = 0;
            }
        }
    }

    // Apoio a personagem no chão.
    if (p->y + p->altura >= CHAO_Y) {
        p->y = CHAO_Y - p->altura;
        p->velocidade_y = 0;
        p->apoiado = 1;
    }

    if (p->y < 0) {
        p->y = 0;
        p->velocidade_y = 0;
    }
}

int verificar_contato(Personagem p, Objeto objeto) {
    const float tolerancia = 0.1f;

    return
        p.x + p.largura >= objeto.x - tolerancia &&
        p.x <= objeto.x + objeto.largura + tolerancia &&
        p.y + p.altura >= objeto.y - tolerancia &&
        p.y <= objeto.y + objeto.altura + tolerancia;
}