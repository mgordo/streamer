/**
 * @brief Este archivo contiene las funciones para el envio del audio
 * Redes II, Practica II
 * @file audio_serv.c
 * @author Diego Gonzalez y Miguel Gordo
 * @date 17-I-12
 * 
*/

#include "audio_serv.h"

/**
 * @brief variacion en el tiempo de envio que se producira por RTCP
*/
#define VAR_TIEMPO 10

/**
 * @brief descriptor del socket RTCP
*/
static int sock_rtcp=-1;

/**
 * @brief descriptor del fichero donde se almacenaran las estadisticas de la conexion
*/
static FILE *f = NULL;

/**
 * @brief numero de paquetes enviados en total
*/
static int enviados=0;

/**
 * @brief descriptor del hilo que escucha paquetes RTCP
*/
static pthread_t hilo_escucha_rtcp;

/**
 * @brief descriptor del hilo que escucha paquetes RTCP
*/
static pthread_t hilo_envio_rtcp;

/**
 * @brief tiempo de espera entre envios de paquetes RTP
*/
static int tiempo=75000;

/**
 * @brief numero de sincronizacion
*/
static int ssrc;

/**
 * @brief direccion del cliente
*/
static struct sockaddr_in dir;

/*Funciones auxiliares*/
static int crea_hilos_RTCP(void);

/*Funciones con las que seran invocados los hilos RTCP*/
static void *escucha_rtcp(void *);
static void *envia_rtcp(void *);

void * main_audio(void *arg)
{

	int file=-1, sockfd = -1;
	int leidos=0;
	char buff[ENV_AUDIO];
	short sec=0;     /*numero de secuencia, inicialmente 0*/

	ssrc = rand();

	/*Estructura que contiene la IP del cliente RTP*/
	dir = *((struct sockaddr_in *)arg);
	dir.sin_port = htons(client_rtp_port_1); /*Le asignamos el puerto correcto, el RTP*/

	/*Creamos el socket por donde enviar la informacion*/
	sockfd = crea_socket_RTP(server_rtp_port_1);
	if(sockfd == -1){
		fprintf(stderr,"Error al intentar abrir el socket RTP de audio\n");
		estado_main = Error_Hilos;
		pthread_mutex_unlock(&sem_main);
		pthread_exit(NULL);
	}

	/*Abrimos el fichero a enviar*/
	file = open(ruta_audio,O_RDONLY);
	if(file == -1){
		fprintf(stderr,"Error al intentar abrir el fichero: %s\n", ruta_audio);
		close(sockfd);
		estado_main = Error_Hilos;
		pthread_mutex_unlock(&sem_main);
		pthread_exit(NULL);
	}

	/*Creamos los hilos RTCP*/
	if(crea_hilos_RTCP() == -1){
		fprintf(stderr,"Error al crear los hilos RTCP\n");
		close(file);
		close(sockfd);
		estado_main = Error_Hilos;
		pthread_mutex_unlock(&sem_main);
		pthread_exit(NULL);
	}
	
	/*Nos ponemos a enviar hasta que se acabe el fichero o nos manden acabar*/
	while((leidos = read(file, buff, ENV_AUDIO)) > 0){

        usleep(tiempo);

		/*Antes de cada envio, comprobamos si el main ha hecho algo nuevo*/
		if(estado_audio == Seguir){

			envia_paquete_RTP(sockfd, (struct sockaddr *)&dir, sizeof(struct sockaddr_in), 
					  	  sec, sec*TEMP, ssrc, buff, ENV_AUDIO);

			sec++;
            enviados++;
            
		}else if(estado_audio == Parar){/*Si nos mandan parar, nos quedamos en el semaforo correspondiente*/

			pthread_mutex_lock(&sem_audio);

		}else{/*Si nos mandan terminar, cerramos los recursos abiertos y terminamos*/

			close(sockfd);
			close(file);

			/*Cancelamos los hilos RTCP y cerramos los recursos que hallan abierto*/
			pthread_cancel(hilo_escucha_rtcp);
			pthread_cancel(hilo_envio_rtcp);
			fclose(f);
			close(sock_rtcp);

			pthread_exit(NULL);
		}

	}

	/*Si hemos acabado con el fichero, avisamos al main y finalizamos*/
	estado_main++;
	/*Si tambien ha acabado el otrofichero, despertamos al main*/
	if(estado_main == Fin_Reproduccion_2)
		pthread_mutex_unlock(&sem_main);

	close(file);
	close(sockfd);
	/*Cancelamos los hilos RTCP y cerramos los recursos que hallan abierto*/
	pthread_cancel(hilo_escucha_rtcp);
	pthread_cancel(hilo_envio_rtcp);
	fclose(f);
	close(sock_rtcp);

	pthread_exit(NULL);
}

