#include <dht11.h>
#include <LiquidCrystal.h>
#include <SFE_BMP180.h> 
#include <Wire.h> 
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial esp(10, 11);         //RX TX
 
SFE_BMP180 pressure; 
 
double baseline; 

int red = 28;
int green = 30;
int yellow = 29;

int valve;
int instant_request;
int reset_situation;

int motor = 12;
int reset_pin = 7;

unsigned long day;
unsigned long hour;
unsigned long reset_time = 6000000;

bool key = true;
bool rain_key = true;
bool water_flow_key = true;

int button = 13;

int rain_pin = A0;

int temp_humidty_pin = 22; 
dht11 temp_humidty_sensor;

int humidty_soil_pin = A1;


const int mq135_analog_pin = A3;  
const int mq135_digital_pin = 24;  


volatile int flow_frequency; 
int vol = 0.0;
int one_minute;
unsigned char flowsensor = 2; 
unsigned long currentTime;
unsigned long cloopTime;

int ldr1 = A15;
int ldr2 = A14;
int ldr3 = A13;
int ldr4 = A12;

LiquidCrystal lcd(9,8,6,5,4,3);

int mq9Sensor_pin(A2);


void setup()
{
  
  lcd.begin(16,2);
  
  esp.begin(9600); 
   
  Serial.begin(9600); 
  
  pressure.begin();

  delay(1000);
  esp.println("AT+IPR=9600");
  updateSerial();
  esp.println("AT+GMR");
  updateSerial();
  esp.println("AT");
  updateSerial();
  esp.println("AT+CWJAP=\"USER\",\"12345678\"");
  updateSerial();
  delay(1000);
  esp.println("AT+CWMODE=1");
  updateSerial();
  esp.println("AT+CIPMUX=0");
  updateSerial();

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);

  pinMode(reset_pin,OUTPUT);
  pinMode(motor,OUTPUT);
  pinMode(rain_pin, INPUT);
  pinMode(button,INPUT); 
  pinMode( mq135_digital_pin, INPUT); 
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH);
  attachInterrupt(digitalPinToInterrupt(flowsensor), flow, RISING); 

  currentTime = millis();
  cloopTime = currentTime;

}

String updateSerial()
{
  String data;
  delay(100);

  if (Serial.available()) {
    int inByte = Serial.read();
    esp.print(inByte, DEC);
  }
  
  if (esp.available()) {
    String result_esp = esp.readString();
    //Serial.print(result, DEC);

    //Serial.println(result_esp);

    if(result_esp.indexOf('{') != -1){
            
      StaticJsonDocument<200> doc;   
      String result_json =result_esp.substring(result_esp.indexOf('{'));    
      DeserializationError error = deserializeJson(doc, result_json);

       if(result_esp.indexOf("result") != -1){

        String result = doc["result"]; 
        data = result;
       }
       else if(result_esp.indexOf("situation") != -1){

        String situation = doc["situation"]; 
        instant_request = situation.toInt();

        String valve_ = doc["valve_intensity"]; 
        valve = valve_.toInt();

        String reset_situation_ = doc["reset_situation"]; 
        reset_situation = reset_situation_.toInt();
        
       }
      
      
      //Serial.println(data);
            
      digitalWrite(yellow, LOW);
      digitalWrite(green, HIGH);
      digitalWrite(red, LOW);
      

    }
    else
    {
      
      digitalWrite(yellow, HIGH);
      digitalWrite(green, LOW);
      digitalWrite(red, LOW);
    }

    
  }
  else
  {
      digitalWrite(red, HIGH);
      digitalWrite(green, LOW);
      digitalWrite(yellow, LOW);
  }
//Serial.print("data:");
//Serial.println(data);
  return data;
}



void flow (){
  
   flow_frequency++;
}


int rain_sensor(){
  
  int rain_state = 0;
  rain_state = analogRead(A0); 
  rain_state = 1024 - rain_state;

  if(digitalRead(button) == true){
    
    lcd.setCursor(0,0);
    lcd.print("Rain state ");
    lcd.setCursor(0,1);
    lcd.print(rain_state);
    delay(2000);
    lcd.clear();
  
    if(rain_state > 901){              
      Serial.println("Saganak Yagis!");
      lcd.setCursor(0,0);
      lcd.print("Saganak Yagis! ");
      delay(2000);
      lcd.clear();
    }
    
    if(rain_state > 301 && rain_state <= 900){
      Serial.println("Yagmur yagiyor!");
      lcd.setCursor(0,0);
      lcd.print("Yagmur yagiyor! ");
      delay(2000);
      lcd.clear();
    }
    
    if(300 > rain_state){
      Serial.println("Yagmur yok!");
      lcd.setCursor(0,0);
      lcd.print("Yagmur yok! ");
      delay(2000);
      lcd.clear();
    }    
  }

  return rain_state;
   
}

