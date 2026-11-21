# BIOMAS: Desafio Selvagem 

Jogo educativo de plataforma 2D sobre os biomas brasileiros, desenvolvido em C com Allegro 5 para um Projeto Integrador.

O público-alvo são estudantes do Ensino Médio. A proposta é combinar exploração, desafios de plataforma e conteúdos de Geografia.

## Integrantes

- Anna Luiza Rodrigues de Oliveira
- Raissa Rodrigues de Sousa

**Turma:** 2° semestre do Bacharelado em Ciência da Computação
**Disciplina:** Projeto Integrador

## Versão atual

A branch `main` contém a implementação do menu inicial, com:

- Tela inicial em pixel art.
- Botões Jogar, Instruções e Sair.
- Tela de instruções com retorno ao menu.
- Redimensionamento da janela e controle de tela cheia.
- Encerramento com liberação dos recursos da Allegro.

O botão **Jogar** realiza uma confirmação temporária. A ligação com a fase da Amazônia será adicionada durante a integração.

## Organização do desenvolvimento

As funcionalidades são desenvolvidas e testadas por etapas antes de serem integradas.

A fase da Amazônia foi desenvolvida separadamente e será adicionada em uma branch própria. Ela inclui movimentação, salto, câmera lateral, obstáculos, moedas e um desafio educativo.

## Biomas do jogo

A progressão planejada segue esta ordem:

1. Amazônia
2. Cerrado
3. Caatinga
4. Pantanal

Cada fase terá apresentação do bioma, exploração, moedas, placas educativas e uma pergunta de múltipla escolha.

## Tecnologias

- Linguagem C
- Allegro 5
- GCC
- MSYS2 UCRT64
- Visual Studio Code
- Git e GitHub

## Estrutura do menu

```text
.
├── .gitignore
├── README.md
├── main.c
├── menu.c
├── menu.h
├── .vscode/
│   └── tasks.json
└── imagens/
    ├── menu.png
    └── instrucoes.png
```

## Como compilar e executar

É necessário ter o compilador GCC e a Allegro 5 com o addon de imagens instalados.

A configuração de compilação utiliza o MSYS2 instalado em `C:\msys64`.

1. Abra a pasta do projeto no VS Code.
2. Pressione `Ctrl + Shift + B`.
3. Selecione a tarefa de compilação do menu, caso solicitado.

Os executáveis são gerados localmente e não são versionados no repositório.

## Controles do menu

- **Mouse:** selecionar os botões.
- **Enter:** retornar das instruções ao menu.
- **F11 ou Fn + F11:** alternar tela cheia.
- **Esc:** sair da tela cheia.
- **Botão Sair ou X da janela:** encerrar o programa.

## Próximas etapas

- Publicar a fase da Amazônia em sua branch.
- Integrar o menu ao fluxo de exploração.
- Desenvolver e adaptar os demais biomas.
- Integrar a progressão entre fases e a conclusão do jogo.

