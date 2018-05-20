OBJ = *.o
OBJ_1450 = src-1450/*.o
TARGET = rage

$(TARGET): $(OBJ) $(OBJ_1450)
	cc $(OBJ) -lc -lm -lpthread -lrt -Lsource -lnet -o $(TARGET)

$(OBJ):
	cc -c abus.c
	cc -c data.c
	cc -c eth.c
	cc -c event.c
	cc -c io.c
	cc -c list.c
	cc -c lstack.c
	cc -c mkio.c
	cc -c queue.c
	cc -c rage.c
	cc -c rc.c
	cc -c serial.c
	cc -c system.c

$(OBJ_1450):
	cc -c src-1450/baselib.c
	cc -c src-1450/heater.c
	cc -c src-1450/id.c
	cc -c src-1450/reg.c
	cc -c src-1450/tech.c

install:
	cp *.desktop /root/Desktop
	cp *.desktop /root/elk/startmenu/applications/autostart

clean:
	rm $(OBJ)
	rm $(TARGET)