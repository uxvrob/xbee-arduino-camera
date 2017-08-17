
int t;
String inputString = "";
boolean stringComplete = false;

void setup() {
  Serial.begin(115200);
  inputString.reserve(200);
  t = millis();

}

void loop() {

  if((millis() - t) > 1000){
    //Serial.println("$$$");
    t=millis();
  }
  
  
    if(stringComplete){

      inputString.trim();
      
      if(inputString.equals("AV+JRBD"))
        Serial.println("+RRBDRES:00FF214E2418");
      else if(inputString.equals("AV+JRES"))
        Serial.println("ROK");
      else if(inputString.equals("OK")){
        
      }
      else{
        Serial.print("NOK: ");
        Serial.println(inputString);
      }

      inputString = "";
      stringComplete = false;
    }

}


void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    inputString += inChar;

    if(inChar == '\n'){
      stringComplete = true;
    }

    //Serial.print("RX: ");
    //Serial.print(inChar);
    //Serial.println("");
  }
}

