#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include <ezButton.h>
#define LED 2
#define MOTOR1_PUL 27
//#define MOTOR1_DIR 26
#define MOTOR1_ENA 32
#define HALL_BOTTOM 35
#define HALL_TOP 34
#define ENA 14
#define MOTORS_DIR 33
#define MOTOR0_PUL 25

// Replace with your network credentials
const char* ssid = "Mindo_IOT";
const char* password = "mj21106b";

bool ledState = 0;
const int ledPin = 2;

int SOL_ON = LOW;
int SOL_OFF = HIGH;
int bottom_true_false = 0;
float voltage_top = 0;
float voltage_top_temp = 0; 
float voltage_bottom = 0;
int voltage_bottom_temp = 0;
float val_top = 0;
bool pul_hi_lo = LOW;
bool holdmotor = 0;
float val_bottom = 0;

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

bool motorRun = false;
bool motorRun1 = false;
//bool motorRun_temp = false;
//bool motorRun_man = false;
//bool motorRun_man_temp = false;
bool dir = false;
float dl = 500;
int rpm = 0;
unsigned long start_time;
unsigned long stop_time;
int turn_temp = 0;
int steps_counter = 0;
int selected_motor = 0;
bool deg_based = 0;
String opening_bottom;
String closing_bottom;
String notif_bottom;
String opening_top;
String closing_top;
String notif_top;
bool rotation_based = 0;
float rotation_counter = 0;
float deg_counter = 0;
float deg = 0;
int yes_no_temp = 0;
bool top_high = 0;
bool bottom_high = 0;
int dl2 = dl * 8;
int sol = 27;

//ezButton switchUp(13);  // create ezButton object that attach to pin 7;
//ezButton switchDown(12);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <!-- <p class="state">state: <span id="state">%STATE%</span></p> -->
      <p class="state">RPM: <input value="60" id="rpm" type="text" name="rpm" class="button" maxlength="3"></p>
      <p class="state">Deg: <input value="360" id="deg" type="text" name="deg" class="button"></p>
      <p><button id="rotate_left" class="button">Rotate left</button></p>
      <p><button id="stop" class="button">Stop</button></p>
      <p><button id="rotate_right" class="button">Rotate right</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  var websocketJson = {
    "turn" : 0,
    "turn_left" : 0,
    "rpm" : 60,
    "deg": 360,
  }
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    //document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButtons();
  }
  function initButtons() {
    var rotate_left = document.getElementById('rotate_left')
    var rotate_right = document.getElementById('rotate_right')
    var stop = document.getElementById('stop')
    rotate_left.addEventListener('click', turn);
    rotate_right.addEventListener('click', turn);
    stop.addEventListener('click', turn);
    rotate_left.turn_left = true;
    rotate_left.turn = true;
    rotate_right.turn_left = false;
    rotate_right.turn = true;
    stop.turn = false;
    stop.turn_left = false;
  }
  function turn(rotate){
    websocketJson.rpm = parseInt(document.getElementById('rpm').value, 10);
    websocketJson.deg = parseInt(document.getElementById('deg').value, 10);
    websocketJson.turn_left = rotate.currentTarget.turn_left ? 1 : 0
    websocketJson.turn = rotate.currentTarget.turn ? 1 : 0
//    console.log("websocketJson",websocketJson)
     websocket.send(JSON.stringify(websocketJson))
    // alert(rotate.currentTarget.myParam);
  }
</script>
</body>
</html>
)rawliteral";

