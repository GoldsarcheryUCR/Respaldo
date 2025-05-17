// Copyright 2025 Luis Arias <luisalfredo.arias@ucr.ac.cr> CC-BY 4.0
#ifndef SIMUL_H
#define SIMUL_H
#define _POSIX_C_SOURCE 200112L
#include <stdbool.h>
#include <errno.h>
#include <float.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "miscelaneos.h"
/**
 * @brief Estructura de datos que representa una lámina de material para
 * almacenar la información relevante.
 * 
 */
typedef struct {
  /**
   * @brief Nombre del archivo que contiene la lámina
   */
  // plate###-ciclos
  char nombre[BUFFER_MAX];
  /**
   * @brief Número de filas de la matriz
   */
  size_t n_filas;
  /**
   * @brief Número de columnas de la matriz
   */
  size_t n_columnas;
  /**
   * @brief La matriz con el estado actual de lámina
   */
  double** matriz;
  /**
   * @brief La matriz con el estado siguiente
   */
  double** matriz2;
  /**
   * @brief Punto de equilibrio, cuando el cambio de cada una las celdas en
   * una iteración es menor al punto de equilibrio, termina la simulación
   */
  double epsilon;
  /**
   * @brief Dimensiones de las celdas de la lámina
   */
  double distancia;
  /**
   * @brief El tiempo que tarda cada iteración en terminar
   */
  size_t tiempo;
  /**
   * @brief La velocidad de propagación de calor que tiene el material.
   */
  double difusividad;
  /**
   * @brief Número de etapas hasta conseguir equilibrio
   */
  //
  size_t iteraciones;
} lamina_t;
/**
 * @brief Estructura de datos qque representa la memoria compartida entre
 * los hilos creados
 */
typedef struct {
  /**
   * @brief Puntero al registro que representa la lamina
   */
  lamina_t* lamina;
  /**
   * @brief Número de filas sin contar bordes
   */
  size_t filas_internas;
  /**
  * @brief Número de hilos que ejecutaran los cálculos
  */
  size_t hilos_totales;
  double max_temp;
  pthread_barrier_t barrier;  // Añadir barrera a la estructura
  double max_temp_global;     // Máximo global compartido
  double coeficiente;
} mem_comp_t;
/**
 * @brief Estructura de datos qque representa la memoria privada de cada
 * hilo
 */
typedef struct {
  /**
  * @brief Identificador del hilo
  */
  size_t num_hilo;
  /**
  * @brief Puntero al registro de la memoria compartida
  */
  double p_max_temp;
  mem_comp_t* mem_comp;
} mem_priv_t;
/**
 * @brief Estructura que contiene las rutas y archivos para la simulación.
 *
 * Esta estructura almacena el prefijo de ruta proporcionado por el usuario,
 * y los punteros a los archivos.
 */
typedef struct {
  /**
   * @brief Prefijo de ruta donde estan los datos
   */
  char* prefijo_ruta_trabajo;
  /**
   * @brief Puntero al archivo de trabajo
   */
  FILE* archivo_trabajo;
  /**
   * @brief Puntero al archivo de salida
   */
  FILE* archivo_salida;
} archivos_t;
/**
 * @brief Función principal que inicia la simulacion.
 *
 * Esta función se encarga de leer un archivo de trbajo con las láminas, 
 * procesar su contenido y generar un archivo de salida en formato TSV. 
 *
 * El nombre del archivo de trabajo y el prefijo de la ruta se reciben como
 * argumentos de línea de comandos.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Arreglo que contiene los argumentos.
 * 
 * @return EXIT_SUCCESS en caso de éxito o en caso contrario EXIT_FAILURE.
 */
int iniciar_simul(int argc, char* argv[]);
/**
 * @brief Función que analiza los argumentos de linea de comando y abre los
 * archivos de entrada y salida.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Arreglo que contiene los argumentos.
 * @param archivos Puntero al struct de los archivos
 * @return EXIT_SUCCESS en caso de éxito o en caso contrario EXIT_FAILURE.
 */
int preparar_archivos(int argc, char* argv[], archivos_t* archivos);
/**
 * @brief Se encarga de tomar los datos del archivo de trabajo que se necesitaran para la simulación
 * 
 * Lee la información de una lámina desde el archivo de trabajo y la almacena en la estructura lámina.
 * @param lamina Puntero a la lamina que se va a cargar
 * @param job_file Archivo de trabajo con los datos de la lamina
 * @param prefijo_ruta_trabajo Nombre de carpeta donde se encuentran todos los archivos
 * @return true si la lámina se cargó, false si no.
 */

/**
* @brief Función para actualizar la temperatura de la fila 
* de la lamina asignada a un hilo.
* @param mem Puntero a la estructura de memoria privada del hilo
* @return void* Hay que retornar algo, en este caso NULL
**/
void* calculo_matriz(void* mem);
/**
 * @brief Se encarga de tomar los datos del archivo de trabajo que se necesitaran para la simulación
 * 
 * Lee la información de una lámina desde el archivo de trabajo y la almacena en la estructura lámina.
 * @param lamina Puntero a la lamina que se va a cargar
 * @param job_file Archivo de trabajo con los datos de la lamina
 * @param prefijo_ruta_trabajo Nombre de carpeta donde se encuentran todos los archivos
 * @return true si la lámina se cargó, false si no.
 */
bool cargar_lamina(lamina_t* lamina, FILE* archivo_trabajo,
  char* prefijo_ruta_trabajo);
/**
 * @brief Se encarga de aleer los datos de la lamina y guardarlos en el struct
 * 
 * @param lamina Puntero a la lamina que se va a cargar
 * @param job_file Archivo de trabajo con los datos de la lamina
 * @param prefijo_ruta_trabajo Nombre de carpeta donde se encuentran todos los archivos
 * @param ruta_lamina Arreglo para almacenar la ruta de la lamina
 * @return true si se lograron leer los datos, false si no.
 */
bool leer_datos_lamina(lamina_t* lamina, FILE* archivo_trabajo,
  char* prefijo_ruta_trabajo, char* ruta_lamina);

/**
 * @brief Procesa los datos de una lámina
 * 
 * Realiza la simulación de la ditribución de calor siguiendo la fórmula
 * dada, hasta que la lámina este estabilizada
 * 
 * @param lamina struct lamina
 * @return EXIT_SUCCESS o en caso contrario EXIT_FAILURE.
 */
int procesar_lamina(lamina_t* lamina, mem_comp_t* mem_comp);

/**
 * @brief Reporta los resultados de la simulación de la lámina.
 * 
 * Escribe los resultados de la simulación en el archivo de salida .tsv
 * @param archivo_salida Archivo donde se escribirán los resultados.
 * @param prefijo_ruta_trabajo Nombre de carpeta donde se encuentran todos los archivos
 * @param lamina struct lamina
 * @return EXIT_SUCCESS o en caso contrario EXIT_FAILURE.
 */
int reportar_lamina(FILE* archivo_salida, char* prefijo_ruta_trabajo,
  lamina_t* lamina);
/**
* @brief función para crear y gestionar hilos. 
* Le asigna la informacion privada a cada hilo y los manda a ejecutar 
* la rutina calculo_matriz()
* @param mem_comp Puntero a la estructura de memoria compartida entre los hilos.
* @return int error: 0 Success : 1 Failure
*/
int crear_hilos(mem_comp_t* mem_comp, pthread_t** hilos, mem_priv_t** mem_priv);
int liberar_memoria(mem_comp_t* mem_comp, mem_priv_t* mem_priv, pthread_t* hilos);
#endif
