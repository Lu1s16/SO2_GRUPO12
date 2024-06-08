#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h> // Incluye signal.h para sig_atomic_t y SIGINT

#define DATA_FILE "practica1.txt"
#define CARACTERES 9

volatile sig_atomic_t stop = 0;
int *total_syscalls;
int (*syscall_count)[3]; // open, read, write

void handle_sigint(int sig) {
    stop = 1;
}

void log_syscall(pid_t pid, const char* syscall) {
    FILE *log = fopen("syscalls.log", "a");
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

void child_process(int id) {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    srand(time(NULL) ^ (getpid()<<16));
    int fd = open(DATA_FILE, O_RDWR | O_CREAT | O_APPEND, 0666); // Open in append mode
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    log_syscall(getpid(), "open");
    (*total_syscalls)++;
    (*syscall_count)[0]++;

    while (!stop) {
        int action = rand() % 3;
        int delay = 1 + rand() % 3;
        sleep(delay);

        if (action == 0) { // Read
            char buffer[9];
            lseek(fd, 0, SEEK_SET);
            read(fd, buffer, 8);
            buffer[8] = '\0';
            log_syscall(getpid(), "read");
            (*total_syscalls)++;
            (*syscall_count)[1]++;
        } else if (action == 1) { // Write
            char line[CARACTERES];
            for (int i = 0; i < CARACTERES - 1; i++) {
                line[i] = 'A' + rand() % 26;
            }
            line[8] = '\n';
            write(fd, line, CARACTERES);
            log_syscall(getpid(), "write");
            (*total_syscalls)++;
            (*syscall_count)[2]++;
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <id>\n", argv[0]);
        exit(1);
    }
    int id = atoi(argv[1]);

    // Crear memoria compartida
    int shm_fd = shm_open("/shm_syscall", O_RDWR, 0666);
    void *ptr = mmap(0, sizeof(int) + sizeof(int[3]), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    total_syscalls = (int *)ptr;
    syscall_count = (int (*)[3])(ptr + sizeof(int));

    child_process(id);
    return 0;
}
