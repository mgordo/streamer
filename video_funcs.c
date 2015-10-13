/**
 * @brief Cuerpo de escuchantes de la GUI para video
 * Redes II, Practica II
 * @file video_funcs.c
 * @author Diego González y Miguel Gordo Garcia
 * @date 8-III-12
 * 
*/
#include "video_funcs.h"


/**
 * @brief estado de la reproduccion del cliente
*/
Estado_Main_Cliente estado_main_cliente=No_Conectado;

/**
 *  Realiza la conexión con el servidor                                                             
 *  Parámetro:                                                                                      
 *	- text: url o ip en texto del servidor:Viene con formato server addr server port videofile     
 *  Retorno:                                                                                        
 *	- FALSE: no se ha podido realizar la conexión                                              	   
 *	- TRUE: se ha establecido la conexión                                                          
 *                                                                                                  
*/
static int aleat_num(int inf, int sup);

int videoConnect(const char *text)
{
  
    struct in_addr ip;
    Men_Describe mensaje;
    Resp_Describe respuesta;
    int puerto;
    char *p=NULL;

    if(estado_main_cliente != No_Conectado) return FALSE;

    if(tomaArgumentos((char *)text, &ip, &puerto)==-1){
        return FALSE;
    }

	ip_serv.s_addr = ip.s_addr;

    sockfd_client=crea_conecta_socket_TCP(ip, (short)puerto);
    if(sockfd_client==-1) return FALSE;
    conectado=1;
    mensaje.tipo=Describe;
    mensaje.sec=secuencia;
    strcpy(mensaje.ruta, videofile);

    if(envia_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&mensaje)==-1){
        close(sockfd_client);
        return FALSE;
    }

    secuencia++;
    if(-1==escucha_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&respuesta)){
        close(sockfd_client);
        return FALSE;
    }
    strcpy(audio,respuesta.a1);
    strcpy(video,respuesta.a2);/*copio las rutas de los ficheros de audio y video*/ 

    p=strstr(respuesta.a1," rtsp");
    *p='\0';
    strcpy(audio,p+1);
    p=strstr(respuesta.a2," rtsp");
    *p='\0';
    strcpy(video,p+1);
	printf("ruta audio: %s\n",audio);
	printf("ruta video: %s\n",video);

	estado_main_cliente = Conectado_RTSP;

    return TRUE;
}

/***************************************************************************************************
*  Realiza las funcionalidades de play                                                             *
*  Retorno:                                                                                        *
*	- FALSE: Error                           									                   *
*	- TRUE: Ok								                                                       *
*                                                                                                  *
***************************************************************************************************/

int videoPlay()
{

    if(estado_main_cliente != Setup_Pulsado && estado_main_cliente != Pausado) return FALSE;
  
	Men_Play mensaje;
	Resp_Play respuesta;

	strcpy(mensaje.ruta,videofile);
	mensaje.sec=secuencia;
	secuencia++;
	mensaje.tipo=Play;

	/*Enviamos un play*/
	if(envia_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&mensaje)==-1){
        close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }

	/*esperamos a recibir la respuesta*/
	if(-1==escucha_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&respuesta)){
        close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }

	/*Comprobamos que la respuesta es la esperada*/
	if(respuesta.tipo!=Res_Pause_Play){
		close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }
    printf("Exito hasta inicio transmision\n");
    

	if(estado_main_cliente == Setup_Pulsado){/*Si empezamos a reproducir desde 0*/

		/*Lanzamos los hilos que se encargaran de la escucha y reproduccion del fichero*/
	    if(lanza_recepcion_rtp()==-1){
	    	close(sockfd_client);
	    	estado_main_cliente = No_Conectado;
	    	return FALSE;
	    }

		if(lanza_rtcp()==-1){
			close(sockfd_client);
			close(fichero_audio[0]);
			close(fichero_audio[1]);
    		close(fichero_video[0]);
			close(fichero_video[1]);
			pthread_cancel(id_hilo_audio_rtp);
			pthread_cancel(id_hilo_video_rtp);
			pthread_cancel(id_hilo_control);
			estado_main_cliente = No_Conectado;
			printf("\n\nError al crear los hilos RTCP\n");
			return FALSE;
		}
	}else{/*En el caso de estar pausado y querer reanudar la ejecucion*/
		
		/*Desbloqueamos los hilos*/
		estado_audio=Seguir;
		pthread_mutex_unlock(&sem_audio_cliente);
		estado_video=Seguir;
		pthread_mutex_unlock(&sem_video_cliente);

	}
	/*Ponemos nuestro estado a reproducir normalmente*/
	estado_main_cliente=Rep;
    
	return TRUE;
}

/***************************************************************************************************
*  Realiza las funcionalidades de pause                                                            *
*  Retorno:                                                                                        *
*	- FALSE: Error                           									                   *
*	- TRUE: Ok								                                                       *
*                                                                                                  *
***************************************************************************************************/

int videoPause()
{

	Men_Pause menP;
	Mensaje_RTSP res;

    if(estado_main_cliente != Rep) return FALSE;

	menP.tipo = Pause;
	menP.sec= secuencia;
	secuencia++;
	strcpy(menP.ruta,videofile);

	/*Enviamos el comando*/
	if(envia_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&menP) == -1){
		fprintf(stderr,"Error en el pause\n");
        return FALSE;
    }

	/*Y esperamos respuesta*/
	if(escucha_comandoRTSP(sockfd_client, &res) == -1){
		fprintf(stderr,"Error en el pause\n");
        return FALSE;
    }

	/*Comprobamos que efectivamente es una respuesta de Pause*/
	if(res.tipo != Res_Pause_Play){
		fprintf(stderr,"Error en el pause\n");
        return FALSE;
    }

	/*Detenemos los hilos de recepcion de audio y video*/
	/*Tomamos el semaforo de audio, para bloquear el hilo*/
	pthread_mutex_lock(&sem_audio_cliente);
	estado_audio = Parar;

	/*Tomamos el semaforo de video, para bloquear el hilo*/
	pthread_mutex_lock(&sem_video_cliente);
	estado_video = Parar;

	/*Ponemos nuestro estado a pausado*/
	estado_main_cliente=Pausado;

	return TRUE;
}

