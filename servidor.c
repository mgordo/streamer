/**
 * @brief Este archivo el cuerpo de las funciones para el servidor RTSP
 * Redes II, Practica II
 * @file servidor.c
 * @author Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/

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
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>

#include "socket_funcs.h"
#include "RTSP.h"
#include "audio_serv.h"
#include "video_serv.h"


/*Defines para los codigos de retorno*/
#define HTTP_OK "200 OK"
#define HTTP_AGALLOWED "460 Only aggregate operation allowed"
#define HTTP_NOTFOUND "404 Not Found"
#define HTTP_BADREQUEST "400 Bad Request"


typedef enum{no_conectado, conectado_rtsp, conexion_aceptada, hilos_creados} estadoServ;
/**
 * @brief variable de estado para saber el que punto de la ejecucion estamos
*/
estadoServ estado=no_conectado;  

/**
 * @brief Puertos para RTP y RTCP del primer setup (audio)
*/
short client_rtp_port_1=-1,server_rtp_port_1=-1,client_rtcp_port_1=-1,server_rtcp_port_1=-1;

/**
 * @brief Puertos para RTP y RTCP del segundo setup (video)
*/
short client_rtp_port_2=-1,server_rtp_port_2=-1,client_rtcp_port_2=-1,server_rtcp_port_2=-1;

/**
 * @brief Rutas para los ficheros de audio y video
*/
char ruta_audio[TAM], ruta_video[TAM];

/**
 * @brief Numero de sesion
*/
int session=-1;

/**
 * @brief Descriptores de los socket. Son globales para leerlas al hacer Ctrl-C
*/
int sockfd=-1, sock=-1;

/**
 * @brief descriptores de los hilos principales de audio y video
*/
pthread_t hilo_audio, hilo_video;

/**
 * @brief Semaforos y variables para avisar que hay que detener el 
 * envio de audio y video
 * Los hilos leeran estas variables y en caso de que se les ordene parar, 
 * se quedaran bloqueados en los semaforos
*/
int estado_audio=Seguir, estado_video=Seguir; 
pthread_mutex_t sem_audio = PTHREAD_MUTEX_INITIALIZER, sem_video = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief descriptor del hilo de escucha RTSP
 * Este hilo se encargara de escuchar los posibles mensajes RTSP del cliente
 * y responder consecuentemente
*/
pthread_t hilo_escucha;

/**
 * @brief semaforo y variable para indicar al hilo principal o bien un teardown,
 * o bien que se ha acabado la reproduccion
 *
*/
pthread_mutex_t sem_main = PTHREAD_MUTEX_INITIALIZER;
int estado_main=Normal;

/*Funciones auxiliares*/
static int atiende_cliente(int sockfd);
static int espera_play(int sockfd);
static int crea_hilos_audio_video(struct sockaddr_in *sockDir);
static int escucha_RTSP(void);

/*Resto de funciones*/
static int aleat_num(int inf, int sup);
static void maneja_SIGINT(int t);

/**
 * @brief funcion principal del servidor
 * @param argc el numero de argumentos
 * @param argv una matriz cn los argumentos
 * @return 0 si todo bien y -1 en caso de error
 * 
*/

