#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

void insert_event(const char *event_data) {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    //Cambiar los datos del password y database
    const char *server = "localhost";
    const char *user = "root";
    const char *password = "3045905330115";
    const char *database = "usuarios";

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
    //Cambiar los datos para insertar y la tabla
    //snprintf(query, sizeof(query), "INSERT INTO user (nombre, edad) VALUES('%s')", event_data);

    //if (mysql_query(conn, query)) {
    //    fprintf(stderr, "%s\n", mysql_error(conn));
    //    mysql_close(conn);
    //    exit(1);
    //}

    mysql_close(conn);
}

int main() {
    char buffer[256];
    char *token;

    

    while (fgets(buffer, sizeof(buffer), stdin)) {
        // Elimina el salto de línea
        buffer[strcspn(buffer, "\n")] = 0;

        printf("***************Salida del systemtap***************\n");
        //printf("%s\n", buffer);
        // Obtener el primer token
        token = strtok(buffer, ",");

        // Continuar obteniendo los tokens restantes
        while (token != NULL) {
            printf("Token: %s\n", token);
            token = strtok(NULL, ",");
            //token es el dato, seria de meterlo a un struct y mandarlo a insert_event para obtener los datos
        }

        printf("**************************\n");

        insert_event(buffer);
    }

    return 0;
}
