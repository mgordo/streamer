/**
 * @brief Este archivo contiene el cuerpo de las funciones para el modulo RTSP
 * Redes II, Practica II
 * @file RTSP.c
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/

#include "RTSP.h"
/*Declaracion de funciones estáticas*/
/*Funciones para envio*/
static int envia_Describe(int sockfd,Men_Describe * mensaje);
static int envia_Pause(int sockfd,Men_Pause * mensaje);
static int envia_Play(int sockfd,Men_Play * mensaje);
static int envia_Setup(int sockfd,Men_Setup * mensaje);
static int envia_Res_Describe(int sockfd, Resp_Describe * mensaje);
static int envia_Res_Info(int sockfd, Resp_Info * mensaje);
static int envia_Res_Setup(int sockfd, Resp_Setup * mensaje);
static int envia_Error(int sockfd, Men_Error * mensaje);
static int envia_Teardown(int sockfd,Men_Teardown *mensaje);
static int envia_Res_Pause_Play(int sockfd, Resp_Play * mensaje);
/*Funciones estaticas para recepcion*/
static int recibe_Pause(char *texto,Men_Pause *mensaje);
static int recibe_Teardown(char *texto,Men_Teardown *mensaje);
static int recibe_Setup(char *texto,Men_Setup *mensaje);
static int recibe_Describe(char *texto,Men_Describe *mensaje);
static int recibe_Play(char *texto,Men_Play *mensaje);
static int recibe_Resp_Setup(char *texto,Resp_Setup *mensaje);
static int recibe_Resp_Describe(char *texto,Resp_Describe *mensaje);
static int recibe_Resp_Pause(char *texto,Resp_Play *mensaje);
static int recibe_Resp_Info(char *texto,Resp_Info *mensaje);