/***************************************************************************************************
*  Realiza las funcionalidades de setup                                                            *
*  Retorno:                                                                                        *
*	- FALSE: Error                           									                   *
*	- TRUE: Ok								                                                       *
*                                                                                                  *
***************************************************************************************************/

int videoSetup()
{

    Men_Setup mensaje;
    Resp_Setup respuesta;

    if(estado_main_cliente != Conectado_RTSP) return FALSE;

    mensaje.tipo=Setup;
    mensaje.sec=secuencia;
    strcpy(mensaje.ruta,audio);
    audio_rtp_port=aleat_num(1025,SHRT_MAX);   
    mensaje.rtp_port=audio_rtp_port;
    audio_rtcp_port_client=audio_rtp_port+1;
    mensaje.rtcp_port=audio_rtcp_port_client;
    audio_rtp_port=mensaje.rtp_port;
    video_rtcp_port_client=mensaje.rtcp_port;
    video_rtp_port=aleat_num(1025,SHRT_MAX);
    video_rtcp_port_client=video_rtp_port+1;
    if(envia_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&mensaje)==-1){/*Envio del setup*/
        close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }
	secuencia++;
    if(-1==escucha_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&respuesta)){
        close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }
	if(respuesta.tipo!=Res_Setup){
		close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }

	/*Nos guardamos los puertos que nos da el servidor*/
	audio_rtcp_port_server = respuesta.server_rtcp_port;

	mensaje.tipo=Setup;
    mensaje.sec=secuencia;
    strcpy(mensaje.ruta,video);
    mensaje.rtp_port=video_rtp_port;
    mensaje.rtcp_port=video_rtcp_port_client;
    
    if(envia_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&mensaje)==-1){/*Envio del setup*/
        close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }
	secuencia++;
    if(-1==escucha_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&respuesta)){
        close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }
	if(respuesta.tipo!=Res_Setup){
		close(sockfd_client);
		estado_main_cliente = No_Conectado;
        return FALSE;
    }

	/*Nos guardamos los puertos que nos da el servidor*/
	video_rtcp_port_server = respuesta.server_rtcp_port;

	printf("Setup correcto\n");
    printf("Socket audio rtp: %d\n",audio_rtp_port);
    printf("Socket audio rtcp cliente: %d\n",audio_rtcp_port_client);
    printf("Socket audio rtcp servidor: %d\n",audio_rtcp_port_server);
    printf("Socket video rtp: %d\n",video_rtp_port);
    printf("Socket video rtcp: %d\n",video_rtcp_port_client);
    printf("Socket video rtcp servidor: %d\n",video_rtcp_port_server);

	estado_main_cliente = Setup_Pulsado;

    return TRUE;
}

/***************************************************************************************************
*  Realiza las funcionalidades de teardown                                                         *
*  Retorno:                                                                                        *
*	- FALSE: Error                           									                   *
*	- TRUE: Ok								                                                       *
*                                                                                                  *
***************************************************************************************************/

int videoTeardown()
{
	
	Men_Teardown menT;
	Mensaje_RTSP res;

    if(estado_main_cliente != Rep && estado_main_cliente != Pausado) return FALSE;

	menT.tipo = Teardown;
	menT.sec = secuencia;
	secuencia++;
	strcpy(menT.ruta, videofile);
	
	/*Enviamos el comando*/
	if(envia_comandoRTSP(sockfd_client, (Mensaje_RTSP *)&menT) == -1){
		fprintf(stderr,"Error en el teardown\n");
        return FALSE;
    }

	/*Y esperamos respuesta*/
	if(escucha_comandoRTSP(sockfd_client, &res) == -1){
		fprintf(stderr,"Error en el teardown\n");
        return FALSE;
    }

	/*Comprobamos que efectivamente es una respuesta de Teardown*/
	if(res.tipo != Info){
		fprintf(stderr,"Error en el teardown\n");
        return FALSE;
    }

	/*Cerramos los hilos encargados de la recepcion RTP*/
	estado_audio=Terminar;
	estado_video=Terminar;
    pthread_cancel(id_hilo_control);
    usleep(10000);/*Esperamos a que finalicen*/
	/*Cerramos los "pipes"*/
	close(fichero_audio[0]);
	close(fichero_audio[1]);
    	close(fichero_video[0]);
	close(fichero_video[1]);

	/*Cerramos los recursos asociados a RTCP*/
	/*Primero cancelamos los hilos*/
	pthread_cancel(id_hilo_audio_escucha_rtcp);
	pthread_cancel(id_hilo_video_escucha_rtcp);
	pthread_cancel(id_hilo_audio_envio_rtcp);
	pthread_cancel(id_hilo_video_envio_rtcp);
	/*Y luego cerramos los sockets*/

	close(audio_rtcp_sockfd);
	close(video_rtcp_sockfd);

	/*Cerramos el socket RTSP*/
	close(sockfd_client);

	/*Volvemos a inicializar los mutex*/
	pthread_mutex_init(&sem_audio_cliente,NULL);
	pthread_mutex_init(&sem_video_cliente,NULL);

	estado_main_cliente = No_Conectado;
	exit(0);
	return TRUE;
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



