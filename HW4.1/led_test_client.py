from time import sleep
import erpc
from HW import *
import sys


if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Usage: python client.py <serial port to use>")
        exit()

    # Initialize all erpc infrastructure
    xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
    client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
    client = client.LEDBlinkServiceClient(client_mgr)

    # Blink LEDs on the connected erpc server
    
    client.location(3,1)
    print("LCD locate") 

    for i in range(0,9):
            client.printtext(i)
            print("LCD print",i) 

        
            
        
  

    
 


  





  


