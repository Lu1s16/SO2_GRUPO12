#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <string.h>
#include <time.h>
//hilos
#include<pthread.h>
#include<unistd.h>

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
    // Nuevo campo para almacenar el número de cuenta duplicado
    int no_cuenta_duplicada; 
} errorcargausua;

// Estructura para representar una operación bancaria
typedef struct
{
    // 1: Depósito, 2: Retiro, 3: Transferencia
    int tipo_operacion; 
    int cuenta_origen;
    int cuenta_destino;
    double monto;
} Operacion;

// Variables globales para usuarios
Usuario usuarios[maximousua];
errorcargausua errores_carga_usuarios_arr[maxerroresusua];
int totalusua = 0;
int total_errores_carga_usuarios = 0;
int errores_cuenta_duplicada = 0;
int errores_saldo_no_real = 0;
int errores_cuenta_no_entero = 0;

//usuarios procesados por hilo
int usuarios_procesados[3] = {0}; 

pthread_mutex_t usuarios_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t errores_mutex = PTHREAD_MUTEX_INITIALIZER;


// Estructura para las operaciones
struct operaciones {
    int operacion;
    int cuenta1;
    int cuenta2;
    double monto;
};
struct operaciones transacciones[1000];
int operaciones_size = 0;

typedef struct {
    int linea;
    char mensaje[256];
    int cuenta 
} errorcargaproc

errorcargaproc errores_carga_procesos_arr[1000]

//Variables globales


// ************  Funciones usuarios ***************

//Función para escribir un error en el archivo de errores
void escribirError(const char *mensaje, int linea, int no_cuenta_duplicada) {
    
    if (total_errores_carga_usuarios < maxerroresusua) {
        errores_carga_usuarios_arr[total_errores_carga_usuarios].linea = linea;
        errores_carga_usuarios_arr[total_errores_carga_usuarios].no_cuenta_duplicada = no_cuenta_duplicada;
        snprintf(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje, sizeof(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje), "%s", mensaje);
        total_errores_carga_usuarios++;
    }
   
}

//Función para leer y procesar usuarios desde un archivo JSON
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

// Leer el archivo json de usuarios
void read_json_file(char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Open File");
        return;
    }
    // Get the file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*)malloc(filesize + 1);
    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0';
    fclose(file);

    // parse the json data
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        perror("Parse JSON");
        free(buffer);
        return;
    }
    //Validar que sea un json array
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

//Crear una estructura de usuarios para escribir en el archivo salida
char* struct_to_json_user() {
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


void print_json_object(char *json_string) {
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }
    char *formatted_json = cJSON_Print(json);
    if (formatted_json == NULL) {
        fprintf(stderr, "Error printing JSON\n");
        cJSON_Delete(json);
        return;
    }
    printf("%s\n", formatted_json);
    free(formatted_json);
    cJSON_Delete(json);
}

