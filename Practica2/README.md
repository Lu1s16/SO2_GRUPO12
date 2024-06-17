**Nombre Estudiante 1:** Luis Enrique Garcia Gutierrez
**Carnet Estudiante 1:** 202010814

**Nombre Estudiante 2:** Nataly Saraí Guzmán Duarte
**Carnet Estudiante 2:** 202001570

# Manual Tecnico 

## Clase read.c

### Parte de carga Usuarios
Primero tenemos las funciones que usamos para leer y procesar los usuarios desde un archivo JSON:
- La función read_json_file lee el archivo JSON especificado que seria el filename y divide el contenido en segmentos para procesarlos en hilos paralelos.
- Cada segmento se envía a la función read_json_chunk para su procesamiento individual.
- La función read_json_chunk utiliza la biblioteca JSON para analizar el segmento de datos JSON.
- Por cada usuario encontrado en el segmento, se crea una estructura de usuario y se llama a la función validarUsuario para verificar la integridad de los datos.
- Si los datos del usuario son válidos, se agrega la estructura de usuario a un vector global de usuarios.
- En caso de encontrar datos de usuario no válidos o duplicados, se llama a la función escribirError para registrar el error en el archivo de errores.

Para la validación y procesamiento de usuarios:

- La función procesarUsuarios crea hilos para procesar segmentos de usuarios y espera a que finalicen.
- La función validarUsuario verifica si los campos obligatorios del usuario osea el nocuenta, el nombre y el usuario están presentes y no vacíos.
- Si se detecta un error de validación, se llama a la función escribirError para registrar el error en el archivo de errores.
- Una vez validados, se agregan los usuarios y ya se pueden usar para depositar, transferir etc.
- La función escribirError recibe un mensaje de error, el número de línea en el archivo JSON donde se encontró el error y el número de cuenta duplicado.
- La función struct_to_json_user convierte una estructura de usuario en un formato JSON.
- La función print_json_object imprime un objeto JSON formateado de manera que se lee en consola que se agrego la cuenta.
- La función write_error_report genera un reporte de errores en un archivo de log, incluyendo la cantidad de usuarios procesados, errores encontrados y detalles de los errores.
- La función eliminarElemento elimina un elemento específico del array de usuarios y ajusta el tamaño del array en consecuencia.

### Funciones depositar, retiro, transaccion y consulta

- Para depositar el usuario ingresa el numero de cuenta a la que desea depositar y luego se introduce el monto que sea depositar, si el numero es mayor a 0 y encuentra la cuenta el deposito se ingresa correctamente.

- Para retiro el usuario ingresa el numero de cuenta a la que desea retirar y luego se introduce el monto que sea retirar, si el numero es mayor a 0, encuentra la cuenta el deposito se ingresa correctamente y verifica que el monto si este disponible en la cuenta entonces realiza el retiro correctamente.

- Para hacer una transaccion el usuario ingresa el numero de cuenta a la que desea depositar, el numero de cuenta que desea transferir y luego se introduce el monto que sea depositar, si el numero es mayor a 0 y encuentra la cuenta el deposito se ingresa correctamente.

- Para realizar la consulta el usuaro ingresa el numero de cuenta y verifica si la cuenta corresponde a las ingresadas en la carga masiva, si es asi muestra los datos.

## Manejo de operaciones

### Funciones para leer y procesar operaciones desde un archivo JSON:

* read_json_transaction(char* filename):
    1. Primero se abre el archivo y se lee todo el contenido en un buffer
    2. Luego se parsea el contenido del json
    3. Se hace un for each del contenido json y guardar los datos en un array llamado transacciones el cual servira para procesar cada operacion.
* struct_to_json(): 
    1. Convierte la estructura de operaciones en un formato JSON.
* is_number(const char *str): 
    1. Esta funcion sirve para validar si el monto es un numero.
    2. En caso de no serlo se marca error.
* procesarTransaccion():
    1. Se utiliza un for para recorrer cada proceso.
    2. Con un Switch se verifica que operacion realizar.
    3. Se realiza la operacion de deposito, retiro y transaccion.
* write_error_report_procesos():
    1. Escribira la fecha en un archivo .log
    2. Luego se escribe la cantidad de procesos que ejecuto cada hilo.
    3. Despues se escribe el total de operaciones de retiros, depositos y transacciones que se ejecutaron.
    4. De ultimo se hace un for que recorre un array de los errores encontrados para escribirlos en el archivo.
