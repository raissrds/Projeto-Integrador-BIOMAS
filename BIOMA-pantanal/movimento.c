#include "movimento.h"

void atualizar_movimento(
    Personagem *p,
    const Objeto objetos[],
    int quantidade_objetos,
    int esquerda,
    int direita,
    float largura_mundo
) {
    const float velocidade = 5.0f;
    const float gravidade = 0.65f;

    // Calculo a direção do movimento horizontal.
    float movimento_x =
        (direita - esquerda) * velocidade; 

    p->x += movimento_x;

    // Verifico as laterais de todos os obstáculos.
    for (int i = 0; i < quantidade_objetos; i++) {
        Objeto objeto = objetos[i];

        int sobreposicao_vertical =
            p->y < objeto.y + objeto.altura &&
            p->y + p->altura > objeto.y;

        int sobreposicao_horizontal =
            p->x < objeto.x + objeto.largura &&
            p->x + p->largura > objeto.x;

        if (sobreposicao_vertical &&
            sobreposicao_horizontal) {
            if (movimento_x > 0) {
                p->x = objeto.x - p->largura;
            } else if (movimento_x < 0) {
                p->x = objeto.x + objeto.largura;
            }
        }
    }

    // Imp impeço que a personagem ultrapasse as laterais da fase.
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

    // Guardo a posição anterior para identificar as colisões verticais.
    float y_anterior = p->y;

    p->velocidade_y += gravidade;
    p->y += p->velocidade_y;
    p->apoiado = 0;

    // Resolvo o movimento de queda sobre as plataformas.
    if (p->velocidade_y >= 0) {
        float superficie = CHAO_Y;
        int encontrou_superficie = 0;

        for (int i = 0; i < quantidade_objetos; i++) {
            Objeto objeto = objetos[i];

            int sobreposicao_horizontal =
                p->x < objeto.x + objeto.largura &&
                p->x + p->largura > objeto.x;

            if (sobreposicao_horizontal &&
                y_anterior + p->altura <= objeto.y &&
                p->y + p->altura >= objeto.y) {
                // Escolho a primeira superfície encontrada durante a queda.
                if (!encontrou_superficie ||
                    objeto.y < superficie) {
                    superficie = objeto.y;
                    encontrou_superficie = 1;
                }
            }
        }

        if (encontrou_superficie) {
            p->y = superficie - p->altura;
            p->velocidade_y = 0;
            p->apoiado = 1;
        }
    } else {
        // Verifico se bato na parte inferior de alguma plataforma.
        float teto = 0;
        int encontrou_teto = 0;

        for (int i = 0; i < quantidade_objetos; i++) {
            Objeto objeto = objetos[i];

            int sobreposicao_horizontal =
                p->x < objeto.x + objeto.largura &&
                p->x + p->largura > objeto.x;

            float base = objeto.y + objeto.altura;

            if (sobreposicao_horizontal &&
                y_anterior >= base &&
                p->y <= base) {
                if (!encontrou_teto || base > teto) {
                    teto = base;
                    encontrou_teto = 1;
                }
            }
        }

        if (encontrou_teto) {
            p->y = teto;
            p->velocidade_y = 0;
        }
    }

    // Mantenho a personagem sobre o chão.
    if (p->y + p->altura >= CHAO_Y) {
        p->y = CHAO_Y - p->altura;
        p->velocidade_y = 0;
        p->apoiado = 1;
    }

    // Imp impeço que a personagem saia pelo topo do mundo.
    if (p->y < 0) {
        p->y = 0;
        p->velocidade_y = 0;
    }
}

int verificar_contato(
    Personagem personagem,
    Objeto objeto
) {
    const float tolerancia = 0.1f;

    // Considero também o contato exato com as bordas.
    return
        personagem.x + personagem.largura >=
            objeto.x - tolerancia &&
        personagem.x <=
            objeto.x + objeto.largura + tolerancia &&
        personagem.y + personagem.altura >=
            objeto.y - tolerancia &&
        personagem.y <=
            objeto.y + objeto.altura + tolerancia;
}