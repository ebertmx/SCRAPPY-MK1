#include <WiFi.h>



void startup(void *pvParameters);            //setup stuff, connect to server, calibrate
void manualControl(void *pvParameters);      //task to be activated for manual control mode
void handleInstruction(void *pvParameters);  //interrupt enabled task that activates on message recieved and
void motorControl(void *pvParameters);

const char ssid[] = "FREE_WIFI!!!";  // whatever one we end up going with
const char pass[] = "WarzoneIsLife1";       // password

const char host[] = "192.168.1.90";
const uint16_t port = 9999;

const MAX_CMD_LENGTH = 64;
const MAX_ARG_LENGTH = 16;



void setup() {
  Serial.begin(115200);
  Serial.println("Serial works");
    printf("Starting\n");
    xTaskCreatePinnedToCore(
      startup,
      "Startup",
      5000,
      NULL,
      2,
      NULL,
      0
    );
}

void loop() {

}


void startup(void *pvParameters) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  printf("Startup has begun\n");
  (void)pvParameters;
  
  for (int i = 0; i < WiFi.scanNetworks(); i++){
    Serial.println(WiFi.SSID(i));
  }
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    printf("...\n");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  

  WiFiClient client;
  int attempt = 1;

  while(!client.connect(host, port)) {
    printf("Connection to host failed\n");
    printf("Attempt number: ");
    printf("%d", attempt);
    printf("\n");
    attempt++;
    delay(1000);
  }
  Serial.println("Connected to server successfully");
  delay(1000);
  client.write("Test data");
  int bytes;
  char command[MAX_CMD_LENGTH];
  while(client.connected()) {
    bytes = client.available();
    if (bytes) {
      for (int i = 0; i < bytes; i++) {\
        command[i] = (char)client.read()
      }
      command[bytes] = '\0';
      Serial.println(command);
      clearString(command, bytes);
    }
    
    delay(100);
  }

}

void parseCommand(char* command, int len, char** args){
  int args = 0;
  int curArg = 0;
  int argIndex = 0;
  char argArray[MAX_ARG_LENGTH];
  char c;
  for(int i = 0; i < len; i++){
    c = command[i];
    if(c == ":"){
      command[i] = '\0'
      args = 1;
    }
    if(args){
      if(c == ","){
        curArg++;
        argIndex = 0;
        continue;
      }
      argArray[curArg][argIndex++] = c;
    }
  }
}

void clearString(char* string, int len){
  for (int i = 0; i < len; i++){
    string[i] = '\0';
  }
}

void handleInstruction(void *pvParameters) {
  char args[][];
  command = (char*)pvParameters;
}


