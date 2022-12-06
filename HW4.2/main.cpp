#include "mbed.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

#include "accelerometer.h"
#include "gyro.h"

// GLOBAL VARIABLES
WiFiInterface *wifi;
InterruptIn btn2(BUTTON1);
//InterruptIn btn3(SW3);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;

AnalogOut Aout(PA_4);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;
Accelerometer acc;
Gyro gyro;
double Accel[3]={0};
double Gyro[3]={0};
double  accAngleX=0; 
double  accAngleY=0;
double elapsedTime=0;
double roll, pitch, yaw;
double gyroAngleX=0;
double gyroAngleY=0;
int counter=0;
int idR[32] = {0};
int indexR = 0;

const char* topic = "Mbed";


Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;


void record(void) {

  acc.GetAcceleromterSensor(Accel);
  acc.GetAcceleromterCalibratedData(Accel);
  accAngleX = (atan(Accel[1] / sqrt(Accel[0]*Accel[1] + Accel[2]*Accel[2])) * 180 / SENSOR_PI_DOUBLE); 
  accAngleY = (atan(-1 * Accel[1] / sqrt(Accel[1]*Accel[1] + Accel[2]*Accel[2])) * 180 / SENSOR_PI_DOUBLE); 
  gyro.GetGyroSensor(Gyro);
  gyro.GetGyroCalibratedData(Gyro);

  elapsedTime=0.1; //100ms by thread sleep time

  gyroAngleX = gyroAngleX + Gyro[0] * elapsedTime; 
  gyroAngleY = gyroAngleY + Gyro[1] * elapsedTime;
  yaw =  yaw + Gyro[2] * elapsedTime;
  roll = accAngleX;
  pitch = accAngleY;
}

void startRecord(void) {
  idR[indexR++] = queue.call_every(100ms, record);
  indexR = indexR % 32;
}

void stopRecord(void) {
  for (auto &i : idR)
    queue.cancel(i);
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    double r = 0.0;   // not important since doesn't print any way but baisclly set if degree less than 10 r , p , y = 0.0
    double p = 0.0;
    double y = 0.0;

    if (roll >= 10 || pitch > 10 || yaw > 10){
        r = roll;
        p = pitch;
        y = yaw;
    } 

    message_num++;
    MQTT::Message message;
    char buff[100];
    char hi[100];
    queue.event(stopRecord);
    if (roll >= 10 || pitch > 10 || yaw > 10){
        r = roll;
        p = pitch;
        y = yaw;
        sprintf(buff, "%f/%f/%f\n", r, p, y);
        printf("Puslish message: %s\r\n", buff);
    } 
    else {
        sprintf(hi, "the degree is less than 10 \r\n");
        printf(hi);
    }
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    int rc = client->publish(topic, message);
}

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char msg[300];
    //sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);
    //printf(msg);
    ThisThread::sleep_for(2000ms);
    char payload[300];
    if (roll >= 10 || pitch > 10 || yaw > 10){
        sprintf(payload, "python reply %.*s\r\n", message.payloadlen, (char*)message.payload);
        printf(payload);
    } 
    ++arrivedcount;
}

void close_mqtt() {
    closed = true;
}

int main() {

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
            printf("ERROR: No WiFiInterface found.\r\n");
            return -1;
    }

    printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
            printf("\nConnection error: %d\r\n", ret);
            return -1;
    }

    NetworkInterface* net = wifi;
    MQTTNetwork mqttNetwork(net);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    //TODO: revise host to your IP
    //const char* host = "192.168.0.13";
    const char* host = "172.20.10.2";
   
    const int port=1883;
    printf("Connecting to TCP network...\r\n");
    printf("address is %s/%d\r\n", host, port);

    int rc = mqttNetwork.connect(host, port);//(host, 1883);
    if (rc != 0) {
            printf("Connection error.");
            return -1;
    }
    printf("Successfully connected!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";

    if ((rc = client.connect(data)) != 0){
            printf("Fail to connect MQTT\r\n");
    }
    if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
            printf("Fail to subscribe\r\n");
    }

    t.start(callback(&queue, &EventQueue::dispatch_forever));
    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    btn2.rise(mqtt_queue.event(&publish_message, &client));
    //btn3.rise(&close_mqtt);

    btn2.fall(queue.event(startRecord));
    //btn2.rise(queue.event(stopRecord));

    int num = 0;
    while (num != 0) {
            client.yield(100);
            ++num;
    }

    while (1) {
            if (closed) break;
            client.yield(500);
            ThisThread::sleep_for(1500ms);
    }

    printf("Ready to close MQTT Network......\n");

    if ((rc = client.unsubscribe(topic)) != 0) {
            printf("Failed: rc from unsubscribe was %d\n", rc);
    }
    if ((rc = client.disconnect()) != 0) {
    printf("Failed: rc from disconnect was %d\n", rc);
    }

    mqttNetwork.disconnect();
    printf("Successfully closed!\n");

    return 0;
}


//     python3 mqtt_client.py