int main(int argc, char **argv)
{

    struct sockaddr_in sockDir;
    int tam;
	
    /*Comprobamos los argumentos*/
 	if(argc != 2){
		fprintf(stderr,"Error en el numero de argumentos\n");
		return -1;
	}
    srand(time(NULL));

    /*Creamos el socket RTSP*/
    sockfd = crea_socket_RTSP(atoi(argv[1]), 10);
    if(sockfd==-1) {
        fprintf(stderr,"Error al crear el socket RTSP\n");
        return -1;
    }

    /*Ponemos el estado a su valor correspondiente*/
    estado = conectado_rtsp;

    sockDir.sin_family = AF_INET;
    sockDir.sin_port = atoi(argv[1]);
    sockDir.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero((void *)&(sockDir.sin_zero), 8);

    /*Capturamos sigint para finalizar el programa*/
 	if(signal(SIGINT, maneja_SIGINT) == SIG_ERR){
        cierra_conexion_RTSP(sockfd);
        fprintf(stderr, "Error al capturar sigint\n");
        return -1;
    } 

    /*Comenzamos el bucle principal de atencion a los clientes*/
    while(1){
	   
        tam = sizeof(sockDir);

        sock=acepta_conexiones_RTSP(sockfd, (struct sockaddr *)&sockDir, (socklen_t *)&tam);
        if(sock==-1){
            fprintf(stderr,"Error al aceptar conexiones RTSP\n");
            close(sockfd);
            return -1;
        }
        estado = conexion_aceptada;
	  
		/*Recibimos los dos setups*/
        if(atiende_cliente(sock)==-1){
			close(sockfd);
            close(sock);
            return -1;
        }

        fprintf(stderr,"Describe y setup pedidos\n");
		fprintf(stderr,"puerto audio cliente: %d\n",client_rtp_port_1);
		fprintf(stderr,"puerto audio servidor: %d\n",server_rtp_port_1);
        fprintf(stderr,"puerto video cliente: %d\n",client_rtp_port_2);
		fprintf(stderr,"puerto video servidor: %d\n",server_rtp_port_2);

		/*Esperamos el play del cliente*/
		if(espera_play(sock) == -1){
			close(sockfd);
            close(sock);
            return -1;
        }

        printf("Play enviado\n");

		/*
			Creamos los hilos
			y nos ponemos a escuchar mas mensajes rtsp
		*/
		if(crea_hilos_audio_video(&sockDir) == -1){
			close(sockfd);
            close(sock);
            return -1;
        }


		printf("Hilos creados\n");
		estado = hilos_creados;

		/*Nos dormimos en un semaforo a la espera de que alguien nos despierte.
		Este dejara la razon en la variable "estado_main"*/
		pthread_mutex_lock(&sem_main);
		printf("Tomamos el semaforo\n");
		pthread_mutex_lock(&sem_main);
		printf("Nos despertamos\n");

		if(estado_main == Fin_Reproduccion_2){	/*Hemos acabado de reproducir audio y video*/

			printf("Final por fin de reproduccion\n");

			/*Finalizamos el hilo que esta escuchando mensajes RTSP*/
			pthread_cancel(hilo_escucha);
			
		}else if(estado_main == Llegada_Teardown){	/*Si nos han parado con teardown*/

			printf("Final por llegada de teardown\n");

			/*Avisamos a los hilos principales de audio y video que deben finalizar
			su ejecucion (y las de los posibles hilos que hallan creado)*/
			estado_audio = Terminar;
			estado_video = Terminar;

			/*Esperamos 3/4 segundos (se puede hacer con semaforos, pero esta solucion
			deberia bastar y queda mucho por hacer). En este tiempo, ya deberian 
			haber finalizado todos los hilos*/
			usleep(750000);

		}else if(estado_main == Error_Hilos){/*Si hubo algun error en los hilos, hay que finalizarlos y esperar otro cliente*/

			printf("Final por error en los hilos\n");

			/*Avisamos a los hilos principales de audio y video que deben finalizar
			su ejecucion (y las de los posibles hilos que hallan creado)*/
			estado_audio = Terminar;
			estado_video = Terminar;

			/*Esperamos 3/4 segundos (se puede hacer con semaforos, pero esta solucion
			deberia bastar y queda mucho por hacer). En este tiempo, ya deberian 
			haber finalizado todos los hilos*/
			usleep(750000);

			/*Finalizamos el hilo que esta escuchando mensajes RTSP*/
			pthread_cancel(hilo_escucha);

		}

		/*Cerramos el socket RTSP, volvemos a inicializar
		las variables y semaforos y nos ponemos a escuchar a otro cliente*/
		estado_audio=Seguir;
		estado_video=Seguir;
		estado_main=Normal;
		close(sock);
		estado=conectado_rtsp;

		/*reiniciamos todos los semaforos*/
		pthread_mutex_init(&sem_main,NULL);
		pthread_mutex_init(&sem_audio,NULL);
		pthread_mutex_init(&sem_video,NULL);
		
		
    }

	/*Nunca deberia llegar el programa a este punto, solo termina con Ctrl-C*/

    return 0;

}

