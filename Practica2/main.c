#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "cJSON.h"
#include <unistd.h>

pthread_mutex_t lock;

#define maximousua 1000
#define hilosusua 3
#define maxerroresusua 1000

// Estructura para representar un usuario
typedef struct {
    int no_cuenta;
    char nombre[50];
    double saldo;
} Usuario;

// Estructura para representar errores en la carga de usuarios
typedef struct {
    int linea;
    char mensaje[256];
    int no_cuenta_duplicada; // Nuevo campo para almacenar el número de cuenta duplicado
} errorcargausua;

// Variables globales
Usuario usuarios[maximousua];
errorcargausua errores_carga_usuarios_arr[maxerroresusua];
int totalusua = 0;
int total_errores_carga_usuarios = 0;
int errores_cuenta_duplicada = 0;
int errores_saldo_no_real = 0;

pthread_mutex_t usuarios_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t errores_mutex = PTHREAD_MUTEX_INITIALIZER;

// Función para escribir un error en el archivo de errores
void escribirError(const char *mensaje, int linea, int no_cuenta_duplicada) {
    pthread_mutex_lock(&errores_mutex);
    if (total_errores_carga_usuarios < maxerroresusua) {
        errores_carga_usuarios_arr[total_errores_carga_usuarios].linea = linea;
        errores_carga_usuarios_arr[total_errores_carga_usuarios].no_cuenta_duplicada = no_cuenta_duplicada;
        snprintf(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje, sizeof(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje), "%s", mensaje);
        total_errores_carga_usuarios++;
    }
    pthread_mutex_unlock(&errores_mutex);
}

// Función para leer y procesar usuarios desde un archivo JSON
void* read_json_chunk(void* arg) {
    cJSON *json_chunk = (cJSON*)arg;
    cJSON *item = NULL;

    cJSON_ArrayForEach(item, json_chunk) {
        if(cJSON_IsObject(item)){
            cJSON *no_cuenta = cJSON_GetObjectItem(item, "no_cuenta");
            cJSON *nombre = cJSON_GetObjectItem(item, "nombre");
            cJSON *saldo = cJSON_GetObjectItem(item, "saldo");

            if(cJSON_IsNumber(no_cuenta) && cJSON_IsString(nombre) && cJSON_IsNumber(saldo)) {
                pthread_mutex_lock(&usuarios_mutex);
                if (totalusua < maximousua) {
                    usuarios[totalusua].no_cuenta = no_cuenta->valueint;
                    snprintf(usuarios[totalusua].nombre, sizeof(usuarios[totalusua].nombre), "%s", nombre->valuestring);
                    usuarios[totalusua].saldo = saldo->valuedouble;
                    totalusua++;
                }
                pthread_mutex_unlock(&usuarios_mutex);
            } else {
                pthread_mutex_lock(&errores_mutex);
                snprintf(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje, sizeof(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje), "Entrada inválida en el índice %d", total_errores_carga_usuarios);
                errores_carga_usuarios_arr[total_errores_carga_usuarios].linea = total_errores_carga_usuarios;
                total_errores_carga_usuarios++;
                pthread_mutex_unlock(&errores_mutex);
            }
        }
    }
    return NULL;
}

void read_json_file(char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Open File");
        return;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*)malloc(filesize + 1);
    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        perror("Parse JSON");
        free(buffer);
        return;
    }

    if (!cJSON_IsArray(json)) {
        perror("JSON is not Array");
        cJSON_Delete(json);
        free(buffer);
        return;
    }

    pthread_t threads[hilosusua];
    int chunk_size = cJSON_GetArraySize(json) / hilosusua;
    for (int i = 0; i < hilosusua; i++) {
        cJSON *json_chunk = cJSON_CreateArray();
        for (int j = i * chunk_size; j < (i + 1) * chunk_size && j < cJSON_GetArraySize(json); j++) {
            cJSON *item = cJSON_GetArrayItem(json, j);
            cJSON_AddItemToArray(json_chunk, cJSON_Duplicate(item, 1));
        }
        pthread_create(&threads[i], NULL, read_json_chunk, json_chunk);
    }

    for (int i = 0; i < hilosusua; i++) {
        pthread_join(threads[i], NULL);
    }

    cJSON_Delete(json);
    free(buffer);
}

char* struct_to_json() {
    cJSON *json_array = cJSON_CreateArray();

    for (int i = 0; i < totalusua; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "no_cuenta", usuarios[i].no_cuenta);
        cJSON_AddStringToObject(item, "nombre", usuarios[i].nombre);
        cJSON_AddNumberToObject(item, "saldo", usuarios[i].saldo);
        cJSON_AddItemToArray(json_array, item);
    }

    char* formatted = cJSON_Print(json_array);
    cJSON_Delete(json_array);
    return formatted;
}

void write_file(char* filename, char* data) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Open file");
        return;
    }

    fwrite(data, 1, strlen(data), file);
    fclose(file);
}

void write_error_report(char* filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Open file");
        return;
    }

    for (int i = 0; i < total_errores_carga_usuarios; i++) {
        fprintf(file, "Error en la línea %d: %s No. Cuenta: %d\n", errores_carga_usuarios_arr[i].linea, errores_carga_usuarios_arr[i].mensaje, errores_carga_usuarios_arr[i].no_cuenta_duplicada);
    }
    fclose(file);
}

void *procesarUsuarios(void *args) {
    int hilo_id = *((int *)args);
    int start_index = hilo_id * (totalusua / hilosusua);
    int end_index = (hilo_id + 1) * (totalusua / hilosusua);
    if (hilo_id == hilosusua - 1)
        end_index = totalusua;
    
    for (int i = start_index; i < end_index; i++) {
        Usuario usuario = usuarios[i];

        int error = 0;
        char causa_error[256] = "";

        if (usuario.saldo <= 0) {
            error = 1;
            errores_saldo_no_real++;
            strcat(causa_error, "El saldo es negativo; ");
        }

        for (int j = 0; j < totalusua; j++) {
            pthread_mutex_lock(&lock);
            if (i != j && usuarios[j].no_cuenta == usuario.no_cuenta) {
                error = 1;
                errores_cuenta_duplicada++;
                snprintf(causa_error, sizeof(causa_error), "La cuenta esta duplicada; ");
                escribirError(causa_error, i + 1, usuario.no_cuenta);
            }
            pthread_mutex_unlock(&lock);
        }

        if (error) {
            escribirError(causa_error, i + 1, usuario.no_cuenta);
        }
        
    }
   
    pthread_exit(NULL);
}

void cargarUsuarios() {
    pthread_t threads[hilosusua];
    int thread_ids[hilosusua];

    for (int i = 0; i < hilosusua; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, procesarUsuarios, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < hilosusua; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    char archivo_usuarios[] = "Usuarios.json";

    read_json_file(archivo_usuarios);
    cargarUsuarios();

    char* users_json = struct_to_json();
    write_file("usuarios_salida.json", users_json);
    free(users_json);

    
    write_error_report("errores.log");
    
    return 0;
}