String notifyClients(String notif) {
//  char buffer[20];
//  sprintf(buffer, "{\"rotation\":\"%s\"}", rotation_counter);
  ws.textAll(notif);
//  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
//  StaticJsonBuffer<500> JSONBuffer;
//  JsonObject& parsed = JSONBuffer.parseObject((char*)data);
//  const char * dl = parsed["delay"];
//
//  if (!parsed.success()) {   //Check for errors in parsing
// 
//    Serial.println("Parsing failed");
//    return;
// 
//  }
//  notifyClients(dl);
  StaticJsonBuffer<500> JSONBuffer;                     //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject((char*)data);
  const char * rpms = parsed["rpm"];
  int turn = parsed["turn"];
  int motor = parsed["motor"];
  selected_motor = motor;
  int turn_left = parsed["turn_left"];
  int degs = parsed["deg"];
  holdmotor = parsed["holdmotor"];
  rpm = atoi(rpms);
  motorRun = turn;
  if(rpm > 0){
    
    dir = turn_left;
    dl = ((float)60/rpm)*1250;
    if(degs > 0){
      deg_based = 1;  
    }
    else{
      deg_based = 0;  
    }
    deg = degs*4;
  }

  
  
  
//  notifyClients();
  
//  AwsFrameInfo *info = (AwsFrameInfo*)arg;
//  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
//    data[len] = 0;
//    if (strcmp((char*)data, "toggle") == 0) {
//      ledState = !ledState;
//      notifyClients();
//    }
//  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
//  switchUp.setDebounceTime(50); // set debounce time to 50 milliseconds
//  switchDown.setDebounceTime(50);
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  pinMode(HALL_BOTTOM, INPUT); //assigning the input port
  pinMode(HALL_TOP, INPUT); //assigning the input port
  pinMode(MOTOR0_PUL, OUTPUT);
  pinMode(ENA,OUTPUT);
  pinMode(MOTOR1_ENA,OUTPUT);
  digitalWrite(MOTOR1_ENA,HIGH);
  pinMode(MOTOR1_PUL,OUTPUT);
  digitalWrite(MOTOR1_PUL,LOW);
  pinMode(sol,OUTPUT);
  digitalWrite(MOTOR0_PUL, LOW);
  pinMode(MOTORS_DIR, OUTPUT);
  digitalWrite(ENA,SOL_OFF);
  digitalWrite(sol,SOL_OFF);
  digitalWrite(MOTORS_DIR, LOW);

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  AsyncElegantOTA.begin(&server); 
  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  val_bottom = analogRead(HALL_BOTTOM);
  voltage_bottom = (val_bottom * 3.3 ) / (4095);
  val_top = analogRead(HALL_TOP);
  voltage_top = (val_top * 3.3 ) / (4095);
  if(millis()%100 == 0){
      if(voltage_bottom < 2.9){ //bright
        if(!bottom_high){
          opening_bottom = String("{\"bottom_sensor\":true,\"voltage_bottom\":");
          closing_bottom = String("}");
          notif_bottom = String();
          notif_bottom = opening_bottom+String(voltage_bottom)+closing_bottom;
          notifyClients(notif_bottom);
          bottom_high = 1;
        }
      }
      else{ //dark
        if(bottom_high){
          opening_bottom = String("{\"bottom_sensor\":false,\"voltage_bottom\":");
          closing_bottom = String("}");
          notif_bottom = String();
          notif_bottom = opening_bottom+String(voltage_bottom)+closing_bottom;
          notifyClients(notif_bottom);
          bottom_high = 0;
        }
      }
      if(voltage_top < 2.9){
        if(!top_high){
          opening_top = String("{\"top_sensor\":true,\"voltage_top\":");
          closing_top = String("}");
          notif_top = String();
          notif_top = opening_top+String(voltage_top)+closing_top;
          notifyClients(notif_top);
          top_high = 1;
        }
      }
      else{
        if(top_high){
          opening_top = String("{\"top_sensor\":false,\"voltage_top\":");
          closing_top = String("}");
          notif_top = String();
          notif_top = opening_top+String(voltage_top)+closing_top;
          notifyClients(notif_top);
          top_high = 0;
        }
      }
  }
  

//  switchUp.loop(); // MUST call the loop() function first
//  switchDown.loop(); // MUST call the loop() function first
//  int swUp = switchUp.getState();
//  int swDown = switchDown.getState();
    
  if(motorRun){
    if(!turn_temp){
      start_time = millis();
      if(!selected_motor){
        digitalWrite(ENA,SOL_ON);
      }
      else{
        digitalWrite(MOTOR1_ENA,SOL_ON);
      }
      delay(1);
      turn_temp = motorRun;
    }
    
//    delayMicroseconds(1000000);
    digitalWrite(MOTORS_DIR,dir);
    pul_hi_lo = !pul_hi_lo;
    if(!selected_motor){
      digitalWrite(MOTOR0_PUL, pul_hi_lo);
    }
    else{
      digitalWrite(MOTOR1_PUL, pul_hi_lo);
    }
    deg_counter = deg_counter + 1.8;
    delayMicroseconds(dl);
    steps_counter = steps_counter + 1;
    if(deg_based){
      if(deg_counter >= deg){
        motorRun = false;
      }
    }
    
//    rotation_counter = rotation_counter + ((float)360/deg_counter);
  } 
  else{
     if(turn_temp){
      stop_time = millis();
      turn_temp = motorRun;
      String opening = String("{\"degress\" : ");
      String closing = String("}");
      String notif = String();
      notif = opening+String(float ((steps_counter)/4)*1.8)+closing;
      notifyClients(notif);
    }
    deg_counter = 0;
    steps_counter = 0;
    digitalWrite(MOTOR0_PUL, LOW);
    digitalWrite(MOTOR1_PUL, LOW);
//    digitalWrite(33, LOW);
    if(!holdmotor){
      delay(1);
      digitalWrite(ENA,SOL_OFF);
      digitalWrite(MOTOR1_ENA,SOL_OFF);
    }
    else{
      delay(1);
      digitalWrite(ENA,SOL_ON);
      digitalWrite(MOTOR1_ENA,SOL_ON);
    }
//    digitalWrite(sol,SOL_OFF);
//    digitalWrite(LED,LOW);
  }
}