/**
 * @brief funcion de atencion de un cliente. Escucha describes y dos setup
 * @param sockfd descriptor del socket RTSP
 * @return 0 si todo bien y -1 en caso de error
 * 
*/
static int atiende_cliente(int sockfd){

	Mensaje_RTSP mens;
	Resp_Describe menDisRes;
	Men_Error menErr;
	Resp_Setup menSetRes;

	DIR *dir;
	FILE *f=NULL;

	Men_Describe *menDis=NULL;
	Men_Setup *menSet=NULL;

	char cadena[TAM_CADENA];
	int contador=0;/*Cuando hallamos recibido dos mensajes de setup, saldremos*/

	session = rand();

	while(contador < 2){

		if(escucha_comandoRTSP(sockfd, &mens)==-1){
			return -1;
		}

		/*Si es un describe*/
		if(mens.tipo == Describe){

			menDis = (Men_Describe *)&mens;
			dir = opendir(menDis->ruta);

			if(dir == NULL){ /*Si no existe el directorio o no se pude acceder a el*/
                printf("No encontrado el directorio: %s\n", menDis->ruta);
				menErr.tipo = Error;
				menErr.sec = menDis->sec;
				strcpy(menErr.mensaje, HTTP_NOTFOUND);

				if(envia_comandoRTSP(sockfd, (Mensaje_RTSP *)&menErr) == -1){
					fprintf(stderr, "Error enviando respuesta de error RTSP\n");
					return -1;
				}
			}else{/*Si el directorio era correcto*/

                printf("Encontrado el directorio: %s\n", menDis->ruta);

				closedir(dir);
				menDisRes.tipo = Res_Describe;
				menDisRes.sec = menDis->sec;
				strcpy(menDisRes.mensaje, HTTP_OK);
				strcpy(menDisRes.content_type, "application/sdp");

				/*Leemos el fichero de config para ver lo que hay en el directorio*/
				strcpy(cadena, menDis->ruta);
				strcat(cadena,"/config");
				f = fopen(cadena,"r");
				if(f == NULL){
					fprintf(stderr, "Error leyendo fichero config\n");
					return -1;
				}
				fgets(menDisRes.m1,TAM_CADENA,f);
                (menDisRes.m1)[strlen(menDisRes.m1)-1] = '\0';
				fgets(menDisRes.a1,TAM_CADENA,f);
                (menDisRes.a1)[strlen(menDisRes.a1)-1] = '\0';
				fgets(menDisRes.m2,TAM_CADENA,f);
                (menDisRes.m2)[strlen(menDisRes.m2)-1] = '\0';
				fgets(menDisRes.a2,TAM_CADENA,f);
                (menDisRes.a2)[strlen(menDisRes.a2)-1] = '\0';
				fclose(f);
				
				if(envia_comandoRTSP(sockfd, (Mensaje_RTSP *)&menDisRes) == -1){
					fprintf(stderr, "Error enviando respuesta de discribe RTSP\n");
					return -1;
				}
			}
		}else if(mens.tipo == Setup){/*Si es un setup*/

			menSet = (Men_Setup *)&mens;

			if(contador == 0){
				client_rtp_port_1 = menSet->rtp_port;
				menSetRes.client_rtp_port = client_rtp_port_1;
				server_rtp_port_1 = aleat_num(1025,SHRT_MAX);
				menSetRes.server_rtp_port = server_rtp_port_1;
				client_rtcp_port_1 = menSet->rtcp_port;
				menSetRes.client_rtcp_port = client_rtcp_port_1;
				server_rtcp_port_1 = server_rtp_port_1+1;
				menSetRes.server_rtcp_port = server_rtcp_port_1;

				strcpy(ruta_audio, menSet->ruta+7);/*Le quitamos la parte de rtsp: */
			}else{
				client_rtp_port_2 = menSet->rtp_port;
				menSetRes.client_rtp_port = client_rtp_port_2;
				server_rtp_port_2 = aleat_num(1025,SHRT_MAX);
				menSetRes.server_rtp_port = server_rtp_port_2;
				client_rtcp_port_2 = menSet->rtcp_port;
				menSetRes.client_rtcp_port = client_rtcp_port_2;
				server_rtcp_port_2 = server_rtp_port_2+1;
				menSetRes.server_rtcp_port = server_rtcp_port_2;

				strcpy(ruta_video, menSet->ruta+7);/*Le quitamos la parte de rtsp: */
			}

			menSetRes.tipo = Res_Setup;
			menSetRes.sec = menSet->sec;
			strcpy(menSetRes.mensaje, HTTP_OK);
			menSetRes.session = session;

			if(envia_comandoRTSP(sockfd, (Mensaje_RTSP *)&menSetRes) == -1){
				fprintf(stderr, "Error enviando respuesta de setup RTSP\n");
				return -1;
			}
			contador++;

		}
	}
	return 0;		
	
}

