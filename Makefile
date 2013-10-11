CC = gcc
STRIP	= strip
wsdiscovery: main.o soapC.o soapServer.o stdsoap2.o util.o mySoapStub.o porting.o
	$(CC) soapC.o soapServer.o stdsoap2.o main.o util.o mySoapStub.o porting.o -lpthread -o wsdiscovery
	$(STRIP) wsdiscovery
	cp wsdiscovery ws-client
	
main.o:	main.c
	$(CC) -c -Wall main.c 
		
util.o:	util.c
	$(CC) -c -Wall util.c 

porting.o:porting.c
	$(CC) -c -Wall porting.c 

clean:	
	rm *.o;rm wsdiscovery ws-client
