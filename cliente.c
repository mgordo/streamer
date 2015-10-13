/**
 * @brief Este archivo contiene el cuerpo de las funciones para el cliente
 * Redes II, Practica II
 * @file cliente.c
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
 */


#include "cliente.h"

/*Variables globales para el manejo desde otros modulos*/
/**
 * @brief identificadores de los hilos que se encargan de la recepcion de audio y video y escriben al pipe
*/
pthread_mutex_t sem_audio_cliente = PTHREAD_MUTEX_INITIALIZER,sem_video_cliente = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief variable para indicar a los hilos que envian al pipe los flujos
*/
Estado_Flujos estado_audio=Seguir,estado_video=Seguir;

/**
 * @brief Direccion IP del servidor
*/
struct in_addr ip_serv;

/**
 * @brief identificadores de los hilos que se encargan de la recepcion de audio y video y escriben al pipe
*/
pthread_t id_hilo_audio_rtp,id_hilo_video_rtp;


/**
 * @brief identificadores de los hilos que se encargan de rtcp
*/
pthread_t id_hilo_audio_escucha_rtcp,id_hilo_video_escucha_rtcp,id_hilo_audio_envio_rtcp,id_hilo_video_envio_rtcp;


/**
 * @brief identificadores del hilo que se encarga de la reproduccion con gst
*/
pthread_t id_hilo_control;

/**
 * @brief array con los ficheros de donde leera el hilo de reproduccion
*/
gint fichero_audio[2], fichero_video[2];

/********************* RTP *********************/ 

/**
 * @brief puerto RTP del audio
*/
short audio_rtp_port;

/**
 * @brief puerto RTP del video
*/
short video_rtp_port;

/******************** RTCP *********************/ 

/**
 * @brief puerto RTCP del audio server
*/
short audio_rtcp_port_server;

/**
 * @brief puerto RTCP del video server
*/
short video_rtcp_port_server;

/**
 * @brief puerto RTCP del audio
*/
short audio_rtcp_port_client;

/**
 * @brief puerto RTCP del video
*/
short video_rtcp_port_client;

/**
 * @brief descriptor del socket RTCP de audio
*/
int audio_rtcp_sockfd;

/**
 * @brief descriptor del socket RTCP de video
*/
int video_rtcp_sockfd;

/*Funciones con las que seran invocados los hilos RTCP*/
static void *escucha_audio_rtcp(void *);
static void *envia_audio_rtcp(void *);

static void *escucha_video_rtcp(void *);
static void *envia_video_rtcp(void *);

/******************* Funciones **************/

int tomaArgumentos(char *entrada, struct in_addr *ip, int *puerto){

	char *p=NULL;

	if(entrada == NULL) return -1;

	p = strtok(entrada," ");
	if(p == NULL) return -1;
	if(inet_aton((const char *)p,ip)==0) return -1;

	p = strtok(NULL," ");
	
	if(p==NULL) return -1;
	*puerto = atoi(p);
	if(*puerto == -1) return -1;
	p = strtok(NULL," ");

	if(p == NULL) return -1;
	strcpy(videofile,p);/*Variable global videofile*/

	return 0;

}

int lanza_recepcion_rtp(void){

	

	/*Creamos los ficheros donde iremos escribiendo audio y video*/
	/*fichero_audio = open(FICH_AUDIO,O_CREAT | O_WRONLY,  (S_IRWXU | S_IRWXG | S_IRWXO));
	if(fichero_audio == -1){
		fprintf(stderr,"Error creando el fichero de recepcion de audio\n");
	    return -1;
	}*/

	if(pipe2(fichero_audio, 0) != 0)
	{
    	perror("Error al crear el pipe para la comunicacion audio");
    	return -1;
	}
	
	if(pipe2(fichero_video, 0) != 0)
	{
    	perror("Error al crear el pipe para la comunicacion video");
    	close(fichero_audio[0]);
		close(fichero_audio[1]);
		return -1;
	}
	
	/*fichero_video = open(FICH_VIDEO,O_CREAT | O_WRONLY,  (S_IRWXU | S_IRWXG | S_IRWXO));
	if(fichero_video == -1){
		fprintf(stderr,"Error creando el fichero de recepcion de video\n");
	    return -1;
	}*/

	/*Lanzamos el hilo de recepcion de video*/
	if(pthread_create(&id_hilo_audio_rtp, NULL,main_audio_client, NULL)){
		fprintf(stderr,"Error creando el hilo de recepcion de audio\n");
    	close(fichero_audio[0]);
		close(fichero_audio[1]);
    	close(fichero_video[0]);
		close(fichero_video[1]);
	    return -1;
	}

	/*Lanzamos el hilo de recepcion de video*/
	if(pthread_create(&id_hilo_video_rtp, NULL,main_video_client, NULL)){
		fprintf(stderr,"Error creando el hilo de recepcion de video\n");
    	close(fichero_audio[0]);
		close(fichero_audio[1]);
    	close(fichero_video[0]);
		close(fichero_video[1]);
		pthread_cancel(id_hilo_audio_rtp);
	    return -1;
	}

    
	if(pthread_create(&id_hilo_control, NULL,main_control, NULL)){
		fprintf(stderr,"Error creando el hilo de recepcion de control\n");
    	close(fichero_audio[0]);
		close(fichero_audio[1]);
    	close(fichero_video[0]);
		close(fichero_video[1]);
		pthread_cancel(id_hilo_audio_rtp);
		pthread_cancel(id_hilo_video_rtp);
	    return -1;
	}

	return 0;
}

