#ifndef MOVIMENTO_H
#define MOVIMENTO_H

#define ALTURA_MUNDO 600.0f
#define CHAO_Y 550.0f

typedef struct {
    float x;
    float y;
    float largura;
    float altura;
    float velocidade_y;
    int apoiado;
} Personagem;

typedef struct {
    float x;
    float y; 
    float largura;
    float altura;
} Objeto;

void atualizar_movimento(
    Personagem *p,
    const Objeto objetos[],
    int quantidade_objetos,
    int esquerda,
    int direita,
    float largura_mundo
);

int verificar_contato(
    Personagem personagem,
    Objeto objeto
);

#endif