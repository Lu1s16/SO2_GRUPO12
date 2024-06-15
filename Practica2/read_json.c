#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <string.h>
#include <pthread.h>

#define MAX_USERS 100
#define THREAD_COUNT 3

struct data_struct {
    int no_cuenta;
    char nombre[50];
    double saldo;
};

struct data_struct usuarios[MAX_USERS];
int usuarios_size = 0;

struct error_struct {
    int index;
    char error_message[100];
};

struct error_struct errores[MAX_USERS];
int errores_size = 0;

pthread_mutex_t usuarios_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t errores_mutex = PTHREAD_MUTEX_INITIALIZER;

void print_json_object(char *json_string) {
    cJSON *json = cJSON_Parse(json_string);
    char *formatted_json = cJSON_Print(json);
    printf("%s\n", formatted_json);
    cJSON_Delete(json);
    free(formatted_json);
}

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
                if (usuarios_size < MAX_USERS) {
                    usuarios[usuarios_size].no_cuenta = no_cuenta->valueint;
                    snprintf(usuarios[usuarios_size].nombre, sizeof(usuarios[usuarios_size].nombre), "%s", nombre->valuestring);
                    usuarios[usuarios_size].saldo = saldo->valuedouble;
                    usuarios_size++;
                }
                pthread_mutex_unlock(&usuarios_mutex);
            } else {
                pthread_mutex_lock(&errores_mutex);
                snprintf(errores[errores_size].error_message, sizeof(errores[errores_size].error_message), "Invalid entry at index %d", errores_size);
                errores[errores_size].index = errores_size;
                errores_size++;
                pthread_mutex_unlock(&errores_mutex);
            }
        }
    }
    return NULL;
}

void read_json_file(char* filename) {
    // Open the JSON file for reading
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Open File");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the entire file into a buffer
    char *buffer = (char*)malloc(filesize + 1);
    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0';

    // Close the file
    fclose(file);

    // Parse the JSON data
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        perror("Parse JSON");
        free(buffer);
        return;
    }

    // Check if it's a valid JSON array
    if (!cJSON_IsArray(json)) {
        perror("JSON is not Array");
        cJSON_Delete(json);
        free(buffer);
        return;
    }

    // Create threads to process the JSON array
    pthread_t threads[THREAD_COUNT];
    int chunk_size = cJSON_GetArraySize(json) / THREAD_COUNT;
    for (int i = 0; i < THREAD_COUNT; i++) {
        cJSON *json_chunk = cJSON_CreateArray();
        for (int j = i * chunk_size; j < (i + 1) * chunk_size && j < cJSON_GetArraySize(json); j++) {
            cJSON *item = cJSON_GetArrayItem(json, j);
            cJSON_AddItemToArray(json_chunk, cJSON_Duplicate(item, 1));
        }
        pthread_create(&threads[i], NULL, read_json_chunk, json_chunk);
    }

    // Wait for threads to finish
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    cJSON_Delete(json);
    free(buffer);
}

char* struct_to_json() {
    // Create an empty JSON Array
    cJSON *json_array = cJSON_CreateArray();

    // Iterate through the structs
    for (int i = 0; i < usuarios_size; i++) {
        // Create an empty JSON Object
        cJSON *item = cJSON_CreateObject();

        // Add attributes to the object
        cJSON_AddNumberToObject(item, "no_cuenta", usuarios[i].no_cuenta);
        cJSON_AddStringToObject(item, "nombre", usuarios[i].nombre);
        cJSON_AddNumberToObject(item, "saldo", usuarios[i].saldo);

        // Add the object to the Array
        cJSON_AddItemToArray(json_array, item);
    }

    // Format the JSON Array
    char* formatted = cJSON_Print(json_array);
    cJSON_Delete(json_array);  // Clean up the JSON array object
    return formatted;
}

void write_file(char* filename, char* data) {
    // Open the file
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Open file");
        return;
    }

    // Write the data
    fwrite(data, 1, strlen(data), file);
    fclose(file);
}

void write_error_report(char* filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Open file");
        return;
    }

    for (int i = 0; i < errores_size; i++) {
        fprintf(file, "Error at index %d: %s\n", errores[i].index, errores[i].error_message);
    }
    fclose(file);
}

int main() {
    char* filename = "Usuarios.json";
    read_json_file(filename);

    for (int i = 0; i < usuarios_size; i++) {
        printf("No. Cuenta: %d   Nombre: %s   Saldo: %.2f\n", 
            usuarios[i].no_cuenta, usuarios[i].nombre, usuarios[i].saldo);
    }

    snprintf(usuarios[0].nombre, sizeof(usuarios[0].nombre), "Carlos Soto");

    char* new_json = struct_to_json();
    write_file("my_json.json", new_json);
    free(new_json);  // Clean up the allocated memory for the JSON string

    write_error_report("error_report.txt");

    return 0;
}
