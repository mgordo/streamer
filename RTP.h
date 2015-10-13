/**
 * @brief Este archivo contiene las cabeceras asociadas RTP
 * Redes II, Practica II
 * @file RTP.h
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/
#ifndef _RTP_H
#define _RTP_H

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
 * @brief tamanno maximo del paquete a enviar
*/
#define ENV 2000

/**
 * @brief Esta funcion crea un socket UDP desde el modulo RTP
 * @param puerto es el numero de puerto para el socket
 * @return -1 si error, 0 en caso contrario
*/
int crea_socket_RTP(short puerto);

/**
 * @brief Esta funcion envia un paquete RTP con los datos especificados
 * 
 * @param sockfd descriptor del socket devuelto por "crea_socket_RTP"
 * @param dir direccion donde enviar el paquete
 * @param tam tamanno del argumento anterior
 * @param sec numero de secuencia
 * @param timestamp marca temporal
 * @param ssrc identificador de fuentes de sincronizacion
 * @param buff carga util del mensaje
 * @param tamBuff tamanno de la carga util del mensaje
 *
 * @return el numero de bytes enviados como carga util si todo bien y -1 si algo fallo
*/
int envia_paquete_RTP(int sockfd, struct sockaddr * dir, socklen_t tam, 
					  short sec, int timestamp, int ssrc, char *buff, int tamBuff);

/**
 * @brief Esta funcion recibe un paquete RTP con los datos especificados
 * 
 * @param sockfd descriptor del socket devuelto por "crea_socket_RTP"
 * @param dir direccion de donde se recibe el paquete
 * @param tam tamanno del argumento anterior y vuelve el tamano de la direccion extraida
 * @param sec numero de secuencia
 * @param timestamp marca temporal
 * @param ssrc identificador de fuentes de sincronizacion
 * @param buff buffer donde almacenar la carga util del mensaje
 * @param tamBuff tamanno del buffer proporcionado
 *
 * @return el numero de bytes recibidos como carga util si todo bien y -1 si algo fallo
*/
int recibe_paquete_RTP(int sockfd, struct sockaddr * dir, socklen_t *tam, 
					  short *sec, int *timestamp, int *ssrc, char *buff, int tamBuff);

#endif


