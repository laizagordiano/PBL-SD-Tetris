#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // Para usleep()
#include "video.h"   // Biblioteca de vídeo para VGA
#include "acl345.c"
#include <pthread.h>
#include "globals.h"
#include "KEY.h"

// Dimensões da tela
#define SCREEN_X 320
#define SCREEN_Y 240

#define ROWS 31
#define COLS 14
#define TRUE 1
#define FALSE 0


// Definições de endereços e registradores
char Table[ROWS][COLS] = {0};
int score = 0;
int linesCompleted = 0; // Contador de linhas completas
char GameOn = TRUE;
int fallDelay = 80000; // Tempo de atraso para a peça cair (em microssegundos)
time_t lastUpdate; // Temporizador para controlar a queda da peça

extern int offset_x;
static accelX ; // Variável global para armazenar o valor do eixo X do acelerômetro

// Estrutura para representar uma peça
typedef struct { 
    char **array;
    int width, row, col;
    int color; // Adiciona cor à estrutura da forma
} Shape;
Shape current;

// Array de peças
const Shape ShapesArray[7] = {
    {(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3}, // S_shape     
    {(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3}, // Z_shape     
    {(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3}, // T_shape     
    {(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3}, // L_shape     
    {(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3}, // ML_shape    
    {(char *[]){(char []){1,1},(char []){1,1}}, 2}, // SQ_shape
    {(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} // R_shape
};

/**
 * Função DrawShape para desenhar uma peça
 * Recebe uma peça e desenha na tela
 * Percorre a matriz da peça e desenha um quadrado para cada valor 1
 */
void DrawShape(Shape shape) {
    int blockSize = 7;  // Tamanho do quadrado   
    int leftBorderX = 110; // Alinhamento da borda esquerda
    int topBorderY = 18;   // Alinhamento da borda superior

    for (int i = 0; i < shape.width; i++) {
        for (int j = 0; j < shape.width; j++) {
            if (shape.array[i][j]) {
                int x1 = leftBorderX + (shape.col + j) * (blockSize);
                int y1 = topBorderY + (shape.row + i) * (blockSize);
                int x2 = x1 + blockSize - 1;
                int y2 = y1 + blockSize - 1;
                video_box(x1, y1, x2, y2, shape.color); // Usa a cor da peça
            }
        }
    }
}


// Array de cores para as peças
const int ShapeColors[7] = {
    video_RED,    // S_shape
    video_GREEN,  // Z_shape
    video_CYAN,   // T_shape
    video_ORANGE, // L_shape
    video_BLUE,   // ML_shape
    video_YELLOW, // SQ_shape
    video_CYAN    // R_shape
};

/**
 * Função GetShapeColor para obter a cor de uma peça
 * Recebe o índice da peça e retorna a cor correspondente
 * Se o índice for inválido, retorna a cor padrão
 */
int GetShapeColor(int shapeIndex) {
    if (shapeIndex >= 0 && shapeIndex < 7) {
        return ShapeColors[shapeIndex];
    }
    return video_WHITE; // Cor padrão
}

/**
 * Função CopyShape para copiar uma peça
 * Aloca memória para a nova peça e copia os valores da peça original
 * Retorna a nova peça
 */
Shape CopyShape(Shape shape){
    Shape new_shape = shape;
    new_shape.array = (char**)malloc(new_shape.width * sizeof(char*));
    for(int i = 0; i < new_shape.width; i++){
        new_shape.array[i] = (char*)malloc(new_shape.width * sizeof(char));
        for(int j = 0; j < new_shape.width; j++) {
            new_shape.array[i][j] = shape.array[i][j];
        }
    }
    return new_shape;
}

/**
 * Função DeleteShape para deletar uma peça
 * Percorre a matriz da peça e libera a memória alocada
 */
void DeleteShape(Shape shape) {
    for(int i = 0; i < shape.width; i++) {
        free(shape.array[i]);
    }
    free(shape.array);
}

/**
 * Função CheckPosition para verificar se a posição da peça é válida
 * Percorre a matriz da peça e verifica se a posição é válida
 * Retorna TRUE se a posição for válida e FALSE caso contrário 
 */
int CheckPosition(Shape shape) { 
    char **array = shape.array;
    for(int i = 0; i < shape.width; i++) {
        for(int j = 0; j < shape.width; j++) {
            if(array[i][j]) {
                if(shape.col + j < 0 || shape.col + j >= COLS || shape.row + i >= ROWS) {
                    return FALSE; // Fora dos limites
                }
                if(shape.row + i >= 0 && Table[shape.row + i][shape.col + j]) {
                    return FALSE; // Colisão com outra peça
                }
            }
        }
    }
    return TRUE; // Posição válida
}

/**
 * Função GetNewShape para obter uma nova peça
 * Gera um número aleatório entre 0 e 6 e copia a peça
 * correspondente para a variável global current
 * A função também centraliza a peça na matriz
 * 
 */
void GetNewShape() { 
    int shapeIndex = rand() % 7; // Gera um número aleatório entre 0 e 6
    current = CopyShape(ShapesArray[shapeIndex]);
    current.col = COLS / 2 - (current.width / 2); // Centraliza a peça na matriz
    current.row = 0; // Começa na linha 0
    current.color = GetShapeColor(shapeIndex); 
    if (!CheckPosition(current)) {
        GameOn = FALSE; // Finaliza o jogo
    }
}

/**
 * Função writeTable para escrever a peça atual na matriz
 * Percorre a matriz da peça atual e escreve os valores na matriz
 * principal
 * 
 */
void WriteToTable() {
    for(int i = 0; i < current.width; i++) {
        for(int j = 0; j < current.width; j++) {
            if(current.array[i][j])
                Table[current.row + i][current.col + j] = current.array[i][j];
        }
    }
}

/**
 * Função para verificar se uma linha foi completada
 * Percorre a matriz e verifica se uma linha foi completada
 * Se uma linha foi completada, ela é removida e as linhas acima
 * são movidas para baixo
 * A função também incrementa a pontuação
 * 
 */
void Check_lines() { 
    int count = 0;
    for(int i = 0; i < ROWS; i++) {
        int sum = 0;
        for(int j = 0; j < COLS; j++) {
            sum += Table[i][j];
        }
        if(sum == COLS) {
            count++;
            linesCompleted++;
            for(int k = i; k >= 1; k--)
                for(int l = 0; l < COLS; l++)
                    Table[k][l] = Table[k - 1][l];
            for(int l = 0; l < COLS; l++)
                Table[0][l] = 0;
        }
    }
    score += 100 * count; // Adiciona 100 pontos por linha completa
    if (linesCompleted % 5 == 0) {
        fallDelay = fallDelay > 1000 ? fallDelay - 1000 : fallDelay; // Reduz o delay até um mínimo de 1ms
    }

}

/**
 * Função para imprimir a matriz na tela
 * A função percorre a matriz e desenha os blocos preenchidos
 * na tela, usando a função video_box() da biblioteca de vídeo.
 * A função video_show() é chamada para atualizar a tela
 */
void PrintTable() {
    video_clear(); // Limpa a tela
    int blockSize = 7;  // Tamanho do quadrado
        

    int leftBorderX = 110; // Coord. x da borda lateral esquerda
    int topBorderY = 18;   // Coord. y da borda superior
    // Desenha o tabuleiro
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int x1 = leftBorderX + (j * (blockSize)); // Ajuste para incluir o espaçamento
            int y1 = topBorderY + (i * (blockSize));  // Ajuste para incluir o espaçamento
            int x2 = x1 + blockSize - 1;
            int y2 = y1 + blockSize - 1;

            if (Table[i][j]) {
                video_box(x1, y1, x2, y2, video_WHITE); // Blocos preenchidos
            } 
        }
    }

    // Desenha a peça atual
    DrawShape(current);
    video_box(105, 18, 110, 239, video_BLUE);   // Lado esquerdo
    video_box(106, 234, 212, 239, video_BLUE);  // Centro
    video_box(208, 18, 213, 239, video_BLUE);   // Lado direit
    video_show(); // Atualiza a tela com as mudanças
}

/**
 * Função para verificar se o jogo acabou
 * Percorre a primeira linha da matriz e verifica se há algum bloco preenchido
 * Se houver, o jogo acaba
 */
void screen_gameOver() {
    video_clear();
    video_erase();
    video_text(35, 30, "GAME OVER!");
    video_show();
    usleep(2000000); // Aguardar 2 segundos
}

/**
 * Função para inicializar o vídeo VGA
 * Abre o vídeo, limpa a tela e chama a função de apagar
 * Retorna 1 se o vídeo foi aberto com sucesso, senão 0
 */
void init_video() {
    if (!video_open()) {
        printf("Erro ao abrir o vídeo.\n");
        return 1;
    }
    video_clear();
    video_erase();
}

/**
 * Função para ler o acelerômetro
 * A função lê o valor do eixo X do acelerômetro e armazena na variável global
 * accelX. O valor é lido a cada 100ms
 */
void* ReadAccelerometer(void* arg) {
    while (GameOn) {
        accelX = get_direcao_movimento(); // Atualiza o valor do eixo X
        printf("%f",accelX);
        usleep(100000); // Ajuste o intervalo de leitura conforme necessário
    }
    return NULL;
}


extern void escrever_registro();

/**
 * Função RotateShape para rotacionar uma peça
 * Recebe uma peça e retorna a peça rotacionada 90 graus
 * A função aloca memória para a nova peça e copia os valores
 * da peça original, rotacionando 90 graus
 * Retorna a peça rotacionada
 */
Shape RotateShape(Shape shape) {
    Shape rotated;
    rotated.width = shape.width;
    rotated.array = (char **)malloc(rotated.width * sizeof(char *));
    for (int i = 0; i < rotated.width; i++) {
        rotated.array[i] = (char *)malloc(rotated.width * sizeof(char));
        for (int j = 0; j < rotated.width; j++) {
            rotated.array[i][j] = shape.array[shape.width - j - 1][i]; // Rotação 90 graus
        }
    }
    rotated.row = shape.row; // Mantém a linha atual
    rotated.col = shape.col; // Mantém a coluna atual
    rotated.color = shape.color; // Mantém a cor da peça original
    return rotated;
}

/**
 * Função PauseGame para pausar o jogo
 * Exibe "JOGO EM PAUSE" na tela e aguarda até que o botão 2 seja pressionado novamenter
 */
void PauseGame() {
    int btn;
    video_text(34, 34, "JOGO EM PAUSE"); 
    video_show(); // Atualiza a tela
    // Aguarda até que o botão 2 seja pressionado novamente
    while (1) {
        if (KEY_read(&btn)) {
            if (btn & 0b0010) { // Verifica se o botão 2 está pressionado
                video_erase();
                break; // Sai do loop para retomar o jogo
            }
        }
        usleep(100000); // Pequeno atraso para não sobrecarregar a CPU
    }
}


/**
 * Função principal do jogo Tetris
 * Inicializa o vídeo, abre o dispositivo de botão e configura o acelerômetro
 * Cria uma thread para ler o acelerômetro
 * Gera a primeira peça e entra em um loop principal
 * O loop principal controla a queda da peça, a movimentação horizontal
 * baseada no acelerômetro e a rotação da peça
 * O loop também verifica se o jogo acabou, pausa o jogo e atualiza a pontuação
 * 
 */
int main() {
    init_video(); // Inicializa o vídeo VGA

    if (KEY_open() == 0) {
        printf("Erro ao abrir o dispositivo de botão.\n");
        return -1;
    }
    int btn;
    configura_acelerometro();
    pthread_t accelThread; // Thread do acelerômetro
    pthread_create(&accelThread, NULL, ReadAccelerometer, NULL); // Cria a thread

    srand(time(NULL));

    GetNewShape(); // Gera a primeira forma

    while (GameOn) {
        lastUpdate = time(NULL);

        while (GameOn) {
            Shape temp = CopyShape(current); // Cria uma cópia da peça atual

                if (KEY_read(&btn)) {
                    // Verifica se o botão 0 está pressionado
                    if (btn & 1) { 
                        Shape rotated = RotateShape(current);
                        if (CheckPosition(rotated)) {
                            DeleteShape(current); // Libere a peça atual antes de atualizar
                            current = rotated; // Atualiza a peça atual para a rotacionada
                        } else {
                            DeleteShape(rotated);
                        }
                    }
                    // Verifica se o botão 2 está pressionado
                    if (btn & 2) {
                        PauseGame(); // Chama a função para pausar o jogo
                    }
                } else {
                    printf("Erro ao ler o botão.\n");
                }

            // Movimentação horizontal baseada no acelerômetro
            if (accelX == -1) { // Exemplo de limite negativo para mover à esquerda
                temp.col--; // Tenta mover a peça para a esquerda
                if (CheckPosition(temp)) {
                    current.col--; // Atualiza a posição se não houver colisão
                }
            } else if (accelX ==1) { // Exemplo de limite positivo para mover à direita
                temp.col++; // Tenta mover a peça para a direita
                if (CheckPosition(temp)) {
                    current.col++; // Atualiza a posição se não houver colisão
                }
            }
            char scoreText[20];
            sprintf(scoreText, "Score: %d", score);
            video_text(60, 20, scoreText); 

            char linesText[30];
            sprintf(linesText, "Linhas: %d", linesCompleted);
            video_text(60, 10, linesText);  
            usleep(fallDelay); // Aguarda um tempo para a queda da peça
            if (time(NULL) - lastUpdate >= 1) { // Atualiza a cada segundo
                current.row++;
                if (!CheckPosition(current)) {
                    current.row--;
                    WriteToTable();
                    Check_lines();
                    GetNewShape();
                }
                PrintTable();
        
            }

        }
    }
    
    pthread_cancel(accelThread); // Cancela a thread do acelerômetro
    pthread_join(accelThread, NULL); // Espera a thread terminar
    screen_gameOver();
    KEY_close();
    video_close();
    return 0;

}
