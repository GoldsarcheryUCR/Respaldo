// Copyright 2025 Luis Arias <luisalfredo.arias@ucr.ac.cr> CC-BY 4.0
#include "simul.h"

int iniciar_simul(int argc, char* argv[]) {
  // Crea struct para los archivos de la simulacion
  archivos_t archivos;
  // Prepara los archivos segun los argumentos enviados por el usuario
  if (preparar_archivos(argc, argv, &archivos)!= EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
  // Obtiene el número de hilos que indica el usuario
  size_t hilos_usuario = sysconf(_SC_NPROCESSORS_ONLN);
  if (sscanf(argv[2], "%zu", &hilos_usuario) == 1) {
  } else {
    fprintf(stderr, "Error: invalid thread count\n");
    return EXIT_FAILURE;
  }
  // Crea el struct de la lamina
  lamina_t lamina;

  mem_comp_t mem_comp;
  mem_comp.lamina = &lamina;
  mem_comp.hilos_totales = hilos_usuario;

  int error = 0;
  bool hay_laminas = true;
  // Hasta que se procesen todas las láminas
  while (hay_laminas) {
    hay_laminas = cargar_lamina(&lamina, archivos.archivo_trabajo,
          archivos.prefijo_ruta_trabajo);
    if (hay_laminas) {
        error = procesar_lamina(&lamina, &mem_comp);
        if (error != 0) {
            break;
        }
        error = reportar_lamina(archivos.archivo_salida,
            archivos.prefijo_ruta_trabajo, &lamina);
        if (error != 0) {
            break;
        }
        liberar_matriz(lamina.matriz, lamina.n_filas);
        liberar_matriz(lamina.matriz2, lamina.n_filas);
    }
  }
  // Cierra los archivos abiertos
  fclose(archivos.archivo_trabajo);
  fclose(archivos.archivo_salida);
  return error;
}

int preparar_archivos(int argc, char* argv[], archivos_t* archivos) {
  // Verifica que el número de argumentos sea el adecuado
  if (argc < 4) {
      fprintf(stderr, "Error, argumentos insuficientes\n");
      return EXIT_FAILURE;
  }
  // Obtiene el nombre del trabajo y el prefijo de la ruta
  char* nombre_trabajo = argv[1];
  archivos->prefijo_ruta_trabajo = argv[3];

  // Elimina la extensión del nombre del trabajo
  char nombre_trabajo_no_ext[BUFFER_MAX] = {0};
  quitar_extension(nombre_trabajo, nombre_trabajo_no_ext);
  // Construye la ruta del archivo de trabajo
  char ruta_trabajo[BUFFER_MAX] = {0};
  if (snprintf(ruta_trabajo, sizeof(ruta_trabajo), "%s/%s",
      archivos->prefijo_ruta_trabajo, nombre_trabajo) >=
          (int)sizeof(ruta_trabajo)) {
      fprintf(stderr, "Error: ruta_trabajo truncada\n");
      return EXIT_FAILURE;
  }
  // Construye la ruta del archivo de salida
  char ruta_salida[BUFFER_MAX] = {0};
  if (snprintf(ruta_salida, sizeof(ruta_salida), "%s/%s.tsv",
      archivos->prefijo_ruta_trabajo, nombre_trabajo_no_ext) >=
          (int)sizeof(ruta_salida)) {
      fprintf(stderr, "Error: ruta_salida truncada\n");
      return EXIT_FAILURE;
  }
  // Abre el archivo de trabajo para lectura
  archivos->archivo_trabajo = fopen(ruta_trabajo, "r");
  if (archivos->archivo_trabajo == NULL) {
      fprintf(stderr, "Error al abrir archivo: %s: %s\n",
          ruta_trabajo, strerror(errno));
      return EXIT_FAILURE;
  }
  // Abre el archivo de salida para escritura
  archivos->archivo_salida = fopen(ruta_salida, "w");
  if (archivos->archivo_salida == NULL) {
      fprintf(stderr, "Error al abrir archivo: %s: %s\n",
          ruta_salida, strerror(errno));
      fclose(archivos->archivo_trabajo);
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

bool cargar_lamina(lamina_t* lamina, FILE* archivo_trabajo,
  char* prefijo_ruta_trabajo) {
  char ruta_lamina[BUFFER_MAX];
  if (!leer_datos_lamina(lamina, archivo_trabajo, prefijo_ruta_trabajo,
      ruta_lamina)) {
    return false;
  }
  FILE* archivo_lamina = fopen(ruta_lamina, "rb");
  if (archivo_lamina == NULL) {
    fprintf(stderr, "Error al abrir archivo:%s: %s\n", ruta_lamina,
        strerror(errno));
    fclose(archivo_trabajo);
    return false;
  }
  // Lee el número de filas y columnas de la matriz de la lámina
  if (fread(&lamina->n_filas, sizeof(size_t), 1, archivo_lamina) == 0) {
    fprintf(stderr, "Error al leer el número de filas");
    fclose(archivo_lamina);
    fclose(archivo_trabajo);
  }
  if (fread(&lamina->n_columnas, sizeof(size_t), 1, archivo_lamina) == 0) {
    fprintf(stderr, "Error al leer el número de columnas");
    fclose(archivo_lamina);
    fclose(archivo_trabajo);
  }
  // Crea las matrices para el estado actual y el siguiente
  lamina->matriz = crear_matriz(lamina->n_filas, lamina->n_columnas);
  if (lamina->matriz == NULL) {
    fprintf(stderr, "Error al crear la matriz 1");
    fclose(archivo_lamina);
    fclose(archivo_trabajo);
    return false;
  }
  lamina->matriz2 = crear_matriz(lamina->n_filas, lamina->n_columnas);
  if (lamina->matriz2 == NULL) {
    fprintf(stderr, "Error al crear la matriz 2");
    liberar_matriz(lamina->matriz, lamina->n_filas);
    fclose(archivo_lamina);
    fclose(archivo_trabajo);
    return false;
  }
  // Lee los valores de la matriz
  for (size_t i = 0; i < lamina->n_filas; ++i) {
    if (fread(lamina->matriz[i], sizeof(double), lamina->n_columnas,
        archivo_lamina) == 0) {
      fprintf(stderr, "Error: al leer valores de la matriz");
      liberar_matriz(lamina->matriz, lamina->n_filas);
      liberar_matriz(lamina->matriz2, lamina->n_filas);
      fclose(archivo_lamina);
      fclose(archivo_trabajo);
      return false;
    }
    // Copiar los datos de matriz a matriz2
    memcpy(lamina->matriz2[i], lamina->matriz[i], sizeof(double) *
      lamina->n_columnas);
  }
  // Cierra el archivo de la lámina
  fclose(archivo_lamina);
  return true;
}
bool leer_datos_lamina(lamina_t* lamina, FILE* archivo_trabajo,
  char* prefijo_ruta_trabajo, char* ruta_lamina) {
  // Verifica si el nombre de la lamina existe
  if (fscanf(archivo_trabajo, "%s", lamina->nombre) != 1) {
    fprintf(stderr, "Error al leer el nombre de la lámina.\n");
    return false;
  }
  // Lee los datos de la lámina (tiempo, difusividad, distancia, epsilon)
  if (fscanf(archivo_trabajo, "%zu %lf %lf %lf", &lamina->tiempo,
        &lamina->difusividad, &lamina->distancia, &lamina->epsilon) < 4) {
    liberar_matriz(lamina->matriz, lamina->n_filas);
    fclose(archivo_trabajo);
    fprintf(stderr, "Error al leer datos de la lamina: %s\n",
      strerror(errno));
    return false;
  }
  lamina->iteraciones = 0;
  // Construye la ruta de la lamina
  if (snprintf(ruta_lamina, BUFFER_MAX, "%s/%s",
    prefijo_ruta_trabajo, lamina->nombre) >= (int) BUFFER_MAX) {
    fprintf(stderr, "Error: ruta_lamina truncada\n");
    return false;
  }
  return true;
}
int procesar_lamina(lamina_t* lamina, mem_comp_t* mem_comp) {
  // Suponemos que la lámina no está en equilibrio
  mem_comp->max_temp = lamina->epsilon + 1;;
  
  // Actualizar filas internas e hilos según los datos de la lamina
  mem_comp->filas_internas = lamina->n_filas - 2;
  mem_comp->hilos_totales = mem_comp->hilos_totales < mem_comp->filas_internas
      ? mem_comp->hilos_totales : mem_comp->filas_internas;
  size_t hilos_totales = mem_comp->hilos_totales;

  mem_comp->coeficiente = (lamina->tiempo * lamina->difusividad)
  /(lamina->distancia * lamina->distancia);
 //
  double epsilon = lamina->epsilon;

  if (pthread_barrier_init(&mem_comp->barrier, NULL,
      mem_comp->hilos_totales + 1) != 0) {
    fprintf(stderr, "Error: barrier initialization failed\n");
    //destroy_matrix(lamina->matriz2, lamina->n_filas);
    return EXIT_FAILURE;
  }

  int error = 0;
  // Hasta que se equilibre.
  pthread_t* hilos = NULL;
  mem_priv_t* mem_priv = NULL;

  error = crear_hilos(mem_comp, &hilos, &mem_priv);

  while (mem_comp->max_temp > epsilon) { 
    pthread_barrier_wait(&mem_comp->barrier);
    double global_max = 0.0;
    for (size_t i = 0; i < hilos_totales; ++i) {
        if (mem_priv[i].p_max_temp > global_max) {
            global_max = mem_priv[i].p_max_temp;
        }
    }
    mem_comp->max_temp = global_max;
    // Intercambiar matrices
    double **temp = lamina->matriz;
    lamina->matriz = lamina->matriz2;
    lamina->matriz2 = temp;
    // Resetear máximo para nueva iteración
    lamina->iteraciones++;
    
    // Sincronizar para nueva iteración (segunda barrera)
    pthread_barrier_wait(&mem_comp->barrier);  
  }  
  for (size_t i = 0; i < hilos_totales; ++i) {
    pthread_join(hilos[i], NULL);
  }
  pthread_barrier_destroy(&mem_comp->barrier);

  free(hilos);
  free(mem_priv);
  return error;
}

void* calculo_matriz(void* mem) {
  // punteros a structs de memoria privada y comparida, y a la lamina
  mem_priv_t* mem_priv = (mem_priv_t*) mem;
  mem_comp_t* mem_comp = mem_priv->mem_comp;
  lamina_t* lamina = mem_comp->lamina;
  double coeficiente = mem_comp->coeficiente;
  double epsilon= lamina->epsilon;
  size_t n_columnas = lamina->n_columnas;
  double p_max_temp = 0.0;

  // Calcular rango de filas para este hilo
  size_t fila_inicio = ((mem_priv->num_hilo * mem_comp->filas_internas) / mem_comp->hilos_totales) + 1;
  size_t fila_fin = (((mem_priv->num_hilo  + 1) * mem_comp->filas_internas) / mem_comp->hilos_totales) + 1;

  while (mem_comp->max_temp > epsilon) { 
    p_max_temp = 0.0;
    for (size_t i = fila_inicio; i < fila_fin; ++i) {
        for (size_t j = 1; j < n_columnas - 1; ++j) {
          // calcular el nuevo valor
            lamina->matriz2[i][j] =  lamina->matriz[i][j] + (coeficiente
              *(lamina->matriz[i - 1][j]+ lamina->matriz[i][j + 1]
              + lamina->matriz[i + 1][j]+ lamina->matriz[i][j - 1]
              - (4 * lamina->matriz[i][j])));
            // Tomar la mayor temperatura del hilo
            double cambio_temp = fabs(lamina->matriz[i][j]-lamina->matriz2[i][j]);
            if (cambio_temp > p_max_temp) {
              p_max_temp = cambio_temp;
            }
        }
    }
    
    // Almacenar temperatura en memoria privada
    mem_priv->p_max_temp = p_max_temp;
    // Esperar a que terminen todos los hilos
    pthread_barrier_wait(&mem_comp->barrier);
    // Esperar por el hilo main
    pthread_barrier_wait(&mem_comp->barrier);
  }
  return NULL;
}


int reportar_lamina(FILE* archivo_salida, char* prefijo_ruta_trabajo,
    lamina_t* lamina) {
  // Guarda el tiempo de la simulación
  char duracion[BUFFER_MAX] = {0};
  format_time(lamina->tiempo * lamina->iteraciones, duracion);

  // Escribe los datos de la lámina en el archivo de salida
  fprintf(archivo_salida, "%s\t%zu\t%lg\t%lg\t%lg\t%zu\t%s\n",
      lamina->nombre, lamina->tiempo, lamina->difusividad,
      lamina->distancia, lamina->epsilon, lamina->iteraciones, duracion);
  fflush(archivo_salida);
  // nombre del archivo sin extensión
  char nombre_reporte_lamina[BUFFER_MAX] = {0};
  quitar_extension(lamina->nombre, nombre_reporte_lamina);

  // Genera el nombre del archivo binario con el número de iteraciones
  char r_lamina_bin[BUFFER_MAX] = {0};
  if (snprintf(r_lamina_bin, BUFFER_MAX, "%s-%zu.bin", nombre_reporte_lamina,
      lamina->iteraciones) >= (int) sizeof(r_lamina_bin)) {
    fprintf(stderr, "Error: r_lamina_bin truncada\n");
    return false;
  }

  // Construye la ruta completa del archivo binario de la lámina
  char ruta_r_lamina[BUFFER_MAX] = {0};
  if (snprintf(ruta_r_lamina, (size_t) BUFFER_MAX, "%s/%s",
      prefijo_ruta_trabajo, r_lamina_bin) >= (int) sizeof(ruta_r_lamina)) {
    fprintf(stderr, "Error: ruta_r_lamina truncada\n");
    return false;
    }

  // Abre el archivo binario para escritura
  FILE* archivo_bin_lamina = fopen(ruta_r_lamina, "wb");
  if (archivo_bin_lamina == NULL) {
    fprintf(stderr, "Error al abrir archivo:%s: %s\n",
      ruta_r_lamina, strerror(errno));
    return EXIT_FAILURE;
  }
  // Escribe el número de filas y columnas en el archivo binario
  fwrite(&lamina->n_filas, sizeof(size_t), 1, archivo_bin_lamina);
  fwrite(&lamina->n_columnas, sizeof(size_t), 1, archivo_bin_lamina);
  // Escribe los valores de la matriz en el archivo binario
  for (size_t i = 0; i < lamina->n_filas; ++i) {
    for (size_t j = 0; j < lamina->n_columnas; ++j) {
      fwrite(&lamina->matriz[i][j], sizeof(double), 1, archivo_bin_lamina);
    }
  }

  // Cierra el archivo binario
  fclose(archivo_bin_lamina);

  return EXIT_SUCCESS;
}
int crear_hilos(mem_comp_t* mem_comp, pthread_t** hilos, mem_priv_t** mem_priv) {
  int error = EXIT_SUCCESS;

  // Reservar memoria para estructuras de los hilos
  *hilos = (pthread_t*)malloc(mem_comp->hilos_totales * sizeof(pthread_t));
  // Memoria privada de cada hilo
  *mem_priv = (mem_priv_t*)calloc(mem_comp->hilos_totales, sizeof(mem_priv_t));

  if (*hilos && *mem_priv) {
    // Inicializar atributos privados
    for (size_t num_hilo = 0; num_hilo < mem_comp->hilos_totales; ++num_hilo) {
      (*mem_priv)[num_hilo].num_hilo = num_hilo;
      (*mem_priv)[num_hilo].mem_comp = mem_comp;
      // Crea los hilos que ejecutarán la rutina del cambio de temperatura
      error = pthread_create(&(*hilos)[num_hilo], NULL, calculo_matriz, &(*mem_priv)[num_hilo]);
      if (error != EXIT_SUCCESS) {
          return error; // Si ocurre un error en la creación de hilos, retorna el error
      }
    }
  } else {
    fprintf(stderr, "Error: could not allocate %zu hilos\n"
      , mem_comp->hilos_totales);
    error = 1;
  }
  return error;
}
int liberar_memoria(mem_comp_t* mem_comp, mem_priv_t* mem_priv, pthread_t* hilos){
  // Esperar a que todos los hilos terminen su ejecución
  for (size_t num_hilo = 0; num_hilo < mem_comp->hilos_totales
      ; ++num_hilo) {
    pthread_join(hilos[num_hilo], /*value_ptr*/ NULL);
  }
  // Liberar memoria reservada
  free(mem_priv);
  free(hilos);
  return EXIT_SUCCESS;
}