/**
 * @brief funcion que crea los hilos y recursos necesarios para RTCP
 * @return 0 si bien -1 si hubo error
*/
static int crea_hilos_RTCP(void)
{

	/*Abrimos el fichero de logs*/
	f = fopen(AUDIO_FICH,"w");
	if(f == NULL) return -1;

	/*Creamos el socket RTCP*/
	if((sock_rtcp = crea_socket_RTCP(server_rtcp_port_1)) == -1){
		fclose(f);
		return -1;
	}

	/*Creamos el hilo de escucha RTCP*/
	if(pthread_create(&hilo_escucha_rtcp, NULL,escucha_rtcp, NULL)){
		fprintf(stderr,"Error creando el hilo de escucha RTCP\n");
		fclose(f);
		close(sock_rtcp);
        return -1;
    }

	/*Creamos el hilo de envio RTCP*/
	if(pthread_create(&hilo_envio_rtcp, NULL,envia_rtcp, NULL)){
		fprintf(stderr,"Error creando el hilo de escucha RTCP\n");
		fclose(f);
		close(sock_rtcp);
		pthread_cancel(hilo_escucha_rtcp);
        return -1;
    }

	return 0;

}

/**
 * @brief hilo de escucha de paquetes RTCP
 * @param nada puntero a los argumentos, que en nuestro caso siempre es NULL
 * @return NULL
*/
static void *escucha_rtcp(void *nada)
{

	Mensaje_RTCP rec;
	RR *rr_rec;
	struct sockaddr_in dir;
	socklen_t tam;
	int enviados_fijo;
	double por_lost;
	int i=0;

	/*Nos ponemos a escuchar mensajes RTCP. Cada vez que recibimos uno, 
	guardamos la informacion relevante en AUDIO_FICH y cambiamos
	el ritmo al que se envian los mensajes RTP*/
	while(1){

		tam = sizeof(struct sockaddr_in);

		if(escucha_comandoRTCP(sock_rtcp, (struct sockaddr *)&dir, &tam, &rec) == -1){/*Si hubo error al recibir el paquete*/
			fprintf(stderr,"Error recibiendo mensaje RTCP audio\n");
		}else{/*Si se recibio correctamente*/
			
			if(rec.tipo == rr_t){/*Comprobamos que sea un RR*/

				rr_rec = (RR *)&rec;

				enviados_fijo = enviados;

				por_lost = ((double)(rr_rec->cum_lost)) / enviados_fijo;

				if(por_lost >= 0.2){/*Si se pierde mas del 10% de los paquetes*/
					tiempo+=VAR_TIEMPO;
				}else{/*Si se pierden menos del 10%*/
					tiempo-=VAR_TIEMPO;
				}

				/*escribimos la informacion en el fichero AUDIO_FICH*/
				/*El formato es: por_predidos jitter*/
				fprintf(f,"%d %lf %d\n",i, por_lost, rr_rec->jitter);
				i++;

			}
		}
	}

	/*Numca deberia llegar aqui el programa*/
	pthread_exit(NULL);
}

/**
 * @brief hilo de envio de paquetes RTCP
 * @param nada puntero a los argumentos, que en nuestro caso siempre es NULL
 * @return NULL
*/
static void *envia_rtcp(void *nada)
{

	SR send_rep;
	socklen_t tam;
    struct sockaddr_in dir_rtcp;

	tam = sizeof(struct sockaddr_in);

    dir_rtcp.sin_port = htons(client_rtcp_port_1);

	send_rep.tipo = sr_t;
	send_rep.ssrc_sender = ssrc;

	/*Bucle principal, cada USLEEPRTCP nos despertamos y enviamos un SR*/
	while(1){

		usleep(USLEEPRTCP);

		send_rep.p_count = enviados;
		send_rep.o_count = enviados/8;

		if(envia_comandoRTCP(sock_rtcp, (struct sockaddr *)&dir_rtcp, tam , (Mensaje_RTCP *)&send_rep) == -1){
			fprintf(stderr,"Error al enviar SR\n");
		}
	}

	/*Numca deberia llegar aqui el programa*/
	pthread_exit(NULL);
}