//Escribir reporte errores usuarios 
void write_error_report() {
     // Obtener la fecha y hora actual
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Formatear la fecha y hora para el nombre del archivo
    char filename[256];
    snprintf(filename, sizeof(filename), "carga_%04d_%02d_%02d-%02d_%02d_%02d.log",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Open file");
        return;
    }
    fprintf(file, "Fecha: %04d_%02d_%02d  %02d:%02d:%02d \n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(file, "==========================Carga de Usuarios=================================== \n"); 
    for (int i = 0; i < 3; i++) {
        fprintf(file, "Hilo #%d = %d\n", i + 1, usuarios_procesados[i]);
    }
    int suma_total = 0;
    for (int i = 0; i < 3; i++) {
        suma_total += usuarios_procesados[i];
    }
    fprintf(file, "Total = %d\n", suma_total);
    for (int i = 0; i < total_errores_carga_usuarios; i++) {
        fprintf(file, "Error en la línea %d: %s No. Cuenta: %d\n", errores_carga_usuarios_arr[i].linea, errores_carga_usuarios_arr[i].mensaje, errores_carga_usuarios_arr[i].no_cuenta_duplicada);
    }
    fclose(file);
}

void eliminarElemento(Usuario usuarios[], int *n, int indice) {
    // Mover los elementos una posición a la izquierda
    for (int i = indice; i < *n - 1; i++) {
        usuarios[i] = usuarios[i + 1];
    }
    // Disminuir el tamaño del array
    (*n)--;
}

//Validar errores en usuarios
void *procesarUsuarios(void *args) {
    int hilo_id = *((int *)args);
    int start_index = hilo_id * (totalusua / hilosusua);
    int end_index = (hilo_id + 1) * (totalusua / hilosusua);
    if (hilo_id == hilosusua - 1)
        end_index = totalusua;
    
    for (int i = start_index; i < end_index; i++) {
        Usuario usuario = usuarios[i];

        //Validar los datos
        int error = 0;
        char causa_error[500] = "";

        if (usuario.saldo <= 0) {
            error = 1;
            errores_saldo_no_real++;
            snprintf(causa_error, sizeof(causa_error), "El saldo es negativo; ");
        }

        //validar cuenta repetida
        for (int j = 0; j < totalusua; j++) {
            
            if (i != j && usuarios[j].no_cuenta == usuario.no_cuenta) {
                
                error = 1;
                errores_cuenta_duplicada++;
                snprintf(causa_error, sizeof(causa_error), "La cuenta esta duplicada; ");

                // Eliminar el usuario duplicado
                eliminarElemento(usuarios, &totalusua, j);
                // Ajustar índice para verificar el mismo índice de nuevo
                i--; 
                // Ajustar el final del índice para reflejar el cambio en el tamaño del array
                end_index--; 
                break;
                break;
            }
            
        }
        if (usuario.no_cuenta <= 0) {
            error = 1;
            errores_cuenta_no_entero++;
            snprintf(causa_error, sizeof(causa_error), "Numero de cuenta no positivo; ");

        }

        if (error) {
            pthread_mutex_lock(&lock);
            
            escribirError(causa_error, i + 1, usuario.no_cuenta);
            pthread_mutex_unlock(&lock); 
            
            
        } else {
            pthread_mutex_lock(&lock);                           // Bloquear el mutex antes de modificar datos compartidos
            printf("Procesando usuario %d\n", usuario.no_cuenta); // Simulando procesamiento
            pthread_mutex_unlock(&lock);                         // Desbloquear el mutex
            usuarios_procesados[hilo_id]++;

        }
        
    }
   
    return NULL;
}

//Crear hilos para la carga de usuarios
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

// ********** Funciones operaciones

//Leer json transacciones
void read_json_transaction(char* filename){
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
    if (buffer == NULL) {
        perror("Allocate buffer");
        fclose(file);
        return;
    }
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

    // Iterate through the array
    int n = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {
        if (cJSON_IsObject(item)) {
            cJSON *operacion = cJSON_GetObjectItem(item, "operacion");
            cJSON *cuenta1 = cJSON_GetObjectItem(item, "cuenta1");
            cJSON *cuenta2 = cJSON_GetObjectItem(item, "cuenta2");
            cJSON *monto = cJSON_GetObjectItem(item, "monto");

            if (cJSON_IsNumber(operacion) && cJSON_IsNumber(cuenta1) 
                && cJSON_IsNumber(cuenta2) && cJSON_IsNumber(monto)) {

                transacciones[n].operacion = operacion->valueint;
                transacciones[n].cuenta1 = cuenta1->valueint;
                transacciones[n].cuenta2 = cuenta2->valueint;
                transacciones[n].monto = monto->valuedouble;
                n++;
                
            } else {
                //Escribir error en archivo de reporte transacciones
                fprintf(stderr, "Invalid data in JSON object\n");
            }
        } else {
            fprintf(stderr, "Item in array is not an object\n");
        }
    }
    operaciones_size = n;


    cJSON_Delete(json);
    free(buffer);
}

char* struct_to_json() {
    // Create an empty JSON Array
    cJSON *json_array = cJSON_CreateArray();

    // Iterate through the structs
    for (int i = 0; i < operaciones_size; i++) {
        // Create an empty JSON Object
        cJSON *item = cJSON_CreateObject();

        // Add attributes to the object
        cJSON_AddNumberToObject(item, "operacion", transacciones[i].operacion);
        cJSON_AddNumberToObject(item, "cuenta1", transacciones[i].cuenta1);
        cJSON_AddNumberToObject(item, "cuenta2", transacciones[i].cuenta2);
        cJSON_AddNumberToObject(item, "monto", transacciones[i].monto);

        // Add the object to the Array
        cJSON_AddItemToArray(json_array, item);
    }

    // Format the JSON Array 
    char* formatted = cJSON_Print(json_array);
    if (formatted == NULL) {
        fprintf(stderr, "Error formatting JSON\n");
        cJSON_Delete(json_array); // Clean up to avoid memory leaks
        return NULL;
    }
    cJSON_Delete(json_array); // Clean up to avoid memory leaks
    return formatted;
}



/// Ejecucion de transacciones  ------
void *procesarTransaccion(void *args) {
    int hilo_id = *((int *)args);
    int inicio_index = hilo_id * (operaciones_size / 4);
    int fin_index = (hilo_id == 4 - 1) ? operaciones_size : (hilo_id + 1) * (operaciones_size / 4); 

    int errores_identificados_operacion_no_existe = 0;

    for (int i = inicio_index; i < fin_index; i++) {
        struct operaciones transaccion = transacciones[i];

        //Procesar la transaccion
        pthread_mutex_lock(&lock);
        switch (transaccion.operacion)
        {
            case 1:
                //Deposito
                printf("Operacion deposito\n");
                break;
            case 2:
                //Retiro
                printf("Operacion retiro\n");
                break;
            case 3:
                //Transferencia
                printf("Operacion transferencia\n");
                break;
            default:
                printf("Error: Operacion desconocida en la linea %d\n", i + 1);
                errores_identificados_operacion_no_existe++;
        }
        pthread_mutex_unlock(&lock);
    }
    pthread_exit(NULL);

}

// ------------ Carga de transacciones ----------

void cargarTransacciones() {

    //Ingresar direccion del archivo
    char dir[100];
    printf("Ingrese ruta del archivo: ");
    scanf("%99s", dir);

    char* filename = dir;
    read_json_transaction(filename);

    //Crear hilos
    pthread_t thread_operaciones[4];
    int hilo_operaciones[4];
    for(int i = 0; i < 4; i++){
        hilo_operaciones[i] = i;
        if(pthread_create(&thread_operaciones[i], NULL, procesarTransaccion, &hilo_operaciones[i]) != 0) {
            perror("Error al crear el hilo de transacciones");
            exit(EXIT_FAILURE);
        }
    }

    //Ejecutar los hilos y esperar que terminen
    for (int i = 0; i < 4; i++) {
        if(pthread_join(thread_operaciones[i], NULL) != 0) {
            perror("Error al esperar el hilo de transacciones");
            exit(EXIT_FAILURE);
        }
    }

    //Generar el reporte de transacciones

    

}
// /home/luis/Escritorio/Sopes2/Practica2/transacciones.json
//Escribir archivos de salida
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

void FuncionDepo(){
    int nocuenta;
    double monto;
    int validar;
    printf("=======BIENVENIDO A LA FUNCION DEPOSITO=====\n");
    printf("Ingrese el no. de cuenta: ");
    scanf("%d", &nocuenta);
    printf("Ingrese el monto a depositar: ");
    validar = scanf("%lf", &monto);
    while (validar != 1 || monto <= 0) {
        while (getchar() != '\n'); // Limpiar el buffer de entrada
        printf("Monto inválido. Ingrese un monto válido mayor que 0: ");
        validar = scanf("%lf", &monto);
    }
    for (int i = 0; i < totalusua; i++){
        if (usuarios[i].no_cuenta == nocuenta)
        {
            pthread_mutex_lock(&usuarios_mutex);
            usuarios[i].saldo += monto;
            printf("Se realizo correctamente el deposito de %.2f a la cuenta %d\n", monto, nocuenta);
            pthread_mutex_unlock(&usuarios_mutex);
            return;
        }
    }
    printf("Error: No existe la cuenta %d\n", nocuenta);
}

void FuncionRetiro(){
    int nocuenta;
    double monto;
    int validar;
    printf("=======BIENVENIDO A LA FUNCION RETIRO=====\n");
    printf("Ingrese el no. de cuenta: ");
    scanf("%d", &nocuenta);
    printf("Ingrese el monto a retirar: ");
    validar = scanf("%lf", &monto);
    while (validar != 1 || monto <= 0) {
        while (getchar() != '\n'); // Limpiar el buffer de entrada
        printf("Monto inválido. Ingrese un monto válido mayor que 0: ");
        validar = scanf("%lf", &monto);
    }
    for (int i = 0; i < totalusua; i++){
        if (usuarios[i].no_cuenta == nocuenta)
        {
            pthread_mutex_lock(&usuarios_mutex);
            if (usuarios[i].saldo >= monto)
            {
                usuarios[i].saldo -= monto;
                printf("Se realizo correctamente el retiro de %.2f en la cuenta %d\n", monto, nocuenta);
            }
            else
            {
                printf("Error: No se puede realizar el retiro por saldo insuficiente en la cuenta %d\n", nocuenta);
            }
            pthread_mutex_unlock(&usuarios_mutex);
            return;
        }
    }
    printf("Error: No existe la cuenta %d\n", nocuenta);
}

void FuncionTransa(){
    int nocuentade;
    int nocuentapa;
    double monto;
    int validar;
    printf("=======BIENVENIDO A LA FUNCION TRANSACCION=====\n");
    printf("Ingrese el no. de cuenta a debitar: ");
    scanf("%d", &nocuentade);
    printf("Ingrese el no. de cuenta a depositar: ");
    scanf("%d", &nocuentapa);
    printf("Ingrese el monto a depositar: ");
    validar = scanf("%lf", &monto);
    while (validar != 1 || monto <= 0) {
        while (getchar() != '\n'); // Limpiar el buffer de entrada
        printf("Monto inválido. Ingrese un monto válido mayor que 0: ");
        validar = scanf("%lf", &monto);
    }
    int i, j;
    for (i = 0; i < totalusua; i++)
    {
        if (usuarios[i].no_cuenta == nocuentade)
            break;
    }
    for (j = 0; j < totalusua; j++)
    {
        if (usuarios[j].no_cuenta == nocuentapa)
            break;
    }

    if (i == totalusua)
    {
        printf("Error: No existe la cuenta a debitar %d\n", nocuentade);
        return;
    }
    if (j == totalusua)
    {
        printf("Error: No existe la cuenta a depositar %d\n", nocuentapa);
        return;
    }

    pthread_mutex_lock(&usuarios_mutex);
    if (usuarios[i].saldo >= monto)
    {
        usuarios[i].saldo -= monto;
        usuarios[j].saldo += monto;
        printf("Se realizo correctamente la transaccion de %.2f de la cuenta %d a la cuenta %d\n", monto, nocuentade, nocuentapa);
    }
    else
    {
        printf("Error: No se puede realizar la transaccion porque el saldo es insuficiente en la cuenta %d\n", nocuentade);
    }
    pthread_mutex_unlock(&usuarios_mutex);
}

void FuncionConsul(){
    int nocuenta;
    printf("=======BIENVENIDO A LA FUNCION CONSULTAR CUENTA =====\n");
    printf("Ingrese el no. de cuenta: ");
    scanf("%d", &nocuenta);
    for (int i = 0; i < totalusua; i++)
    {
        if (usuarios[i].no_cuenta == nocuenta)
        {
            printf("======================\n");
            printf("cuenta %d:\n", nocuenta);
            printf("nombre: %s\n", usuarios[i].nombre);
            printf("saldo: %.2f\n", usuarios[i].saldo);
            printf("======================\n");
            return;
        }
    }
    printf("Error: No existe la cuenta %d\n", nocuenta);
}

void Reportes(){
    //Menu Reportes
    int opcion;
    do
    {
        printf("\nMenu Reportes:\n");
        printf("1. Estado de cuentas\n");
        printf("2. Carga usuarios\n");
        printf("3. Carga de operaciones masivas\n");
        printf("4. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch (opcion)
        {
        case 1:
            //Estado de cuentas
            char* users_json = struct_to_json_user();
            write_file("usuarios_salida.json", users_json);
            free(users_json);
            break;
        case 2:
            //Reporte carga de usuarios
            break;
        case 3:
            //Reporte carga de operaciones masivas
            break;
        case 4:
            //Salir menu reportes
            printf("Menu reportes terminado\n");
            break;
        default:
            printf("Opción inválida\n");
        }
    } while (opcion != 4);

}

int main() {

    //Carga de usuarios
    char dir[100];
    printf("Ingrese ruta del archivo de usuarios: ");
    scanf("%99s", dir);

    char* filename = dir;
    read_json_file(filename);
    cargarUsuarios();

   
    write_error_report();

    //Menu principal
    int opcion;
    
    do
    {
        printf("\nMenu Principal:\n");
        printf("1. Deposito\n");
        printf("2. Retiro\n");
        printf("3. Transacción\n");
        printf("4. Consultar cuenta\n");
        printf("5. Carga de operacion\n");
        printf("6. Reportes\n");
        printf("7. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch (opcion)
        {
        case 1:
            //Funcion para deposito
            FuncionDepo();
            break;
        case 2:
            //Funcion para retiro
            FuncionRetiro();
            break;
        case 3:
            //Funcion para transaccion
            FuncionTransa();
            break;
        case 4:
            //Funcion para consultar cuenta
            FuncionConsul();
            break;
        case 5:
            // Cargar operaciones desde archivo
            cargarTransacciones();
            for (int i = 0; i < operaciones_size; i++) {
                printf("Id: %d   Cuenta1: %d   Cuenta2: %d   Monto: %f\n", 
                transacciones[i].operacion, transacciones[i].cuenta1, transacciones[i].cuenta2, transacciones[i].monto);
            }

            char* new_json = struct_to_json();
            if (new_json != NULL) {
                write_file("my_json.json", new_json);
                free(new_json); // Liberar memoria asignada por cJSON_Print
            }
            break;
        case 6:
            //Funcion reporte
            Reportes();
            break;
        case 7:
            printf("Hasta Pronto!!!\n");
            break;
        default:
            printf("Opción inválida\n");
        }
    } while (opcion != 7);
    return 0;
}