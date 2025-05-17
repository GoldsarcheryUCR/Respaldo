// Copyright 2025 Luis Arias <luisalfredo.arias@ucr.ac.cr> CC-BY 4.0
#include "miscelaneos.h"
// Función para intercambiar dos matrices dinámicas
void intercambiar_matriz(double*** matriz1, double*** matriz2) {
  double** temp = *matriz1;
  *matriz1 = *matriz2;
  *matriz2 = temp;
}
// Función para formatear un tiempo en segundos a una cadena con el formato
// YYYY/MM/DD HH:MM:SS
void format_time(const time_t seconds, char* duracion) {
  // Convierte los segundos en una estructura tm
  const struct tm* gmt = gmtime(&seconds);
  // Transforma el tiempo en el formato indicado
  snprintf(duracion, (size_t) BUFFER_MAX,
      "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year - 70, gmt->tm_mon,
      gmt->tm_mday - 1, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
}
// Función para crear una matriz dinámica de tipo double
double** crear_matriz(const size_t n_filas, const size_t n_columnas) {
  // Allocate the array of pointers to the rows.
  double** matrix = (double**) calloc(n_filas, sizeof(double*));
  if (matrix) {
    for (size_t i = 0; i < n_filas; ++i) {
      if ((matrix[i] = (double*) calloc(n_columnas, sizeof(double))) ==
          NULL) {
        // If allocation fails, free whatever had already been allocated.
        liberar_matriz(matrix, n_filas);
        return NULL;
      }
    }
  }
  return matrix;
}
// Función para liberar la memoria de una matriz dinámica
void liberar_matriz(double** matrix, const size_t n_filas) {
  if (matrix) {
    // Free the rows.
    for (size_t i = 0; i < n_filas; ++i) {
      free(matrix[i]);
    }
    // Free the pointers.
    free(matrix);
  }
}
// Función para eliminar la extensión de un archivo
void quitar_extension(const char *archivo, char *nombre_sin_ext) {
  if (archivo == NULL || nombre_sin_ext == NULL) {
    fprintf(stderr, "Error archivo o nombre_sin_ext es NULL");
    return;
  }
  // Buscar el último punto
  char *punto = strrchr(archivo, '.');
  if (punto) {
    // Calcular la posición del punto
    size_t longitud = punto - archivo;
    strncpy(nombre_sin_ext, archivo, longitud);
    // Agregar el terminador de cadena
    nombre_sin_ext[longitud] = '\0';
  }
}
