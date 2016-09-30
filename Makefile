

LIBS=-Ljpeg-6b/
INCLUDES=-Ijpeg-6b/


nef2jpg: nef2jpg.o
	$(CC) -o nef2jpg nef2jpg.o jpegimage.o jpeg-6b/libjpeg.a 

nef2jpg.o: jpegimage.o nef2jpg.c
	$(CC) $(INCLUDES) -o nef2jpg.o -c nef2jpg.c

jpegimage.o: jpegimage.c jpegimage.h
	$(CC) $(INCLUDES) -o jpegimage.o -c jpegimage.c

clean:
	rm nef2jpg *.o