int DHTdewpoint(){
  
  int chk = temp_humidty_sensor.read(temp_humidty_pin);
  float dewpoint = temp_humidty_sensor.dewPoint();

  if(digitalRead(button) == true){
    
    Serial.print("Cig Olusma Noktasi: ");
    Serial.println(dewpoint, 2);
    lcd.setCursor(0,0);
    lcd.print("Ciy Durumu : ");
    lcd.setCursor(0,1);
    lcd.print(dewpoint, 2);
    delay(2000);
    lcd.clear();
  }

  return dewpoint;

}

int DHTtemperature(){
  
  int chk = temp_humidty_sensor.read(temp_humidty_pin);
  float temp = temp_humidty_sensor.temperature;

  if(digitalRead(button) == true){
    Serial.print("Sicaklik : ");
    Serial.println(temp, 2);
    lcd.setCursor(0,0);
    lcd.print("Sicaklik : ");
    lcd.setCursor(0,1);
    lcd.print(temp, 2);
    delay(2000);
    lcd.clear(); 
  }
  return temp;
}

int DHThumidity(){

  int chk = temp_humidty_sensor.read(temp_humidty_pin);
  float hum = temp_humidty_sensor.humidity;
  if(digitalRead(button) == true){
    
    Serial.print("Nem Orani : ");
    Serial.println(hum, 2);
    lcd.setCursor(0,0);
    lcd.print("Nem Orani : ");
    lcd.setCursor(0,1);
    lcd.print(hum, 2);
    delay(2000);
    lcd.clear();
  }

  return hum;
}

int soil_humidity(){

  int soil_humidty_value;

  soil_humidty_value = analogRead(humidty_soil_pin);
  soil_humidty_value = map(soil_humidty_value,0,1023,0,100);

  if(digitalRead(button) == true){
    
    Serial.print("Toprak nem Degeri :");
    Serial.println(soil_humidty_value);
    lcd.setCursor(0,0);
    lcd.print("Toprak Nem Degeri :");
    lcd.setCursor(0,1);
    lcd.print(soil_humidty_value);
    delay(2000);
    lcd.clear();
  
  }
  return soil_humidty_value;
    
}

int mq135Sensor(){
  int esik;  
  int ppm;
  ppm= analogRead( mq135_analog_pin);  
  esik= digitalRead( mq135_digital_pin); 

  if(digitalRead(button) == true){
    
    Serial.print(" Hava kalitesi: ");  
    Serial.print(ppm);  
    Serial.println("ppm.");
    lcd.setCursor(0,0);
    lcd.print(" Hava Kalitesi: ");
    lcd.setCursor(0,1);
    lcd.print(ppm);  
    lcd.print("ppm. ");
    delay(2000);
    lcd.clear();
  }

 return ppm;
}

int mq9Sensor(){
  
  float RS_air;  
  float R0;  
  float sensorValue; 
  float sensor_volt;
 
  for(int x = 0 ; x < 100 ; x++){
     
    sensorValue = sensorValue + analogRead(mq9Sensor_pin); 
  } 
  
  sensorValue = sensorValue/100.0;  
  sensor_volt = (sensorValue/1024)*5.0; 
  RS_air = (5.0-sensor_volt)/sensor_volt;  
  R0 = RS_air/9.9;
   
  if(digitalRead(button) == true){
    
    Serial.print("CO Degeri : "); 
    Serial.println(R0); 
    lcd.setCursor(0,0);
    lcd.print("CO Degeri :  ");
    //lcd.setCursor(0,1);
    lcd.print(R0);
    delay(2000);
    lcd.clear();
  }

  return R0;
  
}


int waterFlowSensorVol(){

  currentTime = millis();
  if(currentTime >= (cloopTime + 1000)){
    
    cloopTime = currentTime;
    
    if(flow_frequency != 0){      
      
      one_minute = one_minute/60;      
      vol = vol +one_minute;

      if(digitalRead(button) == true){
        
        Serial.print("Vol:");
        Serial.print(vol);
        Serial.println(" L");
        flow_frequency = 0; 
        Serial.print(one_minute, DEC); 
        Serial.println(" L/Sec");
  
        lcd.setCursor(0,0);
        lcd.print("Vol: ");
        lcd.setCursor(0,1);
        lcd.print(vol);
        lcd.print(" L ");
        delay(2000);
        lcd.clear();
  
        lcd.print(one_minute, DEC);
        lcd.print(" L/Sec ");
        delay(2000);
        lcd.clear();
      }      
    }
  }
}


