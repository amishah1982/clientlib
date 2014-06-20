all:
#	gcc main.c soapC.c soapClient.c duration.c -L/usr/lib/x86_64-linux-gnu/ -lgsoap -o wsdiscovery
#	gcc main.c soapC.c soapClient.c -o wsdiscovery
#	gcc main.c soapC.c soapClient.c duration.c stdsoap2.c -DDEBUG -o wsdiscovery
	gcc main.c soapC.c soapClient.c duration.c stdsoap2.c -o wsdiscovery
