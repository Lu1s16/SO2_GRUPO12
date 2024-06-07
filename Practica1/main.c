#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h> 
#include <time.h>

#define LOG_FILE "syscalls.log"
#define FILE_NAME "practica1.txt"

//contadores
int tot_llamadas = 0;
int read_llamadas = 0;
int write_llamadas = 0;


int main(){
    signal(SIGINT, sigint);
    printf("Bienvenido... Iniciando...\n");

    FILE *log_file = fopen(LOG_FILE, "w");
    fclose(log_file);

     for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            int file_desc = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (file_desc == -1) {
                perror("Open");
                exit(EXIT_FAILURE);
            }

            srand(time(NULL) ^ getpid());

            while (1) {
                int syscall_choice = rand() % 3;
                switch (syscall_choice) {
                    case 0: {
                        char buffer[9];
                        for (int i = 0; i < 8; i++) {
                            buffer[i] = 'A' + rand() % 26; 
                        }
                        buffer[8] = '\n'; 
                        write(file_desc, buffer, 9);
                        printf("Proceso %d: Write (fecha y hora)\n", getpid());
                        write_llamadas++;
                        break;
                    }
                    case 1: {
                        char buffer[9];
                        read(file_desc, buffer, 8);
                        buffer[8] = '\0'; 
                        printf("Proceso %d: Read (fecha y hora)\n", getpid());
                        read_llamadas++;
                        break;
                    }
                    case 2:
                        lseek(file_desc, 0, SEEK_SET);
                        printf("Proceso %d: Open (fecha y hora)\n", getpid());
                        break;
                }

                tot_llamadas++;

                sleep(rand() % 3 + 1);
            }
            close(file_desc);
        }
    }
    while (1) {
        pause();
    }

}
void sigint(int señal) {
    printf("Número total de llamadas al sistema de los procesos hijo: %d\n", tot_llamadas);
    printf("Número de llamadas al sistema de tipo Read: %d\n",read_llamadas );
    printf("Número de llamadas al sistema de tipo Write: %d\n",write_llamadas );
    printf("Señal SIGINT recibida");
    printf("Feliz dia");
    exit(0);
}