STRIP	= strip
wsdiscovery: main.o soapC.o soapServer.o stdsoap2.o util.o mySoapStub.o porting.o
	$(CC) soapC.o soapServer.o stdsoap2.o main.o util.o mySoapStub.o porting.o -lpthread -o wsdiscovery
	$(STRIP) wsdiscovery
	cp wsdiscovery ws-client
clean:	
	rm *.o;rm wsdiscovery ws-client
