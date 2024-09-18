#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>  // Para _kbhit() e _getch()
#include <windows.h> // Para Sleep()

#define LINHAS 20
#define COLUNAS 10

int tabuleiro[LINHAS][COLUNAS] = {0};   
int score = 0;

// Função para limpar o terminal de maneira cross-platform, funciona no Windows e Unix
void limparTerminal() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Funções para manipulação da peça no tabuleiro
void limparPeca(int peca[4][2]) {
    for (int i = 0; i < 4; i++) {
        if (peca[i][0] >= 0 && peca[i][1] >= 0 && peca[i][0] < LINHAS && peca[i][1] < COLUNAS) {
            tabuleiro[peca[i][0]][peca[i][1]] = 0;
        }
    }
}

void colocarPeca(int peca[4][2]) {
    for (int i = 0; i < 4; i++) {
        if (peca[i][0] >= 0 && peca[i][1] >= 0 && peca[i][0] < LINHAS && peca[i][1] < COLUNAS) {
            tabuleiro[peca[i][0]][peca[i][1]] = 1;
        }
    }
}

// Função genérica para mover a peça no tabuleiro
void moverPeca(int peca[4][2], int deltaLinha, int deltaColuna) {
    limparPeca(peca);
    for (int i = 0; i < 4; i++) {
        int novaLinha = peca[i][0] + deltaLinha;
        int novaColuna = peca[i][1] + deltaColuna;
        if (novaLinha < 0 || novaColuna < 0 || novaLinha >= LINHAS || novaColuna >= COLUNAS || tabuleiro[novaLinha][novaColuna] != 0) {
            colocarPeca(peca);
            return;
        }
    }
    for (int i = 0; i < 4; i++) {
        peca[i][0] += deltaLinha;
        peca[i][1] += deltaColuna;
    }
    colocarPeca(peca);
}

// Funções para mover a peça para baixo, esquerda e direita
void moveEsquerda(int peca[4][2]) {
    moverPeca(peca, 0, -1);
}

void moveDireita(int peca[4][2]) {
    moverPeca(peca, 0, 1);
}

void moveBaixo(int peca[4][2]) {
    moverPeca(peca, 1, 0);
}

// Função para verificar se a peça atingiu o fundo ou outra peça
int verificaPouso(int peca[4][2]) {
    for (int i = 0; i < 4; i++) {
        if (peca[i][0] >= LINHAS - 1 || tabuleiro[peca[i][0] + 1][peca[i][1]] != 0) {
            return 1;
        }
    }
    return 0;
}

// Função para limpar uma linha completa do tabuleiro
void limparLinha(int linha) {
    for (int r = linha; r > 0; r--) {
        for (int col = 0; col < COLUNAS; col++) {
            tabuleiro[r][col] = tabuleiro[r - 1][col];
        }
    }
    for (int col = 0; col < COLUNAS; col++) {
        tabuleiro[0][col] = 0;
    }
}

// Função para verificar e remover linhas completas
void verificaLinhasCompletas() {
    for (int linha = 0; linha < LINHAS; linha++) {
        int isFull = 1;
        for (int col = 0; col < COLUNAS; col++) {
            if (tabuleiro[linha][col] == 0) {
                isFull = 0;
                break;
            }
        }
        if (isFull) {
            limparLinha(linha);
            score += 100;
        }
    }
}

// Função para renderizar o tabuleiro
void renderizarTabuleiro() {
    limparTerminal();
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (tabuleiro[i][j] == 0) {
                printf(". ");
            } else {
                printf("# ");
            }
        }
        printf("\n");
    }
    printf("Score: %d\n", score);
}

// Função para criar peças aleatórias
void criarPecaAleatoria(int peca[4][2]) {
    int tipo = rand() % 7;
    switch (tipo) {
        case 0: // Linha
            peca[0][0] = 0; peca[0][1] = 4;
            peca[1][0] = 0; peca[1][1] = 5;
            peca[2][0] = 0; peca[2][1] = 6;
            peca[3][0] = 0; peca[3][1] = 7;
            break;
        case 1: // Quadrado
            peca[0][0] = 0; peca[0][1] = 4;
            peca[1][0] = 0; peca[1][1] = 5;
            peca[2][0] = 1; peca[2][1] = 4;
            peca[3][0] = 1; peca[3][1] = 5;
            break;
        case 2: // T
            peca[0][0] = 0; peca[0][1] = 4;
            peca[1][0] = 0; peca[1][1] = 5;
            peca[2][0] = 0; peca[2][1] = 6;
            peca[3][0] = 1; peca[3][1] = 5;
            break;
        case 3: // L
            peca[0][0] = 0; peca[0][1] = 4;
            peca[1][0] = 1; peca[1][1] = 4;
            peca[2][0] = 2; peca[2][1] = 4;
            peca[3][0] = 2; peca[3][1] = 5;
            break;
        case 4: // J
            peca[0][0] = 0; peca[0][1] = 5;
            peca[1][0] = 1; peca[1][1] = 5;
            peca[2][0] = 2; peca[2][1] = 5;
            peca[3][0] = 2; peca[3][1] = 4;
            break;
        case 5: // S
            peca[0][0] = 0; peca[0][1] = 5;
            peca[1][0] = 0; peca[1][1] = 6;
            peca[2][0] = 1; peca[2][1] = 4;
            peca[3][0] = 1; peca[3][1] = 5;
            break;
        case 6: // Z
            peca[0][0] = 0; peca[0][1] = 4;
            peca[1][0] = 0; peca[1][1] = 5;
            peca[2][0] = 1; peca[2][1] = 5;
            peca[3][0] = 1; peca[3][1] = 6;
            break;
    }
}

// Função para inicializar o jogo
void inicializarJogo(int peca[4][2]) {
    srand(time(NULL));  // Inicializa o gerador de números aleatórios
    criarPecaAleatoria(peca);
    colocarPeca(peca);
    renderizarTabuleiro();
}

// Função para processar a entrada do usuário "via teclado"
void processarEntrada(int peca[4][2]) {
    if (_kbhit()) {
        char comando = _getch();
        if (comando == 'a') {
            moveEsquerda(peca);
        } else if (comando == 'd') {
            moveDireita(peca);
        } else if (comando == 'q') {
            exit(0); // Sai do jogo ao pressionar 'q'
        }
    }
}

// Função principal que executa o loop do jogo
int main() {
    int peca[4][2];
    int pecaFixa = 0;
    DWORD lastMoveTime = GetTickCount();

    inicializarJogo(peca);

    while (1) {
        DWORD currentTime = GetTickCount();
        if (currentTime - lastMoveTime >= 1000) {  // Movimento automático a cada 1 segundo
            if (verificaPouso(peca)) {
                colocarPeca(peca);
                verificaLinhasCompletas();
                pecaFixa = 1;
            } else {
                moveBaixo(peca);
                pecaFixa = 0;
            }

            if (pecaFixa) {
                criarPecaAleatoria(peca);
                for (int i = 0; i < 4; i++) {
                    if (tabuleiro[peca[i][0]][peca[i][1]] != 0) {
                        printf("Game Over!\n");
                        return 0;
                    }
                }
                colocarPeca(peca);
                pecaFixa = 0;
            }
            lastMoveTime = currentTime;
        }

        processarEntrada(peca);
        renderizarTabuleiro();
        verificaLinhasCompletas();
    }

    return 0;
}