int escucha_comandoRTSP(int sockfd, Mensaje_RTSP *mensaje){
	int tamano;
	char texto[TAMANO_RESTO+8];
	tamano=recv(sockfd,texto,TAMANO_RESTO+8,0);

    /*Si hubo algun problema en la recepcion*/
	if(tamano==-1) return -1;

    /*Si se recibieron 0 bytes, se cerro la conexion*/
    if(tamano == 0) return -2;
	/*Ver de que tipo de mensaje se trata*/
	/*Primero ver si es de cliente a servidor*/
	if(strstr(texto,"PAUSE")){
		if(recibe_Pause(texto,(Men_Pause *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	if(strstr(texto,"DESCRIBE")){
		if(recibe_Describe(texto,(Men_Describe *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	if(strstr(texto,"PLAY")){
		if(recibe_Play(texto,(Men_Play *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	if(strstr(texto,"TEARDOWN")){
		if(recibe_Teardown(texto,(Men_Teardown *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	if(strstr(texto,"SETUP")){
		if(recibe_Setup(texto,(Men_Setup *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	/*Ahora a ver si son respuestas del server*/
	if(strstr(texto,"client_port")){
		if(recibe_Resp_Setup(texto,(Resp_Setup *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	if(strstr(texto,"Content-Type")){
		if(recibe_Resp_Describe(texto,(Resp_Describe *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	if(strstr(texto,"Session")){/*Por eliminacion, es respuesta a un Pause o play. En otro caso, es respuesta Teardown o error*/
		if(recibe_Resp_Pause(texto,(Resp_Play *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	else{/*En otro caso, es respuesta a Teardown o error (tipo Resp_Info en cualquier caso)*/
		if(recibe_Resp_Info(texto,(Resp_Info *)mensaje)==-1){
			return -1;
		}
		return 0;
	}
	
}

static int recibe_Resp_Info(char *texto,Resp_Info *mensaje){

	char *p=NULL, *n=NULL;

	p = strstr(texto,"\nCSeq");
	if(!p) {
        fprintf(stderr,"Mensaje extranno: %s\n", texto);
        return -1;
    }
	*p = '\0';
	n = texto+9;
	strcpy(mensaje->mensaje, n);

	p++;
	sscanf(p,"CSeq: %d\n",&(mensaje->sec));

	mensaje->tipo=Info;
	return 0;
}

/**
 * @brief Esta funcion gestiona un una respuesta RTSP a un comando PAUSE que acaba de ser recibida
 * @param texto es la respuesta al PAUSE en texto plano
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Resp_Pause(char *texto,Resp_Play *mensaje){

	char *p=NULL;
	p=strstr(texto,"CSeq: ");
	*p='\0';
	strcpy(mensaje->mensaje,texto+9);
	sscanf(p+6,"%d\nSession: %d\n",&(mensaje->sec),&(mensaje->session));
	mensaje->tipo=Res_Pause_Play;
	return 0;
}


/**
 * @brief Esta funcion gestiona un una respuesta RTSP a un comando SETUP que acaba de ser recibida
 * @param texto es la respuesta al SETUP en texto plano
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Resp_Setup(char *texto,Resp_Setup *mensaje){

	char *p=NULL;
	p=strstr(texto,"\nCSeq: ");
	*p='\0';
	strcpy(mensaje->mensaje,texto+9);
	sscanf(p+7,"%d\nTransport: RTP/RTCP; unicast; client_port=%hd-%hd; server_port=%hd-%hd\nSession: %d\n",&(mensaje->sec),&(mensaje->client_rtp_port),&(mensaje->client_rtcp_port),&(mensaje->server_rtp_port),&(mensaje->server_rtcp_port),&(mensaje->session));
	mensaje->tipo=Res_Setup;
	return 0;
}

/**
 * @brief Esta funcion gestiona un una respuesta RTSP a un comando DESCRIBE que acaba de ser recibida
 * @param texto es la respuesta al DESCRIBE en texto plano
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Resp_Describe(char *texto,Resp_Describe *mensaje){

	char *p=NULL,*n=NULL;
	p=strstr(texto,"\nCSeq: ");
	*p='\0';
	strcpy(mensaje->mensaje,texto+9);
	p=p+7;
	n=strstr(p,"\nContent-Type: ");
	*n='\0';
	sscanf(p,"%d",&(mensaje->sec));
	n+=15; /*n al comienzo de lo que hay en Content-type*/
	p=strstr(n, "\nm= ");
	*p='\0';
	strcpy(mensaje->content_type,n);

	n = p+4; /*n esta al principio del contenido del primer m=*/
	p=strstr(n,"\na= ");
	*p='\0';
	strcpy(mensaje->m1,n);

	n = p+4; /*n esta al principio del contenido del primer a=*/
	p=strstr(n,"\nm= ");
	*p='\0';
	strcpy(mensaje->a1,n);

	n = p+4; /*n esta al principio del contenido del segundo m=*/
	p=strstr(n,"\na= ");
	*p='\0';
	strcpy(mensaje->m2,n);

	n = p+4; /*n esta al principio del contenido del segundo a=*/
	p=strstr(n,"\n");
	*p='\0';
	strcpy(mensaje->a2,n);

	mensaje->tipo=Res_Describe;

	return 0;
}

/**
 * @brief Esta funcion gestiona un mensaje RTSP Pause recibido
 * @param texto es el mensaje RTSP en texto plano recibido
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Pause(char *texto,Men_Pause *mensaje){
	
	char *p=NULL, *n=NULL;

	/*Leemos la ruta*/
	p = strstr(texto," RTSP/1.0");
	*p = '\0';
	n = texto+6;
	strcpy(mensaje->ruta, n);

	/*Leemos el resto de argumentos*/
	/*Primero, avanzamos p uno mas para que no se quede en el '\0'*/
	p++;
	sscanf(p,"RTSP/1.0\nCSeq: %d\nSession: %d\n",&(mensaje->sec),&(mensaje->session));

	mensaje->tipo=Pause;
	return 0;
	
}

/**
 * @brief Esta funcion gestiona un mensaje RTSP Teardown recibido
 * @param texto es el mensaje RTSP en texto plano recibido
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Teardown(char *texto,Men_Teardown *mensaje){
	
	char *p=NULL, *n=NULL;

	/*Leemos la ruta, el primero de los parametros*/
	p = strstr(texto, " RTSP/1.0");
	*p = '\0';
	n = texto+9;
	strcpy(mensaje->ruta, n);

	/*Leemos los otros 2 argumentos, ya numericos*/
	p++;
	sscanf(p,"RTSP/1.0\nCSeq: %d\nSession: %d\n",&(mensaje->sec),&(mensaje->session));

	mensaje->tipo=Teardown;
	return 0;
}

/**
 * @brief Esta funcion gestiona un mensaje RTSP Setup recibido
 * @param texto es el mensaje RTSP en texto plano recibido
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Setup(char *texto,Men_Setup *mensaje){

	char *p=NULL, *n=NULL;

	/*Leemos el primer argumento*/
	p = strstr(texto," RTSP/1.0");
	*p = '\0';
	n = texto+6;
	strcpy(mensaje->ruta,n);

	/*Leemos el resto de argumentos numericos*/
	p++;	

	sscanf(p,"RTSP/1.0\nCSeq: %d\nTransport: RTP/RTCP; unicast; client_port=%hd-%hd\nSession: %d\n",&(mensaje->sec),&(mensaje->rtp_port),&(mensaje->rtcp_port),&(mensaje->session));

	mensaje->tipo=Setup;
	return 0;
}

/**
 * @brief Esta funcion gestiona un mensaje RTSP Describe recibido
 * @param texto es el mensaje RTSP en texto plano recibido
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Describe(char *texto,Men_Describe *mensaje){
	
	char *p=NULL, *n=NULL;

	p = strstr(texto," RTSP/1.0");
	*p='\0';
	n = texto+9;
	strcpy(mensaje->ruta,n);

	p++;
	sscanf(p,"RTSP/1.0\nCSeq: %d\n",&(mensaje->sec));

	mensaje->tipo=Describe;
	return 0;
}

/**
 * @brief Esta funcion gestiona un mensaje RTSP Play recibido
 * @param texto es el mensaje RTSP en texto plano recibido
 * @param mensaje es la estructura a rellenar con los datos de texto
 * @return 0 si OK, -1 en caso contrario
*/
static int recibe_Play(char *texto,Men_Play *mensaje){
	
	char *p=NULL,*n=NULL;

	p = strstr(texto," RTSP/1.0");
	*p = '\0';
	n = texto+5;
	strcpy(mensaje->ruta,n);

	p++;
	sscanf(p,"RTSP/1.0\nCSeq: %d\nSession: %d\n",&(mensaje->sec),&(mensaje->session));

	mensaje->tipo=Play;
	return 0;
}

int envia_comandoRTSP(int sockfd, Mensaje_RTSP *mensaje){
	
	switch(mensaje->tipo){
		case Describe:
			if(envia_Describe(sockfd,(Men_Describe *) mensaje)==-1){
				return -1;
			}
			break;
		case Setup:
			if(envia_Setup(sockfd,(Men_Setup *) mensaje)==-1){
				return -1;
			}
			break;
		case Play:
			if(envia_Play(sockfd,(Men_Play *) mensaje)==-1){
				return -1;
			}
			break;
		case Pause:
			if(envia_Pause(sockfd,(Men_Pause *) mensaje)==-1){
				return -1;
			}
			break;
		case Res_Pause_Play:
			if(envia_Res_Pause_Play(sockfd, (Resp_Play *) mensaje)==-1){
				return -1;
			}
			break;
		case Res_Describe:
			if(envia_Res_Describe(sockfd,(Resp_Describe *) mensaje)==-1){
				return -1;
			}
			break;
		case Res_Setup:
			if(envia_Res_Setup(sockfd,(Resp_Setup *) mensaje)==-1){
				return -1;
			}
			break;
		case Info:
			if(envia_Res_Info(sockfd,(Resp_Info *) mensaje)==-1){
				return -1;
			}
			break;
		case Error:
			if(envia_Error(sockfd,(Men_Error *)mensaje)==-1){
				return -1;
			}
			break;
		case Teardown:
			if(envia_Teardown(sockfd,(Men_Teardown *)mensaje)==-1){
				return -1;
			}
			break;
		default:
			return -1;
	}
	return 0;
	
}


/**
 * @brief Esta funcion envia un comando Describe de RTSP al servidor al que se este conectado
 * @param sockfd es el descriptor de socket al que mandar el comando
 * @param mensaje es la estrucutura que contiene el comando a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Describe(int sockfd, Men_Describe * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"DESCRIBE %s RTSP/1.0\nCSeq: %d\n",mensaje->ruta,mensaje->sec);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}

/**
 * @brief Esta funcion envia un comando Pause de RTSP al servidor al que se este conectado
 * @param sockfd es el descriptor de socket al que mandar el comando
 * @param mensaje es la estrucutura que contiene el comando a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Pause(int sockfd,Men_Pause * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"PAUSE %s RTSP/1.0\nCSeq: %d\nSession: %d\n",mensaje->ruta,mensaje->sec,mensaje->session);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}

/**
 * @brief Esta funcion envia un comando Play de RTSP al servidor al que se este conectado
 * @param sockfd es el descriptor de socket al que mandar el comando
 * @param mensaje es la estrucutura que contiene el comando a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Play(int sockfd,Men_Play * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"PLAY %s RTSP/1.0\nCSeq: %d\nSession: %d\n",mensaje->ruta,mensaje->sec, mensaje->session);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}

/**
 * @brief Esta funcion envia un comando Setup de RTSP al servidor al que se este conectado
 * @param sockfd es el descriptor de socket al que mandar el comando
 * @param mensaje es la estrucutura que contiene el comando a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Setup(int sockfd,Men_Setup * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"SETUP %s RTSP/1.0\nCSeq: %d\nTransport: RTP/RTCP; unicast; client_port=%hd-%hd\nSession: %d\n",mensaje->ruta,mensaje->sec,mensaje->rtp_port, mensaje->rtcp_port,mensaje->session);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}

/**
 * @brief Esta funcion envia una respuesta al comando Describe de RTSP al cliente
 * @param sockfd es el descriptor de socket al que mandar la respuesta
 * @param mensaje es la estrucutura que contiene la respuesta a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Res_Describe(int sockfd, Resp_Describe * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"RTSP/1.0 %s\nCSeq: %d\nContent-Type: %s\nm= %s\na= %s\nm= %s\na= %s\n",mensaje->mensaje,mensaje->sec,mensaje->content_type,mensaje->m1,mensaje->a1,mensaje->m2,mensaje->a2);

	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}


/**
 * @brief Esta funcion envia una respuesta al comando Play de RTSP al cliente
 * @param sockfd es el descriptor de socket al que mandar la respuesta
 * @param mensaje es la estrucutura que contiene la respuesta a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Res_Pause_Play(int sockfd, Resp_Play * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"RTSP/1.0 %s\nCSeq: %d\nSession: %d\n",mensaje->mensaje,mensaje->sec,mensaje->session);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}
/**
 * @brief Esta funcion envia una respuesta generica con informacion de RTSP al cliente
 * Puede ser un error o una respuesta a un pause
 * @param sockfd es el descriptor de socket al que mandar la respuesta
 * @param mensaje es la estrucutura que contiene la respuesta a enviar
 * @return 0 si OK, -1 en caso contrario
*/


static int envia_Res_Info(int sockfd, Resp_Info * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"RTSP/1.0 %s\nCSeq: %d\n%s\n",mensaje->mensaje,mensaje->sec,mensaje->relleno);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}

/**
 * @brief Esta funcion envia una respuesta al comando Setup de RTSP al cliente
 * @param sockfd es el descriptor de socket al que mandar la respuesta
 * @param mensaje es la estrucutura que contiene la respuesta a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Res_Setup(int sockfd, Resp_Setup * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"RTSP/1.0 %s\nCSeq: %d\nTransport: RTP/RTCP; unicast; client_port=%hd-%hd; server_port=%hd-%hd\nSession: %d\n",mensaje->mensaje,mensaje->sec,mensaje->client_rtp_port,mensaje->client_rtcp_port,mensaje->server_rtp_port,mensaje->server_rtcp_port,mensaje->session);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}
/**
 * @brief Esta funcion envia un mensaje de error al cliente
 * @param sockfd es el descriptor de socket al que mandar el comando
 * @param mensaje es la estrucutura que contiene el error a enviar
 * @return 0 si OK, -1 en caso contrario
*/
static int envia_Error(int sockfd, Men_Error * mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"RTSP/1.0 %s\nCSeq: %d\n",mensaje->mensaje,mensaje->sec);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}

static int envia_Teardown(int sockfd,Men_Teardown *mensaje){
	char comando[sizeof(Mensaje_RTSP)];
	
	sprintf(comando,"TEARDOWN %s RTSP/1.0\nCSeq: %d\nSession: %d\n",mensaje->ruta,mensaje->sec,mensaje->session);
	if(send(sockfd, comando, sizeof(comando), 0)==-1){
		return -1;
	}
	return 0;
}



int crea_socket_RTSP(int puerto,int listeners){

	int sock;

    if(-1==(sock=crea_registra_escucha_socket_TCP(puerto, listeners))){
        return -1;
    }
    return sock;
}

int acepta_conexiones_RTSP(int sockfd,struct sockaddr *sockdir, socklen_t *tam){
    int sock;
    sock=accept(sockfd, sockdir, tam);
    if(sock==-1) {
		perror("");
		return -1;
	}
    else return sock;
}


int cierra_conexion_RTSP(int sockfd){

    close(sockfd);
    return 0;

}

int conecta_socket_RTSP(int sockfd, struct sockaddr *sockdir, socklen_t *tam){

    if(connect(sockfd , sockdir, *tam)<0) return -1;
    return 0;

}



