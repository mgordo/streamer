/**
 * @brief Este archivo contiene las funciones asociadas RTP
 * Redes II, Practica II
 * @file RTP.c
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/
#include "RTP.h"

/**
 * @brief tamanno maximo de un mensaje RTP
*/
#define MAX_RTP ENV+12

int crea_socket_RTP(short puerto)
{
	return crea_registra_socket_UDP(puerto);
}

int envia_paquete_RTP(int sockfd, struct sockaddr * dir, socklen_t tam, 
					  short sec, int timestamp, int ssrc, char *buff, int tamBuff)
{

	char men[MAX_RTP];
	int tam_env;
	short short_net;
	int int_net;

	tam_env=tamBuff+12;/*La carga util mas la cabecera*/

	if(dir == NULL || buff == NULL || tamBuff > MAX_RTP - 12) return -1;

	/*rellenamos el campo de tipo de carga*/
	men[0] = 0x80;
	men[1] = 0x00; /*Aqui va el tipo de carga POR SI SE QUIERE CAMBIAR*/

	/*Copiamos los numeros de secuencia, marca temporal y ssrc*/
	short_net = htons(sec);
	memcpy(men+2, &short_net, sizeof(short));
	int_net = htonl(timestamp);
	memcpy(men+4, &int_net, sizeof(int));
	int_net = htonl(ssrc);
	memcpy(men+8, &int_net, sizeof(int));

	/*Copiamos la carga util*/
	memcpy(men+12, buff, tamBuff);

	/*Enviamos el mensaje y devolvemos el numero de bytes de carga util enviados*/
	return sendto(sockfd, (const void *)men, tam_env, 0, dir, tam)-12;

}

int recibe_paquete_RTP(int sockfd, struct sockaddr * dir, socklen_t *tam, 
					   short *sec, int *timestamp, int *ssrc, char *buff, int tamBuff)
{

	int rec=0;
	char men[MAX_RTP];

	if(!dir || !tam || !sec || !timestamp || !ssrc || !buff || tamBuff <= 0) return -1;

	/*Cogemos el mensaje*/
	rec = recvfrom(sockfd, men, MAX_RTP, 0,
                        dir, tam);

	/*Si hubo algun error, llegaron muy pocos bytes o no caben en el buffer*/
	if(rec <= 12 || rec-12>tamBuff) return -1;

	/*Extraemos los campos de la cabecera*/
	*sec = ntohs(*(short *)(men+2));
	*timestamp = ntohl(*(int *)(men+4));
	*ssrc = ntohl(*(int *)(men+8));

	/*Extraemos la carga util*/
	memcpy(buff, men+12, rec-12);

	return rec-12;
}


