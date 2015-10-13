/**
 * @brief Include necesario para el uso de las funciones públicas definidas para la práctica
 * Redes II, Practica II
 * @file video_funcs.h
 * @author Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/

#ifndef _video
#define _video

#ifndef FALSE
/**
 * @brief definimos la constante false
*/
	#define FALSE	0
#endif
#ifndef TRUE
/**
 * @brief definimos la constante true
*/
	#define TRUE	1
#endif

#include <stdio.h>
#include "RTSP.h"
#include "cliente.h"

/**
 * @brief definimos una tamanno maximo estandar para las cadenas de texto
*/
#define MAX_STRING 100

/**
 * @brief fichero a reproducir
*/
char videofile[MAX_STRING];
/**
 * @brief descriptor del socket RTSP del cliente
*/
int sockfd_client;
/**
 * @brief numero de secuencia actual, para RTSP
*/
int secuencia;
/**
 * @brief fichero de audio a reproducir
*/
char audio[MAX_STRING];
/**
 * @brief fichero de video a reproducir
*/
char video[MAX_STRING];
/**
 * @brief variable para saber el estado de la conexion del cliente.
 * 0 desconectado y 1 conectado
*/
int conectado;


/**
 * @brief funcion escuchante del boton connect
 * @param url la direccion, puerto y ficherocon que conectarse
 * @return TRUE si todo bien y FALSE en caso de error
 * 
*/
int videoConnect(const char *url);

/**
 * @brief funcion escuchante del boton play
 * @return TRUE si todo bien y FALSE en caso de error
 * 
*/
int videoPlay();
/**
 * @brief funcion escuchante del boton pause
 * @return TRUE si todo bien y FALSE en caso de error
 * 
*/
int videoPause();
/**
 * @brief funcion escuchante del boton setup
 * @return TRUE si todo bien y FALSE en caso de error
 * 
*/
int videoSetup();
/**
 * @brief funcion escuchante del boton teardown
 * @return TRUE si todo bien y FALSE en caso de error
 * 
*/
int videoTeardown();

#endif