int lanza_rtcp(void){

	/*Creamos los sockets RTCP*/
	audio_rtcp_sockfd = crea_socket_RTCP(audio_rtcp_port_client);
	if(audio_rtcp_sockfd == -1){
		fprintf(stderr,"Error creando el socket RTCP de audio\n");
	    return -1;
	}

	video_rtcp_sockfd = crea_socket_RTCP(video_rtcp_port_client);
	if(video_rtcp_sockfd == -1){
		fprintf(stderr,"Error creando el socket RTCP de video\n");
		close(audio_rtcp_sockfd);
	    return -1;
	}

	printf("Sock Audio: %d\nSock Video: %d\n",audio_rtcp_sockfd,video_rtcp_sockfd);

	/*Creamos los hilos RTCP*/

	if(pthread_create(&id_hilo_audio_escucha_rtcp, NULL,escucha_audio_rtcp, NULL)){
		fprintf(stderr,"Error creando el hilo de recepcion de audio RTCP \n");
		close(audio_rtcp_sockfd);
		close(video_rtcp_sockfd);
	    return -1;
	}

	if(pthread_create(&id_hilo_video_escucha_rtcp, NULL,escucha_video_rtcp, NULL)){
		fprintf(stderr,"Error creando el hilo de recepcion de video RTCP \n");
		close(audio_rtcp_sockfd);
		close(video_rtcp_sockfd);
		pthread_cancel(id_hilo_audio_escucha_rtcp);
	    return -1;
	}

	if(pthread_create(&id_hilo_audio_envio_rtcp, NULL,envia_audio_rtcp, NULL)){
		fprintf(stderr,"Error creando el hilo de envio de audio RTCP \n");
		close(audio_rtcp_sockfd);
		close(video_rtcp_sockfd);
		pthread_cancel(id_hilo_audio_escucha_rtcp);
		pthread_cancel(id_hilo_video_escucha_rtcp);
	    return -1;
	}

	if(pthread_create(&id_hilo_video_envio_rtcp, NULL,envia_video_rtcp, NULL)){
		fprintf(stderr,"Error creando el hilo de envio de video RTCP \n");
		close(audio_rtcp_sockfd);
		close(video_rtcp_sockfd);
		pthread_cancel(id_hilo_audio_escucha_rtcp);
		pthread_cancel(id_hilo_video_escucha_rtcp);
		pthread_cancel(id_hilo_audio_envio_rtcp);
	    return -1;
	}

	return 0;

}

