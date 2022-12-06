# Embedded-System-Lab-Hw4

python3 -m serial.tools.list_ports -v      to check port 

to generate shim code
  ~/Downloads/erpcgen led-service.erpc

to run lab 4.1
  cd ~/"Mbed Programs"/HW4.1/
  python3 client.py /dev/cu.usbserial-AC00CJUO

to run lab 4.2
  python3 mqtt_client.py        // to run python run in /Mbed Programs/hw4.2/ 
  /usr/local/opt/mosquitto/sbin/mosquitto -c ~/Mbed\ Programs/mosquitto.conf         //To run mosquitto ( run in from the root directory)
  
