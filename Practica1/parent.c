#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Archivos
#define LOG_FILE "syscalls.log"
#define DATA_FILE "practica1.txt"

// Variables globales
volatile sig_atomic_t stop = 0;
int *total_syscalls;
int (*syscall_count)[3]; // open, read, write

void handle_sigint(int sig) {
    stop = 1;
}

void log_syscall(pid_t pid, const char* syscall) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(log, "Proceso %d : %s (%02d-%02d-%04d %02d:%02d:%02d)\n",
                pid, syscall,
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec);
        fclose(log);
    }
}

int main() {
    signal(SIGINT, handle_sigint);

    // Limpia el archivo de registro
    FILE *log = fopen(LOG_FILE, "w");
    if (log) fclose(log);

    // Limpia el archivo de datos
    FILE *data = fopen(DATA_FILE, "w");
    if (data) fclose(data);

    // Crear memoria compartida
    int shm_fd = shm_open("/shm_syscall", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int) + sizeof(int[3]));
    void *ptr = mmap(0, sizeof(int) + sizeof(int[3]), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    total_syscalls = (int *)ptr;
    syscall_count = (int (*)[3])(ptr + sizeof(int));
    *total_syscalls = 0;
    (*syscall_count)[0] = 0;
    (*syscall_count)[1] = 0;
    (*syscall_count)[2] = 0;

    pid_t pids[2];
    
    for (int i = 0; i < 2; ++i) {
        if ((pids[i] = fork()) == 0) {
            
            char id_str[10];
            snprintf(id_str, sizeof(id_str), "%d", i);
            execl("./child", "./child", id_str, NULL);  // Ejecutar el código del hijo
            perror("exec failed");
            exit(1);
        }
    }

     // Ejecuta el script SystemTap desde el proceso padre
            const char *script = "syscalls_monitor.stp";
            int pid1 = pids[0];
            char param1[16];
            snprintf(param1, sizeof(param1), "%d", pid1);
            printf("  pid: %s\n", param1);
        
            int pid2 = pids[1];
            char param2[16];
            snprintf(param2, sizeof(param2), "%d", pid2);
            printf("  pid: %s\n", param2);
        
            char command[256];
            snprintf(command, sizeof(command), "sudo stap %s %s %s", script, param1, param2);
            int result = system(command);
            if (result == -1) {
                perror("Error ejecutando el comando");
                return 1;
            } else {
                printf("Script ejecutado con éxito\n");
            }

    while (!stop) {
        sleep(1);
    }

    // Espera que terminen los hijos
    for (int i = 0; i < 2; ++i) {
        waitpid(pids[i], NULL, 0);
    }

    // Imprimir Control + C
    printf("Número total de llamadas al sistema: %d\n", *total_syscalls);
    printf("Número de llamadas al sistema por tipo:\n");
    printf("  Open: %d\n", (*syscall_count)[0]);
    printf("  Read: %d\n", (*syscall_count)[1]);
    printf("  Write: %d\n", (*syscall_count)[2]);

    // Cerrar memoria compartida
    munmap(ptr, sizeof(int) + sizeof(int[3]));
    shm_unlink("/shm_syscall");

    return 0;
}
