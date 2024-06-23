#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

void insert_event(int pid, const char *nombre, const char *llamada, int tamanio, const char *fechayhora) {
    MYSQL *conn;

    // Datos del password y database
    const char *server = "localhost";
    const char *user = "root";
    const char *password = "root2000";
    const char *database = "Proyecto";

    conn = mysql_init(NULL);

    if (conn == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }

    char query[512];
    // Cambiar los datos para insertar y la tabla
    snprintf(query, sizeof(query), "INSERT INTO Dashboard (Pid, Nombre, Llamada, Tamanio, FechayHora) VALUES(%d, '%s', '%s', %d, '%s')",
             pid, nombre, llamada, tamanio, fechayhora);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }

    mysql_close(conn);
}

int main() {
    char buffer[256];
    char *token;
    int pid;
    char nombre[100];
    char llamada[100];
    int tamanio;
    char fechayhora[100];
    int inicio;

    while (fgets(buffer, sizeof(buffer), stdin)) {
        // Elimina el salto de línea
        buffer[strcspn(buffer, "\n")] = 0;

        printf("***************Salida del systemtap***************\n");
        printf("Buffer: %s\n", buffer);

        // Obtener el primer token (Pid)
        token = strtok(buffer, ",");
        if (token != NULL) {
            strncpy(llamada, token, sizeof(llamada));
            llamada[sizeof(llamada) - 1] = '\0'; // Asegurarse de que está null-terminated
        }

        // Obtener el segundo token (Nombre)
        token = strtok(NULL, ",");
        if (token != NULL) {
            pid = atoi(token);
        }

        // Obtener el tercer token (Llamada)
        token = strtok(NULL, ",");
        if (token != NULL) {
            strncpy(nombre, token, sizeof(nombre));
            nombre[sizeof(nombre) - 1] = '\0'; // Asegurarse de que está null-terminated
        }

        // Obtener el cuarto token (Tamanio)
        token = strtok(NULL, ",");
        if (token != NULL) {
            tamanio = atoi(token);
        }

        // Obtener el quinto token (FechayHora)
        token = strtok(NULL, ",");
        if (token != NULL) {
            strncpy(fechayhora, token, sizeof(fechayhora));
            fechayhora[sizeof(fechayhora) - 1] = '\0'; // Asegurarse de que está null-terminated
        }

        printf("Pid: %d\n", pid);
        printf("Nombre: %s\n", nombre);
        printf("Llamada: %s\n", llamada);
        printf("Tamanio: %d\n", tamanio);
        printf("FechayHora: %s\n", fechayhora);
        printf("**************************\n");

        insert_event(pid, nombre, llamada, tamanio, fechayhora);
    }

    return 0;
}

