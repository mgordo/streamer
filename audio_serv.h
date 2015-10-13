/**
 * @brief Este archivo contiene las cabeceras de las funciones para
 * la manipulacion del audio
 * Redes II, Practica II
 * @file audio_serv.h
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/
#ifndef _AUDIO_SERV
#define _AUDIO_SERV

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <resolv.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tipos_serv.h"
#include "RTP.h"
#include "RTCP.h"

/**
 * @brief Nombre del fichero donde se almacenaran las estadisticas de la conexion
*/
#define AUDIO_FICH "audio.log"

/**
 * @brief Puertos para RTP y RTCP del primer setup
*/
extern short client_rtp_port_1,server_rtp_port_1,client_rtcp_port_1,server_rtcp_port_1;

/**
 * @brief ruta para el fichero de audio
*/
extern char ruta_audio[TAM];

/**
 * @brief Semaforos y variables para avisar que hay que detener el 
 * envio de audio y video
*/
extern int estado_audio;
extern pthread_mutex_t sem_audio;

/**
 * @brief semaforo y variable para indicar al hilo principal o bien un teardown,
 * o bien que se ha acabado la reproduccion
 *
*/
extern pthread_mutex_t sem_main;
extern int estado_main;

/**
 * @brief Hilo principal de envio de audio
 *
 * @param arg puntero a una estructura struct sockaddr_in con la direaccion a donde
 * enviar los datos
 * @return NULL
*/
void * main_audio(void *arg);


#endif

