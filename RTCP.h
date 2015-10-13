/**
 * @brief Este archivo contiene las cabeceras de las funciones para el modulo RTCP
 * Redes II, Practica II
 * @file RTCP.h
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/
#ifndef _RTCP_H
#define _RTCP_H

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
#include "tipos_serv.h"

/**
 * @brief tamanno maximo para el usuario y el numero del dominio
*/
#define TAM_RTCP 18

/**
 * @brief tamanno maximo que ocupa una estructura RTCP
*/
#define TAM_MEN_RTCP TAM_RTCP+10

/**
 * @brief tipos de mensaje RTCP
*/
typedef enum{sr_t, rr_t, sdes_t, bye_t} Tipo_RTCP;

/**
 * @brief Estructura de un mensaje generico RTCP
*/
typedef struct{
	Tipo_RTCP tipo;/*!<tipo de mensaje RTCP*/
	char relleno[TAM_MEN_RTCP];/*!<relleno*/
}Mensaje_RTCP;

/**
 * @brief Estructura de un mensaje "Sender Report" RTCP
*/
typedef struct{
	Tipo_RTCP tipo;/*!<tipo de mensaje RTCP*/
	int ssrc_sender;/*!<descriptor de la sesion RTP*/
	int p_count;/*!<numero de paquetes enviados por el servidor*/
	int o_count;/*!<numero de paquetes (de 8 en 8)*/
}SR;

/**
 * @brief Estructura de un mensaje "Reciber Report" RTCP
*/
typedef struct{
	Tipo_RTCP tipo;/*!<tipo de mensaje RTCP*/
	int ssrc_sender;/*!<descriptor de la sesion RTP*/
	int ssrc_source;/*!<descriptor de la sesion RTP*/
	char fr_lost;/*!<fraccion de paquetes perdidos*/
	short cum_lost;/*!<numero de paquetes perdidos aunque son 3 bytes, el mas significativo va a ser 0*/
	int extend_sec;/*!<ultimo numero de secuencia recibido*/
	int jitter;/*!<jitter desde el ultimo RR*/
}RR;

/**
 * @brief Estructura de un mensaje "Source DEStination" RTCP
 * al solo haber una fuente, no se usara
*/
typedef struct{
	Tipo_RTCP tipo;/*!<tipo de mensaje RTCP*/
	int ssrc;/*!<descriptor de la sesion RTP*/
	char legth;/*!<tamano del siguiente campo*/
	char name[TAM_RTCP];/*!<nombre del usuario y del dominio*/
}SDES;

/**
 * @brief Estructura de un mensaje "adios" RTCP, para cerrar la sesion
*/
typedef struct{
	Tipo_RTCP tipo;/*!<tipo de mensaje RTCP*/
	int ssrc;/*!<descriptor de la sesion RTP*/
}BYE;

/**
 * @brief Esta funcion crea un socket desde el modulo RTCP
 * @param puerto es el numero de puerto para el socket
 * @return -1 si error, el numero del socket en caso contrario
*/
int crea_socket_RTCP(short puerto);

/**
 * @brief Esta funcion envia un comando RTCP dado a traves de un determinado socket
 * @param sockfd es el numero del socket por el que mandarlo
 * @param dir direcciona donde mandar el mensaje
 * @param tam tamanno de la estructura anterior
 * @param mensaje es el mensaje a mandar por el usuario (debe de haber sido rellenado antes)
 * @return -1 si error, 0 en caso contrario
*/
int envia_comandoRTCP(int sockfd, struct sockaddr * dir, socklen_t tam, Mensaje_RTCP *mensaje);

/**
 * @brief Esta funcion escucha mensajes RTCP de un socket determinado y devuelve una estrucutra con los datos del mensaje
 * @param sockfd es l numero de socket a escucharç
 * @param dir direcciona de donde recibir el mensaje
 * @param tam tamanno de la estructura anterior
 * @param mensaje es puntero a la estrucutra donde se guarda toda la informacion relevante al mensaje
 * @return 0 si Ok, -1 en caso de error
 */
int escucha_comandoRTCP(int sockfd, struct sockaddr * dir, socklen_t *tam, Mensaje_RTCP *mensaje);


#endif
