#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
WiFiClient espClient;
PubSubClient client(espClient);
long CurrentTime= millis();
long DisconnectedTime = 0.0000;
const int BAUD_RATE = 115200;
bool PrevConnectionStatus = false;
long WifiReconnectionInterval = 5000000;
bool btn_1_change = true;
bool btn_2_change = true;
const int buttonPin1 = 12;
const int buttonPin2 = 13;
const int relaypin1 = 14;
const int relaypin2 = 25;
long lastMsg = 0;
char msg[50];
int value = 0;
int timer1 = 360;
int Ontime1 = 0;
int buttonPushCounter1 = 0;
int buttonState1 = 0;
int lastButtonState1 = 0;
int timer2 = 360;
int Ontime2=0;
int buttonPushCounter2 = 0;
int buttonState2 = 0;
int lastButtonState2 = 0;
const char
*ssid = "............", //change here to nearby wifi ssid
*password = ".........”, //wifi password
*mqtt_server = "y3d5b6c8.us-east-1.emqx.cloud";
void WIFI_init() {
//This part just used for testing, to connect to wifi
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);
Serial.print("Connecting to WiFi ..");
while (WiFi.status() != WL_CONNECTED) {
Serial.print('.');
delay(1000);
}
Serial.println(WiFi.localIP());

}
void reconnect() {
//Connecting to MQTT broker
Serial.print("Attempting MQTT connection...");
if (client.connect("ESP8266Client", "Vinuja", "Vinuja@199534")) {
Serial.println("connected");
PrevConnectionStatus = true;
// Subscribe
client.subscribe("output1");
client.subscribe("output2");
client.subscribe("Timer1");
client.subscribe("Timer2");
Serial.println("Subscribed");
} else {
Serial.print("failed, rc=");
Serial.print(client.state());
Serial.println(" System Will Try to Reconnect in 5s");
}
}
void Publish(){
//When a button pressed manually, it is publishing to the MQTT broker and
also switching the circuit
if (buttonState1){
if (btn_1_change){
client.publish("switch_1", "ON");
digitalWrite(relaypin1,HIGH);
btn_1_change = !btn_1_change;
}
else{
client.publish("switch_1", "OFF");
digitalWrite(relaypin1,LOW);
btn_1_change = !btn_1_change;
}
buttonState1 = 0;
}
if (buttonState2){
if (btn_2_change){
client.publish("switch_2", "ON");
digitalWrite(relaypin2,HIGH);
btn_2_change = !btn_2_change;
}
else{
client.publish("switch_2", "OFF");
digitalWrite(relaypin2,LOW);
btn_2_change = !btn_2_change;
}
buttonState2 = 0;
}
}
void Wifi_Access(){
//Accessing a wifi network, configuring wifi
WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
// it is a good practice to make sure your code sets wifi mode how you
want it.

// put your setup code here, to run once:
Serial.begin(115200);
//WiFiManager, Local intialization. Once its business is done, there is
no need to keep it around
WiFiManager wm;
// reset settings - wipe stored credentials for testing
// these are stored by the esp library
// wm.resetSettings();
// Automatically connect using saved credentials,
// if connection fails, it starts an access point with the specified name
( "AutoConnectAP"),
// if empty will auto generate SSID, if password is blank it will be
anonymous AP (wm.autoConnect())
// then goes into a blocking loop awaiting configuration and will return
success result
bool res;
// res = wm.autoConnect(); // auto generated AP name from chipid
// res = wm.autoConnect("AutoConnectAP"); // anonymous ap
res = wm.autoConnect("AutoConnectAP","password"); // password protected
ap
if(!res) {
Serial.println("Failed to connect");
//ESP.restart();
}
else {
//if you get here you have connected to the WiFi
Serial.println("connected...yeey :)");
}
}
void callback(String topic, byte* message, unsigned int length) {
// Subscribing data from the MQTT broker is subscribed with respect to the
topic
String messageTemp;
for (int i = 0; i < length; i++) {
//Serial.print((char)message[i]);
messageTemp += (char)message[i];
}
Serial.println();
// Feel free to add more if statements to control more GPIOs with MQTT
if(topic=="output1"){
if(messageTemp == "ON"){
digitalWrite(relaypin1, HIGH);
Serial.println("ON_1");
}
else if(messageTemp == "OFF"){
digitalWrite(relaypin1, LOW);
Serial.println("OFF_1");

}
}
if(topic=="output2"){
if(messageTemp == "ON"){
digitalWrite(relaypin2, HIGH);
Serial.println("ON_2");
}
else if(messageTemp == "OFF"){
digitalWrite(relaypin2, LOW);
Serial.println("OFF_2");
}
}
if(topic == "Timer1"){
Serial.println("I am Here");
timer1=messageTemp.toInt();
Ontime1 = millis();
Serial.println(timer1);
}
if(topic == "Timer2"){
Serial.println("I am Here Bro");
timer2=messageTemp.toInt();
Ontime2 = millis();
Serial.println(timer2);
}
Serial.println();
}
void UPDATE_CONNECTED_DEVICE() {
if (!client.connected() && abs((int)CurrentTime - DisconnectedTime) >
WifiReconnectionInterval)
reconnect();
if (PrevConnectionStatus && !client.connected()) {
DisconnectedTime = CurrentTime;
PrevConnectionStatus = false;
}
}
void CheckTimer(){
//Timer controller
if((millis()-Ontime1)>timer1*60*1000){
client.publish("switch_1","OFF");
digitalWrite(relaypin1,LOW);
Serial.println("1_off");
timer1=360;
}
if((millis()-Ontime2)>timer2*60*1000){
client.publish("switch_2","OFF");
digitalWrite(relaypin2,LOW);
Serial.println("2_off");
timer2=360;
}
}
void IRAM_ATTR btn_1(){
buttonState1 = 1;
}

void IRAM_ATTR btn_2(){
buttonState2 = 1;
}
void setup() {
// put your setup code here, to run once:
Serial.begin(BAUD_RATE);
pinMode(buttonPin1, INPUT_PULLUP);
pinMode(buttonPin2, INPUT_PULLUP);
pinMode(relaypin1, OUTPUT);
pinMode(relaypin2, OUTPUT);
attachInterrupt(buttonPin1, btn_1, RISING);
attachInterrupt(buttonPin2, btn_2, RISING);
Serial.println("Serial Monitor Intialized.... !");
delay(1000);
//WIFI_init();
Wifi_Access();
client.setServer(mqtt_server, 15742);
client.setCallback(callback);
reconnect();
client.publish("switch_1", "OFF");
//Serial.println("HHHH");
client.publish("switch_2", "OFF");
//Serial.println("RCB");
}
void loop() {
// put your main code here, to run repeatedly:
client.setCallback(callback);
client.loop();
client.setServer(mqtt_server, 15742);
UPDATE_CONNECTED_DEVICE();
Publish();
CheckTimer();

}
