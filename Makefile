STRIP	= strip
wsdiscovery: main.o soapC.o soapServer.o stdsoap2.o 
	$(CC) soapC.o soapServer.o stdsoap2.o main.o  -lpthread -o wsdiscovery
	$(STRIP) wsdiscovery
	cp wsdiscovery ws-client
clean:	
	rm *.o;rm wsdiscovery ws-client