/**
 * @brief funcion de espera de play
 * @param sockfd el descriptor del socket donde esperar el play
 * @return 0 si bien -1 si error
 * 
*/
static int espera_play(int sockfd)
{

	Mensaje_RTSP men;
	Resp_Play menResPlay;

	if(escucha_comandoRTSP(sockfd,&men) == -1){
		fprintf(stderr, "Error recibiendo play\n");
		return -1;
	}

	if(men.tipo != Play){
		fprintf(stderr, "Error, se esperaba un play\n");
		return -1;
	}

	menResPlay.tipo = Res_Pause_Play;
	menResPlay.sec = men.sec;
	strcpy(menResPlay.mensaje,HTTP_OK);
	menResPlay.session = session;

	if(envia_comandoRTSP(sockfd, (Mensaje_RTSP *)&menResPlay) == -1){
		fprintf(stderr, "Error enviando respuesta de play RTSP\n");
		return -1;
	}

	return 0;

}
/**
 * @brief esta funcion crea los hilos principales de audio y video (uno de cada)
 * @return 0 si todo bien y -1 si algo salio mal
 * 
*/
static int crea_hilos_audio_video(struct sockaddr_in *sockDir)
{

	/*Creamos el hilo que se encargara del audio*/
    if(pthread_create(&hilo_audio, NULL,main_audio, (void *)sockDir)){
		fprintf(stderr,"Error creando el hilo de audio\n");
        return -1;
    }

	/*Creamos el hilo que se encargara del video*/
    if(pthread_create(&hilo_video, NULL,main_video, (void *)sockDir)){
		fprintf(stderr,"Error creando el hilo de video\n");
        return -1;
    }

	/*Creamos el hilo de escucha de mas mensajes RTSP*/
	if(pthread_create(&hilo_escucha, NULL,(void *)escucha_RTSP, NULL)){
		fprintf(stderr,"Error creando el hilo de escucha RTSP\n");
        return -1;
    }

	return 0;
}