int waterFlowSensorRate(){

  float one_minute_rate;

  currentTime = millis();
   if(currentTime >= (cloopTime + 1000)){
    
    cloopTime = currentTime;
    if(flow_frequency != 0){
                  
      one_minute_rate = (flow_frequency / 7.5); 

      if(digitalRead(button) == true){
        
        Serial.print("Rate: ");
        Serial.print(one_minute_rate);
        Serial.println(" L/M");
  
        lcd.setCursor(0,0);
        lcd.print("Rate:  ");
        lcd.setCursor(0,1);
        lcd.print(one_minute_rate);
        lcd.print(" L/M ");
        delay(2000);
        lcd.clear();
      }      
    }
  }
   return one_minute_rate;
}

int ldr1Sensor(){
  
  int ldr_1;
  
  if(digitalRead(button) == true){
    int ldr_1 = analogRead(ldr1);
    Serial.println("ldr1 :");
    Serial.print(ldr_1);

    lcd.print("ldr1: ");
    lcd.print(ldr_1);
    delay(1000);
    lcd.clear();
  }
  else
  {
     ldr_1 = analogRead(ldr1);
  }
  
  return ldr_1;
}

int ldr2Sensor(){
int ldr_2;

  if(digitalRead(button) == true){
      int ldr_2 = analogRead(ldr1);
      Serial.println("ldr2 :");
      Serial.print(ldr_2);
  
      lcd.print("ldr2: ");
      lcd.print(ldr_2);
      delay(1000);
      lcd.clear();
  }
  else
  {
     ldr_2 = analogRead(ldr2);
  }
  
  return ldr_2;
}

int ldr3Sensor(){
int ldr_3;

  if(digitalRead(button) == true){
      int ldr_3 = analogRead(ldr3);
      Serial.println("ldr3 :");
      Serial.print(ldr_3);
  
      lcd.print("ldr3: ");
      lcd.print(ldr_3);
      delay(1000);
      lcd.clear();
  }
  else
  {
     ldr_3 = analogRead(ldr3);
  }
  
  return ldr_3;
}


int ldr4Sensor(){
int ldr_4;

  if(digitalRead(button) == true){
      int ldr_4 = analogRead(ldr4);
      Serial.println("ldr4 :");
      Serial.print(ldr_4);
  
      lcd.print("ldr4: ");
      lcd.print(ldr_4);
      delay(1000);
      lcd.clear();
  }
  else
  {
     ldr_4 = analogRead(ldr4);
  }
  
  return ldr_4;
}


int bmp180Sensor(){
  
  char status;
  double T,P;  
  pressure.startPressure(3);
  pressure.getPressure(P,T);
  
  if(digitalRead(button) == true){
    
    Serial.print("Basınç: ");
    Serial.print(P);
    Serial.println(" mb");
    
    delay(500);
  }

  return P;
  
}


void loop(){
  
  if(digitalRead(button) == true){
  
    digitalWrite(red, HIGH);
    digitalWrite(green, LOW);
    digitalWrite(yellow, LOW);
    
    lcd.clear();
    rain_sensor();
    DHTdewpoint();
    DHTtemperature();
    DHThumidity();
    soil_humidity();
    mq135Sensor();
    mq9Sensor();
    waterFlowSensorVol();
    waterFlowSensorRate();
    ldr1Sensor();
    ldr2Sensor();
    ldr3Sensor();
    ldr4Sensor();
    bmp180Sensor();
  
   // Serial.println("------------------"); 
    key = true;
  }
  else
  {  
  
    if(key == true){
    lcd.print("Arduino Busy!!");
    key = false;
  }

  int waterFlow_val; 
  int rain_value = rain_sensor();

  getInstantRequest();

  endOfDay();
  dayTimeSituation();

  if(reset_situation==1)
  {
    digitalWrite(reset_pin, HIGH);
  }

  if(rain_value>300 && rain_key == true)
  {
    addRainSituation(1);
    rain_key = false;
  }
  else if(rain_value<300 && rain_key == false)
  {
    addRainSituation(0);
    rain_key = true;
  }

    //getValveSituation();

   
   digitalWrite(motor, valve);
  if(valve >0)
  {
    
     if(waterFlow_val == 0 && water_flow_key == true)
      {
      addWaterStoppage(true);
      water_flow_key = false;
      }
     else if(waterFlow_val > 0 && water_flow_key==false)
      {
        addWaterStoppage(false);
        water_flow_key = true;
      }
  }
  
  
  if(instant_request == 1){
    
    //Serial.println("iff getInstantRequest girdi!!!!!!");
    updateInstantSituation();
  }

  check_reset();

 }

}