void *main_control(void *v){
	GMainLoop *loop;
	GstElement *pipeline;
	GstElement * vsource;
    
	/* Initialisation */
	gst_init (NULL, NULL);

	loop = g_main_loop_new (NULL, FALSE);

    pipeline = gst_parse_launch("fdsrc name=videosource ! decodebin2 name=dec ! queue !\
 ffmpegcolorspace ! autovideosink dec. fdsrc name=audiosource !\
 decodebin2 name=dec2 ! queue ! audioconvert ! audioresample ! autoaudiosink", NULL);
	if (!pipeline) {
		g_printerr ("One element could not be created. Exiting.\n");
		return NULL;
	}
	
	
	vsource = gst_bin_get_by_name (GST_BIN (pipeline), "videosource");

	g_object_set (G_OBJECT (vsource), "fd", fichero_video[0], NULL);

	vsource = gst_bin_get_by_name (GST_BIN (pipeline), "audiosource");

	g_object_set (G_OBJECT (vsource), "fd", fichero_audio[0], NULL);
	gst_element_set_state(GST_ELEMENT(pipeline),GST_STATE_PLAYING);

	g_main_loop_run (loop);
    printf("Y ME FUI\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT(pipeline));

	pthread_exit(NULL);


}


/*char commando[]="gst-launch-0.10 filesrc location=.video.dat ! decodebin2 name=dec ! queue ! ffmpegcolorspace ! autovideosink dec. filesrc location=.audio.dat ! decodebin2 name=dec2 ! queue ! audioconvert ! audioresample ! autoaudiosink";
    sleep(20);

	system(commando);

	pthread_exit(NULL);*/
/* Set up the pipeline */

	/* we set the input filename to the source element */

	
	/* we add all elements into the pipeline */
	/* file-source  vorbis-decoder | converter | alsa-output */
    /*gst_bin_add_many(GST_BIN(pipeline), source_audio, source_video, videodec, audiodec, audioqueue,
		   videoqueue, audioconv, audioesam, ffmpegcolorspace, videosink, audiosink, NULL);

    gst_element_link(source_audio, audiodec);
    gst_element_link(source_video, videodec);
    gst_element_link(videodec, videoqueue);
    gst_element_link(audiodec, audioqueue);
	

	gst_element_link_many(videoqueue, ffmpegcolorspace, videosink, NULL);
    gst_element_link_many(audioqueue, audioconv, audioesam, audiosink, NULL);*/

/* *source_audio, *source_video, *audioconv, *audiosink, *ffmpegcolorspace, *videosink, *audioqueue, *videoqueue, *audioesam;*/


/* Create gstreamer elements */
	/*pipeline = gst_pipeline_new ("media-player");
	source_audio   = gst_element_factory_make ("filesrc",       "file-source_audio");
	source_video   = gst_element_factory_make ("filesrc",       "file-source_video");
    audioqueue = gst_element_factory_make("queue", "audio-queue");
    videoqueue = gst_element_factory_make("queue", "video-queue");
    audioesam = gst_element_factory_make("audioresample",  "audio-esamble");
	audiodec  = gst_element_factory_make ("vorbisdec",     "vorbis-decoder");
	audioconv     = gst_element_factory_make ("audioconvert",  "converter");
	audiosink     = gst_element_factory_make ("autoaudiosink", "audio-output");
	videodec = gst_element_factory_make ( "theoradec" , "video-decoder" ) ;
	ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace" ,"ffmpegcolorspace");
	videosink = gst_element_factory_make ( "autovideosink" , "video-sink" ) ;*/

/*|| !source_video  || !audiodec || !audioconv || !audiosink || !ffmpegcolorspace || !videosink || !videodec || !audioesam || !videoqueue || ! audioqueue || !source_audio*/


/*static GstElement * videodec = NULL, * audiodec = NULL;*/


/**
 * @brief numero de paquetes de audio perdidos
*/
static short perdidos_audio=0;

/**
 * @brief jitter de audio
*/
static int jitter_audio=0;

/**
 * @brief numero de paquetes de audio perdidos
*/
static short perdidos_video=0;

/**
 * @brief jitter de audio
*/
static int jitter_video=0;

void *main_audio_client(void *v)
{

	int sockfd;
	struct sockaddr_in dir;
	gchar buff[ENV_AUDIO];
	socklen_t tam;
	short sec=0;
	int timestamp;
	int ssrc;
	int recibidos;
	int fl=0;

	/*Variables para el control RTCP*/
	int last_sec=-1;
	struct timeval tiempo_nuevo, tiempo_anterior;
	unsigned int retraso;

	/*Creamos el socket RTP para la recepcion*/
	sockfd = crea_socket_RTP(audio_rtp_port);
	if(sockfd == -1){
		fprintf(stderr,"Error creando el socket de escucha para la recepcion del audio\n");
	    pthread_exit(NULL);
	}

	gettimeofday(&tiempo_anterior,NULL);

	/*Nos ponemos a leer del socket y enviarlo al pipe*/
	while(fl == 0){

		/*Si tengo que seguir reproduciendo*/
		if(estado_audio == Seguir){
			
			/*Sacamos del socket paquetes*/
			recibidos =recibe_paquete_RTP(sockfd, (struct sockaddr *)&dir, &tam, 
					  &sec, &timestamp, &ssrc, (char *)buff, ENV_AUDIO);

			if(recibidos == -1){
				fprintf(stderr,"Error en la recepcion desde el socket UDP de audio\n");
			}else{/*Si todo fue bien, enviamos lo recibido al pipe*/

				/*Calculamos los datos necesarios para RTCP*/
				/*Primero los paquetes perdidos*/
				perdidos_audio+=(sec-last_sec-1);
				last_sec = sec;

				/*El jitter*/
				gettimeofday(&tiempo_nuevo,NULL);
				retraso = (tiempo_nuevo.tv_sec-tiempo_anterior.tv_sec)*1000000;
				retraso+= (tiempo_nuevo.tv_usec-tiempo_anterior.tv_usec);

				/*Para el jitter usamos pesos de 15/16 y 1/16*/
				jitter_audio = (int)(0.9375*jitter_audio + 0.0625*retraso);

				/*Nos guardamos la marca de tiempo de este paquete*/				
				tiempo_anterior = tiempo_nuevo;

				if(write(fichero_audio[1], buff, recibidos) == -1){
					perdidos_audio++;
					fprintf(stderr,"Error al escribir en el pipe de audio\n");
				}
				

			}
		}else if(estado_audio == Parar){/*Si me han enviado un pause, debo dejar de recibir y bloquearme*/

			pthread_mutex_lock(&sem_audio_cliente);

			/*Actualizamos la marca de tiempo*/
			gettimeofday(&tiempo_anterior,NULL);

		}else if(estado_audio == Terminar){/*Si me mandan terminar, cierro el socket y finalizo*/

			close(sockfd);
			fl = 1;
		}
	}

	pthread_exit(NULL);
}

void *main_video_client(void *v)
{

	int sockfd;
	struct sockaddr_in dir;
	gchar buff[ENV_VIDEO];
	socklen_t tam;
	short sec=0;
	int timestamp;
	int ssrc;
	int recibidos;
	int fl=0;

	/*Variables para el control RTCP*/
	int last_sec=-1;
	struct timeval tiempo_nuevo, tiempo_anterior;
	unsigned int retraso;

	/*Creamos el socket RTP para la recepcion*/
	sockfd = crea_socket_RTP(video_rtp_port);
	if(sockfd == -1){
		fprintf(stderr,"Error creando el socket de escucha para la recepcion del video\n");
	    pthread_exit(NULL);
	}

	gettimeofday(&tiempo_anterior,NULL);

	/*Nos ponemos a leer del socket y enviarlo al pipe*/
	while(fl == 0){

		/*Si tengo que seguir reproduciendo*/
		if(estado_video == Seguir){
			
			/*Sacamos del socket paquetes*/
			recibidos =recibe_paquete_RTP(sockfd, (struct sockaddr *)&dir, &tam, 
					  &sec, &timestamp, &ssrc, (char *)buff, ENV_VIDEO);
			
			if(recibidos == -1){
				fprintf(stderr,"Error en la recepcion desde el socket UDP de video\n");
			}else{/*Si todo fue bien, enviamos lo recibido al pipe*/

				/*Calculamos los datos necesarios para RTCP*/
				/*Primero los paquetes perdidos*/
				perdidos_video+=(sec-last_sec-1);
				/*fprintf(stderr, "Sec y Last Sec: %d %d\n",sec,last_sec);*/
				last_sec = sec;
				
				/*El jitter*/
				gettimeofday(&tiempo_nuevo,NULL);
				retraso = (tiempo_nuevo.tv_sec-tiempo_anterior.tv_sec)*1000000;
				retraso+= (tiempo_nuevo.tv_usec-tiempo_anterior.tv_usec);

				/*Para el jitter usamos pesos de 15/16 y 1/16*/
				jitter_video = (int)(0.9375*jitter_video + 0.0625*retraso);

				/*Nos guardamos la marca de tiempo de este paquete*/				
				tiempo_anterior = tiempo_nuevo;

				if(write(fichero_video[1], buff, recibidos) == -1){
					perdidos_video++;
					fprintf(stderr,"Error al escribir en el pipe de video\n");
				}

			}
		}else if(estado_video == Parar){/*Si me han enviado un pause, debo dejar de recibir y bloquearme*/

			pthread_mutex_lock(&sem_video_cliente);

			/*Actualizamos la marca de tiempo*/
			gettimeofday(&tiempo_anterior,NULL);

		}else if(estado_video == Terminar){/*Si me mandan terminar, cierro el socket y finalizo*/

			close(sockfd);
			fl = 1;
		}
	}

	pthread_exit(NULL);
}

static void *escucha_audio_rtcp(void *nada)
{

	Mensaje_RTCP rec;
	SR *sr_rec;
	struct sockaddr_in dir;
	socklen_t tam;

	/*Nos ponemos a escuchar mensajes RTCP.*/
	while(1){



		tam = sizeof(struct sockaddr_in);

		if(escucha_comandoRTCP(video_rtcp_sockfd, (struct sockaddr *)&dir, &tam, &rec) == -1){/*Si hubo error al recibir el paquete*/
			fprintf(stderr,"Error recibiendo mensaje RTCP\n");
		}else{/*Si se recibio correctamente*/
			
			if(rec.tipo == sr_t){/*Comprobamos que sea un SR*/

				sr_rec = (SR *)&rec;
			}else{

                fprintf(stderr,"Mensaje extranno del servidor\n");
            }
		}
	}

	/*Numca deberia llegar aqui el programa*/
	pthread_exit(NULL);

}

void *envia_audio_rtcp(void *nada)
{

	RR send_rep;
	socklen_t tam;
    struct sockaddr_in dir_audio_rtcp;

	tam = sizeof(struct sockaddr_in);

	dir_audio_rtcp.sin_family=AF_INET;
    dir_audio_rtcp.sin_port = htons(audio_rtcp_port_server);
    dir_audio_rtcp.sin_addr.s_addr = ip_serv.s_addr;

	send_rep.tipo = rr_t;
	send_rep.ssrc_sender = 0;/*Tecnicamente no es este, pero al haber solo una fuente no importa*/
    send_rep.ssrc_source = 0;/*Tecnicamente no es este, pero al haber solo una fuente no importa*/
    send_rep.fr_lost = 0;
    send_rep.extend_sec = 0;

	/*Bucle principal, cada USLEEPRTCP nos despertamos y enviamos un SR*/
	while(1){

		usleep(USLEEPRTCP);

        send_rep.cum_lost = perdidos_audio;
        send_rep.jitter = jitter_audio;

		if(envia_comandoRTCP(audio_rtcp_sockfd, (struct sockaddr *)&dir_audio_rtcp, tam , (Mensaje_RTCP *)&send_rep) == -1){
			fprintf(stderr,"Error al enviar RR audio\n");
		}
	}

	/*Numca deberia llegar aqui el programa*/
	pthread_exit(NULL);

}

static void *escucha_video_rtcp(void *nada)
{

	Mensaje_RTCP rec;
	SR *sr_rec;
	struct sockaddr_in dir;
	socklen_t tam;

	/*Nos ponemos a escuchar mensajes RTCP.*/
	while(1){



		tam = sizeof(struct sockaddr_in);

		if(escucha_comandoRTCP(video_rtcp_sockfd, (struct sockaddr *)&dir, &tam, &rec) == -1){/*Si hubo error al recibir el paquete*/
			fprintf(stderr,"Error recibiendo mensaje RTCP\n");
		}else{/*Si se recibio correctamente*/
			
			if(rec.tipo == sr_t){/*Comprobamos que sea un SR*/

				sr_rec = (SR *)&rec;
			}else{

                fprintf(stderr,"Mensaje extranno del servidor\n");
            }
		}
	}

	/*Numca deberia llegar aqui el programa*/
	pthread_exit(NULL);

}

void *envia_video_rtcp(void *nada)
{

	RR send_rep;
	socklen_t tam;
    struct sockaddr_in dir_video_rtcp;

	tam = sizeof(struct sockaddr_in);

	dir_video_rtcp.sin_family=AF_INET;
    dir_video_rtcp.sin_port = htons(video_rtcp_port_server);
    dir_video_rtcp.sin_addr = ip_serv;

	send_rep.tipo = rr_t;
	send_rep.ssrc_sender = 1;/*Tecnicamente no es este, pero al haber solo una fuente no importa*/
    send_rep.ssrc_source = 2;/*Tecnicamente no es este, pero al haber solo una fuente no importa*/
    send_rep.fr_lost = 3;
    send_rep.extend_sec = 4;

	/*Bucle principal, cada USLEEPRTCP nos despertamos y enviamos un SR*/
	while(1){

		usleep(USLEEPRTCP);

        send_rep.cum_lost = perdidos_video;
        send_rep.jitter = jitter_video;
	
		if(envia_comandoRTCP(video_rtcp_sockfd, (struct sockaddr *)&dir_video_rtcp, tam , (Mensaje_RTCP *)&send_rep) == -1){
			fprintf(stderr,"Error al enviar RR video\n");
		}
	}

	/*Numca deberia llegar aqui el programa*/
	pthread_exit(NULL);

}

