/**
 * @brief Este archivo contiene las cabeceras de las funciones para
 * la creacion, uso y destruccion de sockets
 * Redes II, Practica II
 * @file socket_funcs.h
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/
#ifndef _SOCKET_FUNCS
#define _SOCKET_FUNCS

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
/**
 * @brief esta funcion crea un socket TCP. Le asigna un numero de puerto concreto
 * y lo pne a escuchar como mucho a max_connections
 * @param puerto el puerto en el que crear el socket
 * @param max_connections el numero maximo de usuarios a la espera de ser aceptados
 * @return el descriptor del socket si bien -1 en caso contrario
*/
int crea_registra_escucha_socket_TCP(short puerto, int max_connections);
/**
 * @brief esta funcion crea un socket TCP y se conecta a un servidor
 * @param puerto puerto del socket al que conectarse
 * @param ip_dest direccion del socket del servidor (en orden de red)
 * @return el descriptor del socket si bien -1 en caso contrario
*/
int crea_conecta_socket_TCP(struct in_addr ip_dest, short puerto);

/**
 * @brief esta funcion crea un socket UDP y devuelve el descriptor
 * @param puerto puerto del socket a crear
 * @return el descriptor del socket si OK -1 en caso contrario
*/

int crea_registra_socket_UDP(short puerto);




#endif

