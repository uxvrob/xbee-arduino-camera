// Gateway node on COM 6
// Serial: 55635303838351319170


void setup() {

  Serial.begin(115200);

}

void loop() {
  
  if (Serial.available() > 0){
    Serial.print("RX: ");
    while(Serial.available()>0){
      Serial.write(Serial.read());
    }
    Serial.println("");
  }

}
