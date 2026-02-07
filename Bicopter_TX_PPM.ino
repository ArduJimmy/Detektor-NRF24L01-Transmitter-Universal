#include <SPI.h>
#include <nRF24L01.h>  
#include <RF24.h>

#define LED_PIN     9
#define BUZZER_PIN  7

const uint64_t pipeOut = 0xE8E8F0F0E1LL; //IMPORTANT: The same as in the receiver!!!
RF24 radio(4, 3); // CE, CSN

unsigned long lastSendTime = 0; // Track last data send time
const unsigned long sendInterval = 10; // Send data every 10ms
bool buzzerPlayed = false;

struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
};
MyData data;

void resetData() {
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
}

void beep2x() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(80);
    digitalWrite(BUZZER_PIN, LOW);
    delay(80);
  }
}

void setup(){
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);  
  
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  resetData();  
}

int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse){
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

void loop() {
  if (radio.isChipConnected()) {
    digitalWrite(LED_PIN, HIGH);
    if (!buzzerPlayed) {
      beep2x();
      buzzerPlayed = true;
    }
    data.throttle = mapJoystickValues(analogRead(A1),  1,   524, 1024, false);  // Throttle
    data.yaw      = mapJoystickValues(analogRead(A0),  2,   506, 1021, false);  // Yaw
    data.pitch    = mapJoystickValues(analogRead(A3),  2,   512, 1021, false);  // Pitch
    data.roll     = mapJoystickValues(analogRead(A2),  1,   512, 1023, false);  // Roll
    // Read AUX1 state
    data.AUX1 = digitalRead(2);
    data.AUX2 = digitalRead(8);
    
    // Send data at regular intervals
    if (millis() - lastSendTime >= sendInterval) {
      lastSendTime = millis();
      radio.write(&data, sizeof(MyData));
    }
  }
  else{
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);  
    delay(1000);
    digitalWrite(BUZZER_PIN, HIGH); 
    delay(1000);    
   }  
}
