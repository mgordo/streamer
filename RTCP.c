/**
 * @brief Este archivo contiene la implementacion de las funciones 
 * para el modulo RTCP
 * Redes II, Practica II
 * @file RTCP.c
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/

#include "RTCP.h"

/*Funciones privadas para enviar mensajes RTCP*/
static int enviaSR(int sockfd,struct sockaddr * dir,socklen_t tam,SR *mensaje);
static int enviaRR(int sockfd,struct sockaddr * dir,socklen_t tam,RR *mensaje);
static int enviaSDES(int sockfd,struct sockaddr * dir,socklen_t tam,SDES *mensaje);
static int enviaBYE(int sockfd,struct sockaddr * dir,socklen_t tam,BYE *mensaje);

/*Funciones privadas para recibir mensajes RTCP*/
static int recibeSR(char *buff,SR *mensaje);
static int recibeRR(char *buff,RR *mensaje);
static int recibeSDES(char *buff,SDES *mensaje);
static int recibeBYE(char *buff,BYE *mensaje);

int crea_socket_RTCP(short puerto)
{
	return crea_registra_socket_UDP(puerto);
}

int envia_comandoRTCP(int sockfd, struct sockaddr * dir, socklen_t tam, Mensaje_RTCP *mensaje)
{

	if(sockfd == -1 || dir == NULL || tam <= 0 || mensaje == NULL) {
		printf("error en el formato RTCP\n");
		return -1;
	}

	if(mensaje->tipo == sr_t)
		return enviaSR(sockfd,dir,tam,(SR *)mensaje);
	else if(mensaje->tipo == rr_t)
		return enviaRR(sockfd,dir,tam,(RR *)mensaje);
	else if(mensaje->tipo == sdes_t)
		return enviaSDES(sockfd,dir,tam,(SDES *)mensaje);
	else if(mensaje->tipo == bye_t)
		return enviaBYE(sockfd,dir,tam,(BYE *)mensaje);
	else
		return -1;
	return -1;
}

static int enviaSR(int sockfd,struct sockaddr * dir,socklen_t tam, SR *mensaje)
{

	char men[16];
	int n;

	/*Creamos el mensaje con la cabecera y la cuenta de paquetes*/
	men[0] = 0x40;
	men[1] = 200;

	/*La longitud del mensaje es 16 siempre*/
	men[2] = 0;
	men[3] = 16;
	
	/*Guardamos los demas campos (en orden de red)*/
	n = htonl(mensaje->ssrc_sender);
	memcpy(men+4, (void *)&n, sizeof(int));

	n = htonl(mensaje->p_count);
	memcpy(men+8, (void *)&n, sizeof(int));

	n = htonl(mensaje->o_count);
	memcpy(men+12, (void *)&n, sizeof(int));

	/*Enviamos el mensaje*/
	if(sendto(sockfd, men, 16, 0,
                      (const struct sockaddr *)dir, tam) == -1)
		return -1;
	return 0;

}

static int enviaRR(int sockfd,struct sockaddr * dir,socklen_t tam,RR *mensaje)
{

	char men[24];
	int n;
	short t;

	/*Creamos el mensaje con la cabecera y la cuenta de paquetes*/
	men[0] = 0x40;
	men[1] = 201;

	/*La longitud del mensaje es 16 siempre*/
	men[2] = 0;
	men[3] = 24;

	/*Guardamos los demas campos (en orden de red)*/
	n = htonl(mensaje->ssrc_sender);
	memcpy(men+4, (void *)&n, sizeof(int));

	n = htonl(mensaje->ssrc_source);
	memcpy(men+8, (void *)&n, sizeof(int));

	men[12] = mensaje->fr_lost;
	men[13] = 0;
	t = htons(mensaje->cum_lost);
	memcpy(men+14, (void *)&t, sizeof(short));

	n = htonl(mensaje->extend_sec);
	memcpy(men+16, (void *)&n, sizeof(int));

	n = htonl(mensaje->jitter);
	memcpy(men+20, (void *)&n, sizeof(int));

	/*Enviamos el mensaje*/
	if(sendto(sockfd, (const void *)men, 24, 0,
                      dir, tam) == -1){
		perror("");
		return -1;
	}
	return 0;
}

