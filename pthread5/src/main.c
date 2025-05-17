// Copyright 2025 Luis Arias <luisalfredo.arias@ucr.ac.cr> CC-BY 4.0
#include "simul.h"
/**
 * @brief Función principal que llama a la funcion de iniciar la simulacion.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Arreglo quecontiene los argumentos.
 * 
 * @return EXIT_SUCCESS en caso de éxito o en caso contrario EXIT_FAILURE.
 */
int main(int argc, char* argv[]) {
  int simulacion = iniciar_simul(argc, argv);
  return simulacion;
}