/**
 * @brief funcion para escuchar mensajes RTSP.
 * Este hilo se dedicara a escuchar del socket RTSP los posibles mensajes
 * que pueda enviar el cliente (PAUSE, PLAY para reanudar y TEARDOWN)
 * @return 0 si todo bien y -1 si mal
*/
static int escucha_RTSP(void)
{

	Mensaje_RTSP men;
	Men_Error menErr;
	Resp_Play menPlayPause;
	Resp_Info menInfo;

	DIR *dir=NULL;
	int fl=1;

	Men_Pause *menPause=NULL;
	Men_Play *menPlay=NULL;
	Men_Teardown *menTeardown=NULL;

    int ret;

	/*Bucle principal de escucha de mensajes RTSP*/
	while(fl == 1){

		if((ret =escucha_comandoRTSP(sock,&men)) == -1){
			fprintf(stderr, "Error recibiendo comando RTSP\n");
        }else if(ret == -2){
            fprintf(stderr,"Cerraron la conexion\n");
            /*Avisamos al hilo principal de que debe terminar de enviar audio y video*/
			estado_main = Llegada_Teardown;
			pthread_mutex_unlock(&sem_main);

			/*Y finalizo mi ejecucion*/
			fl = 0;
		}else{
	
			/*Si recibimos un pause*/
			if(men.tipo == Pause){
				
				menPause = (Men_Pause *)&men;
	
				/*Comprobamos que se quiera detener el flujo completo (audio y video)*/
				dir = opendir(menPause->ruta);
		
				if(dir == NULL){ /*Si no existe el directorio o no se pude acceder a el*/
					menErr.tipo = Error;
					menErr.sec = menPause->sec;
					strcpy(menErr.mensaje, HTTP_AGALLOWED);
		
					if(envia_comandoRTSP(sock, (Mensaje_RTSP *)&menErr) == -1){
						fprintf(stderr, "Error enviando respuesta de error RTSP\n");
						return -1;
					}
				}else{	/*Si el pause era correcto*/
					
					closedir(dir);
					menPlayPause.tipo = Res_Pause_Play;
					menPlayPause.sec = men.sec;
					strcpy(menPlayPause.mensaje, HTTP_OK);
					menPlayPause.session = menPause->session;
		
					/*Enviamos la respuesta*/
					if(envia_comandoRTSP(sock, (Mensaje_RTSP *)&menPlayPause) == -1){
						fprintf(stderr, "Error enviando respuesta de pause RTSP\n");
						return -1;
					}
		
					/*Y bloqueamos los hilos*/
					estado_audio=Parar;
					pthread_mutex_lock(&sem_audio);
					estado_video=Parar;
					pthread_mutex_lock(&sem_video);
		
				}
		
			}else if(men.tipo == Play){	/*Si nos envian un play, para reanudar despues del pause*/
	
				menPlay = (Men_Play *)&men;
	
				menPlayPause.tipo = Res_Pause_Play;
				menPlayPause.sec = men.sec;
				strcpy(menPlayPause.mensaje, HTTP_OK);
				menPlayPause.session = menPlay->session;
	
				/*Desbloqueamos los hilos*/
				estado_audio=Seguir;
				pthread_mutex_unlock(&sem_audio);
				estado_video=Seguir;
				pthread_mutex_unlock(&sem_video);
	
				/*Enviamos la respuesta*/
				if(envia_comandoRTSP(sock, (Mensaje_RTSP *)&menPlayPause) == -1){
					fprintf(stderr, "Error enviando respuesta de play RTSP\n");
					return -1;
				}
			}else if(men.tipo == Teardown){	/*Si nos envian un teardown*/
	
				menTeardown = (Men_Teardown *)&men;
	
				menInfo.tipo = Info;
				menInfo.sec = men.sec;
				strcpy(menInfo.mensaje, HTTP_OK);

				/*Enviamos la respuesta*/
				if(envia_comandoRTSP(sock, (Mensaje_RTSP *)&menInfo) == -1){
					fprintf(stderr, "Error enviando respuesta de teardown RTSP\n");
					return -1;
				}

				/*Avisamos al hilo principal de que debe terminar de enviar audio y video*/
				estado_main = Llegada_Teardown;
				pthread_mutex_unlock(&sem_main);

				/*Y finalizo mi ejecucion*/
				fl = 0;
			}
		}
	}
	
	pthread_exit(NULL);
}

/**
 * @brief funcion para generar numeros aleatorios entre unos limites
 * @param inf limite inferior
 * @param sup numero superior
 * @return el numero generado
 * 
*/
static int aleat_num(int inf, int sup){

	int copia=0;
	float trans=0;
	if(sup<inf){
		copia=sup;
		sup=inf;
		inf=copia;
	}
	
	trans=(rand()/(RAND_MAX+1.0));
	trans=trans*(sup-inf+1)+inf;
	return (int)trans;
  
}

/**
 * @brief funcion manejadora de SIGINT. Libera recursos y finaliza el programa
 * @param t el numero de la sennal que llega
 * 
*/
static void maneja_SIGINT(int t){

    if(estado == conectado_rtsp){
		fprintf(stderr,"Cerrando el socket RTSP\n");
        close(sockfd);
    }else if(estado == conexion_aceptada){
		fprintf(stderr,"Cerrando el socket RTSP y el socket con el cliente\n");
        close(sock);
        close(sockfd);      
    }else if(estado == hilos_creados){
		fprintf(stderr,"Cerrando los hilos, cerramos los sockets y cancelamos todos los hilos\n");
		/*Avisamos a los hilos principales de audio y video que deben finalizar
		su ejecucion (y las de los posibles hilos que hallan creado)*/
		estado_audio = Terminar;
		estado_video = Terminar;

		/*Esperamos 3/4 segundos (se puede hacer con semaforos, pero esta solucion
		deberia bastar y queda mucho por hacer). En este tiempo, ya deberian 
		haber finalizado todos los hilos*/
		usleep(750000);
        close(sock);
		close(sockfd);
		
	}

	fprintf(stderr,"Servidor de video finalizado\n");

    exit(0);
}



