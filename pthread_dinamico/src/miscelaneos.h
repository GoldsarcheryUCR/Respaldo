// Copyright 2025 Luis Arias <luisalfredo.arias@ucr.ac.cr> CC-BY 4.0
#ifndef MISCELANEOS_H
#define MISCELANEOS_H
#define BUFFER_MAX 1024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/**
 * @brief Transforma el tiempo en formato con fecha y hora.
 * 
 * Convierte una cantidad de segundos a un formato de fecha y hora legible.
 * 
 * @param seconds Cantidad de segundos a formatear.
 * @param duracion Caracteres donde se almacenará el tiempo formateado.
 */
void format_time(const time_t seconds, char* duracion);
/**
 * @brief Crea una matriz dinámica
 * 
 * @param n_filas Número de filas de la matriz.
 * @param n_columnas Número de columnas de la matriz.
 * @return Matriz creada o NULL si no se pudo crear.
 */
double** crear_matriz(const size_t n_filas, const size_t n_columnas);
/**
 * @brief Libera la memoria ocupada por una matriz.
 * 
 * @param matrix matriz a liberar.
 * @param n_filas Número de filas de la matriz.
 */
void liberar_matriz(double** matrix, const size_t n_filas);
/**
 * @brief Elimina la extensión de un nombre de archivo y la guarda una cadena.
 * 
 * @param archivo Nombre del archivo con su extensión.
 * @param nombre_sin_ext Cadena donde se almacenará el nombre sin extensión.
 */
void quitar_extension(const char *archivo, char *nombre_sin_ext);
/**
 * @brief Intercambia los punteros de dos matrices bidimensionales.
 * La matriz1 tomará el valor de la matriz2 y viceversa.
 *
 * @param matriz1 Puntero a la matriz1
 * @param matriz2 Puntero a la matriz2
 */
void intercambiar_matriz(double*** matriz1, double*** matriz2);
#endif
