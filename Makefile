all:
#	gcc main.c soapC.c soapClient.c duration.c -L/usr/lib/x86_64-linux-gnu/ -lgsoap -o wsdiscovery
#	gcc main.c soapC.c soapClient.c -o wsdiscovery
#	gcc main.c soapC.c soapClient.c duration.c stdsoap2.c -DDEBUG -o wsdiscovery
#	gcc main.c onviflib.c soapC.c soapClient.c duration.c stdsoap2.c -o wsdiscovery

	gcc -Wall -g -c soapC.c -o soapC.o
	gcc -Wall -g -c soapClient.c -o soapClient.o
	gcc -Wall -g -c duration.c -o duration.o
	gcc -Wall -g -c stdsoap2.c -o stdsoap2.o
	gcc -Wall -g -c onviflib.c -o onviflib.o
	ar ruv libonvif.a soapC.o soapClient.o duration.o stdsoap2.o onviflib.o
	ranlib libonvif.a 
	gcc -Wall -g main.c -I../IPCameraController -L../IPCameraController -lonvif -o wsdiscovery


