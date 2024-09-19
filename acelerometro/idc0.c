#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h> // Para mmap() e munmap()
#include <unistd.h>

#define I2C_BASE 0xFFC04000 // Endereço base do I2C0
#define I2C_SPAN 0x1000     // Tamanho da memória mapeada para o I2C
#define IC_CON 0x0          // Offset do registrador de controle
#define IC_TAR 0x4          // Offset do registrador do endereço alvo

// Função para abrir /dev/mem
int open_mem() {
    int fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1) {
        perror("ERRO: não foi possível abrir \"/dev/mem\"");
        return -1;
    }
    return fd;
}

int main(void) {
    volatile int *I2c_ptr; // Ponteiro de endereço virtual para I2C0
    int fd = -1; // Usado para abrir /dev/mem
    void *I2C_virtual; // Endereço virtual mapeado para o I2C

    // Abre /dev/mem para dar acesso aos endereços físicos
    fd = open_mem();
    if (fd == -1) {
        return (-1);
    }

    // Mapeia o espaço de memória do I2C0
    I2C_virtual = mmap(NULL, I2C_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, I2C_BASE);
    if (I2C_virtual == MAP_FAILED) {
        perror("ERRO: mmap() falhou");
        close(fd);
        return (-1);
    }

    // Define o ponteiro de endereço virtual para I2C0
    I2c_ptr = (int *)(I2C_virtual);
    // Exemplo: Adicionar 1 ao registrador de controle I2C0 (ic_con)
    I2c_ptr[IC_CON] = I2c_ptr[IC_CON] + 1;

    // Fecha o mapeamento de endereço virtual aberto anteriormente
    if (munmap(I2C_virtual, I2C_SPAN) != 0) {
        printf("ERRO: munmap() falhou...\n");
        close(fd);
        return (-1);
    }

    close(fd); // Fechar o descritor de arquivo após desmapear a memória
    return 0;
}
