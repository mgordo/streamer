################################################################################
# Makefile con auto-deteccion de dependencias: nunca querras usar otro
# (de tipo .h; las de linkado hay que enumerarlas)
# Ver http://www.cs.washington.edu/orgs/acm/tutorials/dev-in-unix/makefiles.html
################################################################################

# banderas de compilacion (PUEDE SER UTIL CAMBIARLAS)
CC = gcc

CFLAGS = -Wall -g -ansi -DUSE_ETHD -I . -D_GNU_SOURCE
LDLIBS = -lm -lpthread -pthread

# fuentes a considerar (si no se cambia, todos los '.c' del directorio actual)
SOURCES = $(shell ls -1 *.c* | xargs)

# busca los ejecutables (todos los .c con metodo tipo 'int main')
EXEC_SOURCES = $(shell grep -l "^int main" $(SOURCES) | xargs)

# fich. de dependencia (.d) y nombres de ejecutables (los anteriores, sin '.c')
EXECS = servidor video
#$(shell echo $(EXEC_SOURCES) | sed -e 's:\.c[p]*::g')
DEPS = $(shell echo $(SOURCES) | sed -e 's:\.c[p]*:\.d:g')

all:	servidor video

# las dependencias (CAMBIA PARA CADA PRACTICA)
servidor:	servidor.o socket_funcs.o RTSP.o RTP.o RTCP.o audio_serv.o video_serv.o
video:	video.o socket_funcs.o RTSP.o RTP.o RTCP.o cliente.o video_funcs.o 

# objetivos adicionales, para generar la documentacion
doc:
	doxygen Doxyfile

# receta para hacer un .d (dependencias automaticas de tipo .h para tus .o)
%.d : %.c
	@set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@

#incluye las dependencias generadas
-include $(DEPS)

# receta para hacer un .o
%.o :	%.c
	@echo -n compilando objeto \'$<\'...
	@$(CC) $(CFLAGS) $< -c `pkg-config --cflags gtk+-2.0 gstreamer-0.10 gstreamer-plugins-base-0.10 gstreamer-interfaces-0.10`
	@echo [OK]

# receta para hacer un ejecutable (suponiendo resto de dependencias OK)
% :	%.o
	@echo -n compilando ejecutable \'$@\'...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) `pkg-config --libs gtk+-2.0 gstreamer-0.10 gstreamer-plugins-base-0.10 gstreamer-interfaces-0.10`
	@echo [OK]

# limpieza, el segundo comando borra la documentacion generada por doxygen
clean:	
	@rm -f $(wildcard *.o *.d core* *.P) $(EXECS)
	@rm -rf html latex

# ayuda (nunca viene mal)
help:
	@echo "Use: make <target> ..."
	@echo "Valid targets:"
	@$(MAKE) --print-data-base --question | sed -e "s:makefile::g" |\
	awk '/^[^.%][-A-Za-z0-9_]*:/	\
		{ print "   " substr($$1, 1, length($$1)-1) }'
