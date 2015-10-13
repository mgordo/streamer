/**
 * @brief Este archivo contiene las cabeceras de las funciones para el modulo RTSP
 * Redes II, Practica II
 * @file RTSP.h
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/
#ifndef _RTSP_H
#define _RTSP_H

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

#include "socket_funcs.h"

/**
 * @brief tamano de relleno de un mensaje generico RTSP
*/
#define TAMANO_RESTO 1200
/**
 * @brief tamanno de una cadena RTSP
*/
#define TAM_CADENA 100
/**
 * @brief tipo de mensaje RTSP
*/
typedef enum {
	Describe=1,
	Setup,
	Play,
	Pause,
	Res_Describe,
	Res_Setup,
	Res_Pause_Play,
	Error,
	Info,
	Teardown
} Tipos_Rtsp;
/**
 * @brief Estructura generica de un mensaje RTSP
*/
typedef struct{
	Tipos_Rtsp tipo; /*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char resto[TAMANO_RESTO]; /*!<resto del mensaje RTSP*/
}Mensaje_RTSP;

/**
 * @brief Estructura de un teardown
*/
typedef struct{
	Tipos_Rtsp tipo; /*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char ruta[TAM_CADENA]; /*!<ruta del mensaje RTSP*/
	int session;/*!<numero de session*/
}Men_Teardown;

/**
 * @brief Estructura de un describe
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char ruta[TAM_CADENA];/*!<ruta donde buscar los ficheros*/
}Men_Describe;

/**
 * @brief Estructura de un error RTSP
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char mensaje[TAM_CADENA];/*!<mensaje de error*/
}Men_Error;

/**
 * @brief Estructura de una respuesta de ddescribe RTSP
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char mensaje[TAM_CADENA];/*!<codigo de informacion*/
	char content_type[TAM_CADENA];/*!<contenido*/
	char m1[TAM_CADENA];/*!<contenido del primer fichero*/
	char a1[TAM_CADENA];/*!<fichero*/
	char m2[TAM_CADENA];/*!<contenido del segundo fichero*/
	char a2[TAM_CADENA];/*!<fichero*/
}Resp_Describe;

/**
 * @brief Estructura de un setup
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char ruta[TAM_CADENA];/*!<ruta*/
	short rtp_port;/*!<numero de puerto RTP*/
	short rtcp_port;/*!<numero de puerto RTCP*/
	int session;/*!<numero de sesion*/
}Men_Setup;

/**
 * @brief Estructura de respuesta de un setup
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char mensaje[TAM_CADENA];/*!<codigo de informacion*/
	short client_rtp_port;/*!<numero de puerto RTP del cliente*/
	short client_rtcp_port;/*!<numero de puerto RTCP del cliente*/
	short server_rtp_port;/*!<numero de puerto RTP del servidor*/
	short server_rtcp_port;/*!<numero de puerto RTCP del servidor*/
	int session;/*!<numero de sesion*/
}Resp_Setup;

/**
 * @brief Estructura de un play
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char ruta[TAM_CADENA];/*!<ruta*/
	int session;/*!<sesion*/
}Men_Play;

/**
 * @brief Estructura de un pause
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char ruta[TAM_CADENA];/*!<ruta*/
	int session;/*!<sesion*/
}Men_Pause;
/**
 * @brief Estructura de informacion
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char mensaje[TAM_CADENA];/*!<codigo de informacion*/
	char relleno[TAM_CADENA];/*!<resto de la informacion*/
}Resp_Info;

/**
 * @brief Estructura de respuesta a Play
*/
typedef struct{
	Tipos_Rtsp tipo;/*!<tipo del mensaje*/
	int sec;/*!<numero de secuencia del mensaje*/
	char mensaje[TAM_CADENA];/*!<codigo de informacion*/
	int session;/*!<numero de sesion*/
	char relleno[TAM_CADENA];/*!<resto de la informacion*/
}Resp_Play;



/**
 * @brief Esta funcion escucha mensajes RTSP de un socket determinado y devuelve una estrucutra con los datos del mensaje
 * @param sockfd es l numero de socket a escuchar
 * @param mensaje es puntero a la estrucutra donde se guarda toda la informacion relevante al mensaje
 * @return 0 si Ok, -1 en caso de error y -2 si se cerro la conexion
 */
int escucha_comandoRTSP(int sockfd, Mensaje_RTSP *mensaje);

/**
 * @brief Esta funcion envia un comando RTSP dado a traves de un determinado socket
 * @param sockfd es el numero del socket por el que mandarlo
 * @param mensaje es el mensaje a mandar por el usuario (debe de haber sido rellenado antes)
 * @return -1 si error, 0 en caso contrario
*/
int envia_comandoRTSP(int sockfd, Mensaje_RTSP *mensaje);

/**
 * @brief Esta funcion crea un socket TCP desde el modulo RTSP
 * @param puerto es el numero de puerto para el socket
 * @param listeners es el numero de escuchantes que se aceptan en el socket a crear
 * @return -1 si error, 0 en caso contrario
*/
int crea_socket_RTSP(int puerto,int listeners);

/**
 * @brief Esta funcion acepta conexiones a un socket TCP desde el modulo RTSP
 * @param sockfd es el descriptor de socket del que aceptar conexiones
 * @param sockdir es la estructura en la que se guardará el usuario conectado
 * @param tam es el tamanyo de la estrucutra sockdir
 * @return -1 si error, el descriptor de socket destinado a la conexion establecida en otro caso
*/
int acepta_conexiones_RTSP(int sockfd,struct sockaddr *sockdir, socklen_t *tam);

/**
 * @brief Esta funcion cierra un socket TCP asociado al modulo RTSP
 * @param sockfd es el descriptor del socket a cerrar
 * @return -1 si error, 0 en caso contrario
*/
int cierra_conexion_RTSP(int sockfd);

/**
 * @brief Esta funcion se conecta a un socket TCP remoto
 * @param sockfd es el descriptor de socket propio
 * @param sockdir es la estructura a la que te quieres conectar
 * @param tam es el tamanyo de la estrucutra sockdir
 * @return -1 si error, el descriptor de socket destinado a la conexion establecida en otro caso
*/
int conecta_socket_RTSP(int sockfd,struct sockaddr *sockdir, socklen_t *tam);



#endif