static int enviaSDES(int sockfd,struct sockaddr * dir,socklen_t tam,SDES *mensaje)
{

	char men[28];
	int n;
	
	/*Creamos el mensaje con la cabecera y la cuenta de paquetes*/
	men[0] = 0x40;
	men[1] = 202;

	/*La longitud del mensaje es 16 siempre*/
	men[2] = 0;
	men[3] = 28;

	/*Guardamos los demas campos (en orden de red)*/
	n = htonl(mensaje->ssrc);
	memcpy(men+4, (void *)&n, sizeof(int));

	men[8] = 1;
	men[9] = mensaje->legth;
	memccpy(men+10, mensaje->name, strlen(mensaje->name), TAM_RTCP);

	/*Enviamos el mensaje*/
	if(sendto(sockfd, men, 28, 0,
                      (const struct sockaddr *)dir, tam) == -1)
		return -1;
	return 0;

}

static int enviaBYE(int sockfd,struct sockaddr * dir,socklen_t tam,BYE *mensaje)
{

	char men[8];
	int n;
	
	/*Creamos el mensaje con la cabecera y la cuenta de paquetes*/
	men[0] = 0x40;
	men[1] = 203;

	/*La longitud del mensaje es 16 siempre*/
	men[2] = 0;
	men[3] = 8;

	/*Guardamos los demas campos (en orden de red)*/
	n = htonl(mensaje->ssrc);
	memcpy(men+4, (void *)&n, sizeof(int));

	/*Enviamos el mensaje*/
	if(sendto(sockfd, men, 8, 0,
                      (const struct sockaddr *)dir, tam) == -1)
		return -1;
	return 0;

}

int escucha_comandoRTCP(int sockfd, struct sockaddr * dir, socklen_t *tam, Mensaje_RTCP *mensaje)
{

	char buff[100];/*Tamanno maximo de un paquete RTCP*/
	unsigned char tipo;

	if(sockfd == -1 || dir == NULL || tam == NULL || mensaje == NULL){
		return -1;
	}

	if(recvfrom(sockfd, buff, 28, 0 ,
                        dir, tam) == -1)
		return -1;

	tipo = buff[1];

	if(tipo == 200)
		return recibeSR(buff,(SR *)mensaje);
	else if(tipo == 201)
		return recibeRR(buff,(RR *)mensaje);
	else if(tipo == 202)
		return recibeSDES(buff,(SDES *)mensaje);
	else if(tipo == 203)
		return recibeBYE(buff,(BYE *)mensaje);
	else
		return -1;
	
	return -1;
}

static int recibeSR(char *buff,SR *mensaje)
{

	mensaje->tipo = sr_t;
	mensaje->ssrc_sender = ntohl(*((int *)(buff+4)));
	mensaje->p_count = ntohl(*((int *)(buff+8)));
	mensaje->o_count = ntohl(*((int *)(buff+12)));

	return 0;

}

static int recibeRR(char *buff,RR *mensaje)
{

	mensaje->tipo = rr_t;
	mensaje->ssrc_sender = ntohl(*((int *)(buff+4)));
	mensaje->ssrc_source = ntohl(*((int *)(buff+8)));
	mensaje->fr_lost = buff[12];
	mensaje->cum_lost = ntohs(*((short *)(buff+14)));
	mensaje->extend_sec = ntohl(*((int *)(buff+16)));
	mensaje->jitter = ntohl(*((int *)(buff+20)));

	return 0;

}

static int recibeSDES(char *buff,SDES *mensaje)
{

	mensaje->tipo = sdes_t;
	mensaje->ssrc = ntohl(*((int *)(buff+4)));
	mensaje->legth = buff[9];
	strncpy(mensaje->name,buff+10, TAM_RTCP);

	return 0;

}

static int recibeBYE(char *buff,BYE *mensaje)
{

	mensaje->tipo = bye_t;
	mensaje->ssrc = ntohl(*((int *)(buff+4)));

	return 0;

}

