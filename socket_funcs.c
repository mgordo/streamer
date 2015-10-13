/**
 * @brief Este archivo contiene las funciones para
 * la creacion, uso y destruccion de sockets
 * Redes II, Practica II
 * @file socket_funcs.c
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/

#include "socket_funcs.h"

int crea_registra_escucha_socket_TCP(short puerto, int max_connections)
{

    int sockfd = -1;
    struct sockaddr_in Direccion;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd == -1) return -1;

    Direccion.sin_family=AF_INET;   /* TCP/IP family */
    Direccion.sin_port=htons(puerto); /* Asigning port */
    Direccion.sin_addr.s_addr=INADDR_ANY; /* Accept all adresses */
    bzero((void *)&(Direccion.sin_zero), 8);

    if(bind(sockfd, (struct sockaddr *)&Direccion, sizeof(struct sockaddr_in)) == -1){
        close(sockfd);
        return -1;
    }

    if (listen (sockfd, max_connections)<0){
        close(sockfd);
        return -1;
    }

    return sockfd;

}

int crea_conecta_socket_TCP(struct in_addr ip_dest, short puerto)
{

    int sockfd=-1;
    struct sockaddr_in sock_destino;

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1){
        return -1;
    }

    /*Conexion con el socket*/
    sock_destino.sin_family=AF_INET;
    sock_destino.sin_addr.s_addr = ip_dest.s_addr;
    sock_destino.sin_port = htons(puerto);
    if(connect(sockfd , (struct sockaddr *) &sock_destino, sizeof(sock_destino))<0){
		perror("");
        close(sockfd);
        return -1;
    }

    return sockfd;

}

int crea_registra_socket_UDP(short puerto){

    struct sockaddr_in ip;
    int sockfd;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd==-1){
        return -1;
    }

    /*Bind y espera de llamadas*/
    ip.sin_family=AF_INET;
    ip.sin_port = htons(puerto);
    ip.sin_addr.s_addr=INADDR_ANY; /* Accept all adresses */
    bzero((void *)&(ip.sin_zero), 8);
    if(-1==bind(sockfd,(const struct sockaddr *)&ip,sizeof(ip))){
        close(sockfd);
        return -1;
    }
    return sockfd;

}
