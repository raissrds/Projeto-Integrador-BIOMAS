#ifndef PERGUNTAS_H
#define PERGUNTAS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define PERGUNTA_TEMPO_ABERTURA 0.45
#define PERGUNTA_TEMPO_REINICIO 1.8

typedef enum {
    ESCOLHENDO_RESPOSTA,
    AGUARDANDO_REVISAO,
    LENDO_REVISAO,
    RESPOSTA_CORRETA,
    LIMITE_DE_ERROS
} EtapaPergunta;

typedef enum {
    PERGUNTA_NENHUMA,
    PERGUNTA_CONTINUAR
} AcaoPergunta;

typedef struct {
    ALLEGRO_BITMAP *imagem;

    int errou[4];
    int erros;
    int alternativa_correta;
    int animando;

    double inicio_abertura;
    double instante_erro;

    EtapaPergunta etapa;

    const char *explicacao1;
    const char *explicacao2;
} PerguntaFinal;

int pergunta_carregar(
    PerguntaFinal *p,
    const char *caminho,
    int correta,
    const char *explicacao1,
    const char *explicacao2
);

void pergunta_iniciar(PerguntaFinal *p, double agora);

void pergunta_responder(
    PerguntaFinal *p,
    int alternativa,
    double agora
);

void pergunta_revisar(PerguntaFinal *p);
void pergunta_voltar(PerguntaFinal *p);

int pergunta_atualizar(PerguntaFinal *p, double agora);

int pergunta_deve_reiniciar(
    const PerguntaFinal *p,
    double agora
);

void pergunta_desenhar(
    const PerguntaFinal *p,
    ALLEGRO_BITMAP *conteudo,
    ALLEGRO_FONT *fonte,
    int largura,
    int altura,
    double agora
);

AcaoPergunta pergunta_processar_clique(
    PerguntaFinal *p,
    ALLEGRO_BITMAP *conteudo,
    int mouse_x,
    int mouse_y,
    int largura,
    int altura,
    double agora
);

void pergunta_destruir(PerguntaFinal *p);

#endif