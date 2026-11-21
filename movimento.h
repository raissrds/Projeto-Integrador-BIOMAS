
#ifndef MOVIMENTO_H
#define MOVIMENTO_H

typedef struct {
    float x;
    float y;
    float tamanho;
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
    Objeto objeto,
    int esquerda,
    int direita
);

int verificar_contato(
    Personagem p,
    Objeto objeto
);

#endif