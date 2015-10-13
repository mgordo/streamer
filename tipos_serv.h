/**
 * @brief Este archivo contiene los tipos para el funcionamiento del servidor
 * Redes II, Practica II
 * @file tipos_serv.h
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/
#ifndef _TIPOS_SERV_H
#define _TIPOS_SERV_H

/**
 * @brief Tiempo de espera (en microsegundos) para los hilos RTCP
*/
#define USLEEPRTCP 200000

/**
 * @brief Tamanno estandar para una cadena de caracteres
*/
#define TAM 100


/**
 * @brief tamanno maximo del paquete a enviar audio
*/
#define ENV_AUDIO 1000

/**
 * @brief tamanno maximo del paquete a enviar video
*/
#define ENV_VIDEO 1000


/**
 * @brief tiempo (us) en reproducir "ENV" bytes
 * El flujo multimedia reproduce cierto numero de bytes por segundo. 
 * Nosotros estimamos este numero y lo guardamos en una macro
 * para los timestamps
*/
#define TEMP 4*21848

/**
 * @brief enumerado para indicarle a los hilos principales de audio y video
 * la accion a realizar. "Seguir" es el caso normal, para seguir enviando paquetes
 * En el caso de que llegue un "Parar", los hilos deberan detener su ejecucion 
 * (hay un semaforo para que se bloqueen) y las de los hilos que hallan creado.
 * En el caso de "Terminar", los hilos deberan finalizar la ejecucion
 * de los hilos que hayan creado y liberar los recursos
*/
typedef enum{Seguir=0, Parar=1, Terminar = 2} Estado_Flujos;
/** 
 * @brief enumerado para el estado de la ejecucion del hilo principal del servidor.
 * Dependiendo de esta variable (la que hay definida en los modulos correspondientes),
 * el hilo principal tomara una decision u otra.
*/
typedef enum{Normal=0, Fin_Reproduccion_1=1,Fin_Reproduccion_2=2, Llegada_Teardown=3, Error_Hilos=4} Estado_Main;

/** 
 * @brief enumerado para el estado de la ejecucion del hilo principal del cliente.
 * Dependiendo de esta variable (la que hay definida en los modulos correspondientes),
 * el hilo principal tomara una decision u otra.
*/
typedef enum{No_Conectado=0, Conectado_RTSP=1, Setup_Pulsado=2,  Rep=3, Pausado=4} Estado_Main_Cliente;

#endif

