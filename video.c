/**
 * @brief Interfaz para el cliente RTSP
 * 
 * Interfaz simple para RTSP que permite toda la interacción mínima. Dispone de
 *    un campo de introducción de la url del servidor, un botón de conexión, una ventana de video
 *    para la salida de los frames recibidos y una serie de botones que realizan todas las
 *	 funcionalidades del protocolo.
 * 
 *    Todos los callbacks están completamente resueltos salvo los que tiene que resolver el alumno
 *    que son los referentes a los mensajes RTSP y el de conexión. Los callbacks llaman a funciones
 *    disponibles en video_funcs.c
 *
 * Redes II, Practica II
 * @file video.c
 * @author Miguel Gordo Garcia
 * @date 27-II-2012 
 * 
*/

#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gst/gst.h>
#include <gdk/gdkx.h>
#include "video_funcs.h"

/* Ventanas y diálogos */
GtkWidget *window;
GtkWidget *playerror,*pauseerror,*setuperror,*teardownerror, *connecterror;
GstElement *play;
/* Contenedores */
GtkWidget *vbox;
GtkWidget *table1;
GtkWidget *table2;


/* Widget activos */
GtkWidget *connectButton, *playButton, *pauseButton, *teardownButton, *setupButton,*openButton;
GtkWidget *label1;
GtkWidget *view;
GtkWidget *connectionTextBox;
/* Manejo de las entradas en la ventana de texto */
GtkTextBuffer *buffer;
GtkTextIter start, end;
GtkTextIter iter;

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void connectWidget(GtkWidget *widget, gpointer data)
{
  const char *text;
  text = gtk_entry_get_text((GtkEntry *)connectionTextBox);
 
  if(videoConnect(text)==FALSE) /* Presenta un diálogo de error */
  {
    gtk_dialog_run(GTK_DIALOG(connecterror));
    gtk_widget_hide (connecterror);
  }
}

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void playWidget(GtkWidget *widget, gpointer data)
{
  if(videoPlay()==FALSE) /* Presenta un diálogo de error */
  {
    gtk_dialog_run(GTK_DIALOG(playerror));
    gtk_widget_hide (playerror);
  }
}

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void pauseWidget(GtkWidget *widget, gpointer data)
{
  const char *text;
  text = gtk_entry_get_text((GtkEntry *)connectionTextBox);
 
  if(videoPause()==FALSE) /* Presenta un diálogo de error */
  {
    gtk_dialog_run(GTK_DIALOG(pauseerror));
    gtk_widget_hide (pauseerror);
  }
}

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void setupWidget(GtkWidget *widget, gpointer data)
{
  const char *text;
  text = gtk_entry_get_text((GtkEntry *)connectionTextBox);
 
  if(videoSetup()==FALSE) /* Presenta un diálogo de error */
  {
    gtk_dialog_run(GTK_DIALOG(setuperror));
    gtk_widget_hide (setuperror);
  }
}

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void teardownWidget(GtkWidget *widget, gpointer data)
{
 
  if(videoTeardown()==FALSE) /* Presenta un diálogo de error */
  {
    gtk_dialog_run(GTK_DIALOG(teardownerror));
    gtk_widget_hide (teardownerror);
  }
}

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

int play_func(gchar *addr) {
  GMainLoop *loop;

  loop = g_main_loop_new(NULL, FALSE);

  play = gst_element_factory_make("playbin", "play");
  g_object_set(G_OBJECT(play), "uri", addr, NULL);

  GstElement* x_overlay=gst_element_factory_make ("autovideosink", "videosink");
  g_object_set(G_OBJECT(play),"video-sink",x_overlay,NULL);
  gst_element_set_state(play, GST_STATE_PLAYING);

  g_main_loop_run(loop);

  gst_object_unref(GST_OBJECT(play));

  return 0;
}

/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void fileChooser(GtkButton *button, GtkWindow *mainwindow) {
  GtkWidget *filechooser;
  gchar *uri;

  filechooser = gtk_file_chooser_dialog_new("Open File...", mainwindow, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filechooser), FALSE);

  gint response = gtk_dialog_run(GTK_DIALOG(filechooser));

  if (response == GTK_RESPONSE_OK) {
    uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filechooser));
    gtk_widget_destroy(filechooser);
    play_func(uri);
    g_free(uri);
  }
  else if (response == GTK_RESPONSE_CANCEL) {
    gtk_widget_destroy(filechooser);
  }
}



