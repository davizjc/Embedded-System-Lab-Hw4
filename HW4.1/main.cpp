#include "mbed.h"
#include "TextLCD.h"
I2C i2c_lcd(D14, D15); // SDA, SCL
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2);
// main() runs in its own thread in the OS

/**
 * Macros for setting console flow control.
 */
#define CONSOLE_FLOWCONTROL_RTS     1
#define CONSOLE_FLOWCONTROL_CTS     2
#define CONSOLE_FLOWCONTROL_RTSCTS  3
#define mbed_console_concat_(x) CONSOLE_FLOWCONTROL_##x
#define mbed_console_concat(x) mbed_console_concat_(x)
#define CONSOLE_FLOWCONTROL mbed_console_concat(MBED_CONF_TARGET_CONSOLE_UART_FLOW_CONTROL)


#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "HW_server.h"


void location(uint8_t col,uint8_t row) {
    lcd.locate(col,row); 
    printf("LCD locate %d , %d\n",col,row);
}

 int intToAscii(int number) {
   return '0' + number;
}

void printtext(uint8_t c) {
    // lcd.putc(0x48);           // ‘H’
    // lcd.putc(0x45);           // ‘E’
    // lcd.putc(0x4C);           // ‘L’
    // lcd.putc(0x4C);           // ‘L’     
    // lcd.putc(0x4F);           // ‘O’  
    
    lcd.putc(intToAscii(c));      // display numbers 0-9
      
  
    printf("LCD print %d\n", c );
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

/** LED service */
LEDBlinkService_service led_service;

int main(void) {
	// Initialize the rpc server
	uart_transport.setCrc16(&crc16);

	// Set up hardware flow control, if needed
#if CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTS
	uart_transport.set_flow_control(mbed::SerialBase::RTS, STDIO_UART_RTS, NC);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_CTS
	uart_transport.set_flow_control(mbed::SerialBase::CTS, NC, STDIO_UART_CTS);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTSCTS
	uart_transport.set_flow_control(mbed::SerialBase::RTSCTS, STDIO_UART_RTS, STDIO_UART_CTS);
#endif
	
    printf("Initializing server.\n");
	rpc_server.setTransport(&uart_transport);
	rpc_server.setCodecFactory(&basic_cf);
	rpc_server.setMessageBufferFactory(&dynamic_mbf);

    printf("Adding server.\n");
	rpc_server.addService(&led_service);

	// Run the server. This should never exit
    printf("Running server.\n");
	rpc_server.run();
}




// python3 -m serial.tools.list_ports -v      to check port 
// cd ~/"Mbed Programs"/HW4.1/

// /dev/cu.usbserial-AC00CJUO
// python3 led_test_client.py /dev/cu.usbserial-AC00CJUO


