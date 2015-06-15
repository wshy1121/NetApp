############################################################################
#
# Makefile for DH-DVR_ARM2510
#
# arm-uclinux-elf-gcc version 3.4.3
#
############################################################################
#CROSS  = sh4-linux-uclibc-
#CROSS  = arm-none-linux-gnueabi-
#CROSS  = arm-linux-gnueabihf-

CXXFLAGS += -std=c++0x -I../CrossPlat  -I../LogCore  -I../Encrypt  -I../TraceWorker  -I../NetApp
LIB_OBJS += net_client.o net_server.o  data_work.o  data_handle.o  trace_handel.o

CPP	=	@echo " g++ $@"; $(CROSS)g++
CC	=	@echo " gcc $@"; $(CROSS)gcc
LD	=	@echo " ld  $@"; $(CROSS)ld
AR  = @echo " ar  $@"; $(CROSS)ar
STRIP	=	@echo " strip $@"; $(CROSS)strip
RANLIB = @echo " ranlib  $@"; $(CROSS)ranlib

CP	= cp -rf
RM	= rm

AFLAGS	+= -r   


LIB_TARGET=../CosApp/lib/libNetApp.a

all	:	$(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^
	$(RANLIB) $@

.c.o:
	$(CC) -c $(CFLAGS) $^ -o $@

.cpp.o:
	$(CPP) -c -Wall $(CXXFLAGS) $^ -o $@

clean:
	$(RM) $(LIB_OBJS)

install:
	cp $(LIB_TARGET) ../Build/Libs/libNetApp.a 


	