/*******************************************************************************
*  Realiza la atención al botón de conexión                                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

int main( int argc, char *argv[])
{
  
  gtk_init(&argc, &argv); /* Inicia gnome */
  gst_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL); /* Ventana principal */
  srand(time(NULL));
  /*Se establecen los comandos de error*/
  connecterror = gtk_message_dialog_new (GTK_WINDOW(window), /* Diálogo error conexión */
         GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "Connecting error");
  playerror = gtk_message_dialog_new (GTK_WINDOW(window), /* Diálogo error conexión */
         GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "Play error");
  pauseerror = gtk_message_dialog_new (GTK_WINDOW(window), /* Diálogo error conexión */
         GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "Pause error");
  setuperror = gtk_message_dialog_new (GTK_WINDOW(window), /* Diálogo error conexión */
         GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "Setup error");
  teardownerror = gtk_message_dialog_new (GTK_WINDOW(window), /* Diálogo error conexión */
         GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "Teardown error");

  /*Se genera la ventana principal*/
  
  gtk_window_set_title(GTK_WINDOW(window), "RTSP Video Streaming"); /* Título ventana principal */
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 100); /* Tamaño ventana principal */
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); /* Posición ventana principal */

  /* Creación de los distintos Widgets con algunas de sus características */
  label1 = gtk_label_new("Server");
  vbox = gtk_vbox_new(FALSE,5);
  table1 = gtk_table_new(1, 3, FALSE);
  table2 = gtk_table_new(1, 6, FALSE);
  connectionTextBox = gtk_entry_new(); 

  
  view = gtk_text_view_new();  
  connectButton = gtk_button_new_with_label("Connect");
  playButton = gtk_button_new_with_label("Play");
  teardownButton = gtk_button_new_with_label("Teardown");
  pauseButton = gtk_button_new_with_label("Pause");
  setupButton = gtk_button_new_with_label("Setup");
  openButton = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(openButton), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_SMALL_TOOLBAR));
  g_signal_connect(G_OBJECT(openButton), "clicked", G_CALLBACK(fileChooser), (gpointer) window);


  /* Estructura de los Widgets */
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_box_pack_start(GTK_BOX(vbox), table1,FALSE,FALSE,1);
  gtk_box_pack_start(GTK_BOX(vbox), table2,FALSE,FALSE,1);

  gtk_table_attach(GTK_TABLE(table1), label1, 0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table1), connectionTextBox, 1, 2, 0, 1, GTK_FILL| GTK_EXPAND, GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table1), connectButton, 2, 3, 0, 1, GTK_FILL| GTK_SHRINK,GTK_SHRINK, 5, 5);

  gtk_table_attach(GTK_TABLE(table2), playButton, 1, 2, 2, 3, GTK_FILL| GTK_SHRINK, GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table2), pauseButton, 2, 3, 2, 3, GTK_FILL| GTK_SHRINK, GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table2), setupButton, 3, 4, 2, 3, GTK_FILL| GTK_SHRINK, GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table2), teardownButton, 4, 5, 2, 3, GTK_FILL| GTK_SHRINK, GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table2), openButton, 5, 6, 2, 3, GTK_FILL| GTK_SHRINK, GTK_SHRINK, 5, 5);

  gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view),GTK_WRAP_WORD_CHAR);

  /* Conexión de las señales a sus correspondientes funciones */

  g_signal_connect(G_OBJECT(connectButton), "clicked", G_CALLBACK(connectWidget), NULL);
  g_signal_connect(G_OBJECT(playButton), "clicked", G_CALLBACK(playWidget), NULL);
  g_signal_connect(G_OBJECT(pauseButton), "clicked", G_CALLBACK(pauseWidget), NULL);
  g_signal_connect(G_OBJECT(setupButton), "clicked", G_CALLBACK(setupWidget), NULL);
  g_signal_connect(G_OBJECT(teardownButton), "clicked", G_CALLBACK(teardownWidget), NULL);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(window); /* Presentación de las ventanas */

  gtk_main(); /* Administración de la interacción */

  return 0;
}