void check_reset()
{
  if(millis()>reset_time )
  {
    digitalWrite(reset_pin, HIGH);
  }
  
}
void dayTimeSituation()
{
  
  if(millis()>hour )
  {
    hour += 3600000;
    //hour += 10000;
    String temperature_val = String(DHTtemperature());
    String humidity_val = String(DHThumidity());
    String soil_humidity_val = String(soil_humidity());
    String air_quality_val =  String(mq135Sensor());
    
    String co_val = String(mq9Sensor());
    String co2_val = "0";                       //bu sensör olmadığından dolayı 0 yazılmıştır
    String dew_formation = String(DHTdewpoint());
    String pressure_val = String(bmp180Sensor());

    String request = "/addDayTimeSituation.php?key=123&temperature="+temperature_val;
    request += "&moisture="+humidity_val;
    request += "&soil_moisture="+soil_humidity_val;
    request += "&co="+co_val;
    request += "&co2="+co2_val;
    request += "&dew_formation="+dew_formation;
    request += "&pressure="+pressure_val;
    request += "&air_quality="+air_quality_val;
    int_internet(request);
    
  }
}

void endOfDay()
{
  if(millis()>day )
  {
    day += 86400000;

    //day += 30000;
    String request = "/endOfDay.php?key=123&volume_water_used="+vol;
    vol = 0;

    int_internet(request);
    
  }
}
/*
int getValveSituation()
{
  String request = "/getValveSituation.php?key=123";
  return int_internet(request);
}*/

int getInstantRequest()
{
 // Serial.println("int getInstantRequest girdi!!!!!!");
  String request = "/getInstantRequest.php?key=123";
  return int_internet(request);
}

void addRainSituation(int situation)
{
  String request = "/addRainSituation.php?key=123&situation="+String(situation);
  int_internet(request);
}

void addWaterStoppage(bool situation)
{
  String request = "/addWaterStoppage.php?key=123&situation="+String(situation);
  int_internet(request);
}


void updateInstantSituation(){

 // Serial.println("void updateInstantSituation girdi!!!!!!");
  String temperature_val = String(DHTtemperature());
  String humidity_val = String(DHThumidity());
  String soil_humidity_val = String(soil_humidity());
  String air_quality_val =  String(mq135Sensor());
  String co_val = String(mq9Sensor());
  String dew_formation = String(DHTdewpoint());
  String pressure_val =String(bmp180Sensor());
  String ldr1_val = String(ldr1Sensor());
  String ldr2_val = String(ldr2Sensor());
  String ldr3_val = String(ldr3Sensor());
  String ldr4_val = String(ldr4Sensor());
  String water_flow_val = String(waterFlowSensorRate());
  String rain_val = String(rain_sensor());


  String request = "/updateInstantSituation.php?key=123&temperature="+temperature_val;
  request += "&moisture="+humidity_val;
  request += "&soil_moisture="+soil_humidity_val;
  request += "&co="+co_val;
  request += "&co2=0";
  request += "&dew_formation="+dew_formation;
  request += "&pressure="+pressure_val;
  request += "&air_quality="+air_quality_val;
  request += "&ldr1="+ldr1_val;
  request += "&ldr2="+ldr2_val;
  request += "&ldr3="+ldr3_val;
  request += "&ldr4="+ldr4_val;
  request += "&valve_intensity="+String(valve);
  request += "&water_flow="+water_flow_val;
  request += "&rain_situation="+rain_val;



 // Serial.println(request);
  
  int_internet(request);

}


int int_internet(String request){
  
  //Serial.println("buraya geldi");
  int result;
  esp.println("AT+CIPSTART=\"TCP\",\"167.71.121.3\",80");
  updateSerial();
  
  String str_request="GET /methods"+request;
  //str_request+='"';
  //Serial.println(str_request);
   
  esp.print("AT+CIPSEND=");
  updateSerial();
  esp.println(request.length()+14);
  updateSerial();
  esp.println(str_request); 
  //updateSerial();

   
  String str_result = updateSerial();

 // Serial.print("str_result : ");
 // Serial.println(str_result);
  
  result = str_result.toInt();
  //Serial.print("int result :");
  //Serial.println(result);
  return result;
  
}
/*
bool bool_internet(String request)
{
  
  bool result;
  
  String str_request="at+httppara=\"URL\",\"http://www.arduino.epsilonarge.com/methods"+request;
  str_request+='"';


  //Serial.println(str_request);

  gsm.println(str_request); 
  updateSerial();
  gsm.println("AT+HTTPACTION=0"); 
  updateSerial();
  gsm.println("AT+HTTPREAD"); 
  String str_result = updateSerial();

  //Serial.print("str_result : ");
  //Serial.println(str_result);

  
  result = str_result.toInt();
  Serial.print("bool result :");
  Serial.println(result);
  return result;
}
*/
