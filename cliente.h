/**
 * @brief Este archivo contiene la cabecera de las funciones para el cliente
 * Redes II, Practica II
 * @file cliente.h
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/

#ifndef _CLIENTE_H
#define CLIENTE_H

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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gst/gst.h>
#include <glib.h>
#include <sys/time.h>

#include "tipos_serv.h"
#include "RTP.h"
#include "RTCP.h"

/**
 * @brief definimos una tamanno maximo estandar para las cadenas de texto
*/
#define MAX_STRING 100

/**
 * @brief identificadores de los hilos que se encargan de la recepcion de audio y video y escriben al pipe
*/
extern pthread_mutex_t sem_audio_cliente, sem_video_cliente;

/**
 * @brief variable para indicar a los hilos que envian al pipe los flujos
*/
extern Estado_Flujos estado_audio,estado_video;

/**
 * @brief Direccion IP del servidor.
*/
extern struct in_addr ip_serv;/*Se guarda en videoConnect linea 43*/

/**
 * @brief identificadores de los hilos que se encargan de la recepcion de audio y video y escriben al pipe
*/
extern pthread_t id_hilo_audio_rtp,id_hilo_video_rtp;


/**
 * @brief identificadores de los hilos que se encargan de rtcp
*/
extern pthread_t id_hilo_audio_escucha_rtcp,id_hilo_video_escucha_rtcp,id_hilo_audio_envio_rtcp,id_hilo_video_envio_rtcp;


/**
 * @brief identificadores del hilo que se encarga de la reproduccion con gst
*/
extern pthread_t id_hilo_control;

/**
 * @brief array con los ficheros de donde leera el hilo de reproduccion
*/
extern gint fichero_audio[2], fichero_video[2];



/************************** RTCP ************************/
/**
 * @brief puerto RTCP del audio server
*/
extern short audio_rtcp_port_server;

/**
 * @brief puerto RTCP del video server
*/
extern short video_rtcp_port_server;

/**
 * @brief puerto RTCP del video
*/
extern short video_rtcp_port_client;

/**
 * @brief puerto RTCP del audio
*/
extern short audio_rtcp_port_client;

/**
 * @brief descriptor del socket RTCP de audio
*/
extern int audio_rtcp_sockfd;

/**
 * @brief descriptor del socket RTCP de video
*/
extern int video_rtcp_sockfd;


/************************* RTP *************************/

/**
 * @brief puerto RTP del audio
*/
extern short audio_rtp_port;

/**
 * @brief puerto RTP del video
*/
extern short video_rtp_port;

/**
 * @brief variable con la ruta del fichero a reproducir
*/
extern char videofile[MAX_STRING];

/**
 * @brief Esta funcion auxiliar traduce la entrada de videoConnect en diferentes parametros para el programa. Guarda en una variable global la ruta de video
 * @param entrada es el texto a separar
 * @param ip es puntero a la estructura que contendrá la direccion ip a la que conectarse
 * @param puerto contendrá el puerto al que conectarse
 * @return 0 si OK, -1 en caso contrario
*/
int tomaArgumentos(char *entrada, struct in_addr *ip, int *puerto);

/**
 * @brief Esta funcion lanza los hilos para recepcion de RTP en el cliente
 * @return 0 si OK, -1 en caso contrario
*/
int lanza_recepcion_rtp(void);

/**
 * @brief Esta funcion lanza los hilos para recepcion y envio de RTCP en el cliente
 * @return 0 si OK, -1 en caso contrario
*/

int lanza_rtcp(void);

/**
 * @brief funcion de recepcion del audio de un socket RTP y envio a un pipe
 * @return 0 si OK, -1 en caso contrario
*/
void *main_audio_client(void *);

/**
 * @brief funcion de recepcion del video de un socket RTP y envio a un pipe
 * @return 0 si OK, -1 en caso contrario
*/
void *main_video_client(void *);

/**
 * @brief Funcion que se encarga de llamar a gstreamer e iniciar la reproduccion
*/
void *main_control(void *);

#endif

