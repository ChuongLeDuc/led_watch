
#include "server.h"

#include <SimpleTimer.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <EEPROM.h>

WiFiUDP udp;
NTPClient ntp(udp,"pool.ntp.org");
ESP8266WebServer webserver;
SimpleTimer timer;
int time_interval;

short scan_speed=1; 
int start_read_eeprom=5;

short button =0;// flash button

short latchPin = 15;//d8 tím STCP
short clockPin = 5;//d1 trắng SHCP
short dataPin = 14;//d5 xanh DS

unsigned long past_time=0;

short second_led=4;//d2
 
unsigned int lable[][5] = {
  {0x0E,0x11,0x1F,0x11,0x11},//A
  {0x1F,0x11,0x0F,0x11,0x1F},//B
  {0x1F,0x01,0x01,0x01,0x1F},//C
  {0x0F,0x11,0x11,0x11,0x0F},//D
  {0x1F,0x01,0x1F,0x01,0x1F},//E
  {0x1F,0x01,0x0F,0x01,0x01},//F
  {0x1E,0x01,0x19,0x11,0x0E},//G
  {0x11,0x11,0x1F,0x11,0x11},//H
  {0x1F,0x04,0x04,0x04,0x1F},//I
  {0x10,0x10,0x10,0x11,0x0E},//J
  {0x12,0x0A,0x06,0x0A,0x12},//K
  {0x01,0x01,0x01,0x01,0x1F},//L
  {0x1B,0x15,0x11,0x11,0x11},//M
  {0x11,0x13,0x15,0x19,0x11},//N
  {0x0E,0x11,0x11,0x11,0x0E},//O
  {0x1F,0x11,0x1F,0x01,0x01},//P
  {0x1F,0x11,0x1F,0x10,0x10},//Q
  {0x1F,0x11,0x0F,0x11,0x11},//R
  {0x1E,0x01,0x0E,0x10,0x0F},//S
  {0x1F,0x04,0x04,0x04,0x04},//T
  {0x11,0x11,0x1F,0x11,0x0E},//U
  {0x11,0x11,0x11,0x0A,0x04},//V
  {0x11,0x11,0x15,0x15,0x0A},//W
  {0x11,0x0A,0x04,0x0A,0x11},//X
  {0x11,0x11,0x0E,0x04,0x04},//Y
  {0x1F,0x08,0x04,0x02,0x1F},//Z
  {0x0E,0x0A,0x0A,0x0A,0x0E},//0
  {0x04,0x06,0x04,0x04,0x04},//1
  {0x0E,0x08,0x0E,0x02,0x0E},//2
  {0x0E,0x08,0x0E,0x08,0x0E},//3
  {0x0A,0x0A,0x0E,0x08,0x08},//4
  {0x0E,0x02,0x0E,0x08,0x0E},//5
  {0x0E,0x02,0x0E,0x0A,0x0E},//6
  {0x0E,0x0A,0x08,0x08,0x08},//7
  {0x0E,0x0A,0x0E,0x0A,0x0E},//8
  {0x0E,0x0A,0x0E,0x08,0x0E},//9
  {0x00,0x00,0x00,0x0E,0x0E},//.
  {0x04,0x04,0x04,0x00,0x04},//!
  {0x00,0x00,0x00,0x00,0x00},// space
  };
int row[] = {1,2,4,8,16};

char character[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9','.','!',' '};
short max_value=4;
String hour;
String minute;
short old_second=0;
char  old_value[]={' ',' ',' ',' '};
  
void setup() {
   Serial.begin(9600);
   
   pinMode(latchPin, OUTPUT);
   pinMode(clockPin, OUTPUT);
   pinMode(dataPin, OUTPUT);
   pinMode(second_led, OUTPUT);
   pinMode(button,INPUT_PULLUP);

   EEPROM.begin(500);

   set_esp_mode();
   int time_zone=EEPROM.read(4);
   Serial.print("TIME ZONE: ");
   Serial.println(time_zone);
   ntp.setTimeOffset(time_zone*3600);

   webserver.on("/",mainpage);
   webserver.on("/get_local_ssid",get_local_ssid);
   webserver.on("/device_wifi",device_wifi);
   webserver.on("/local_wifi",local_wifi);
   webserver.begin();

   attachInterrupt(digitalPinToInterrupt(button), set_ap_mode,FALLING);
   time_interval=timer.setInterval(1000,blink_led);
   }
 
void loop() { 
 if(WiFi.getMode()==WIFI_STA && WiFi.status()==WL_CONNECTED){

  if(millis()-past_time>=5000){
    Serial.println("STA_MODE");
    String current_time=get_time();
    Serial.println(current_time);
    Serial.println("----------------------------");

    for(int i=max_value-1;i>=0;i--){
    String animation_text;
    if(current_time.charAt(i)!=old_value[i]){
      
      old_value[i]=current_time.charAt(i);
      for(int s=0;s<max_value;s++){
        animation_text.concat(String(old_value[s]));
      }
      int pos=i+1;
      animation(pos,animation_text);
    }
    old_value[i]=current_time.charAt(i);
  }
    display_character(current_time);
    past_time=millis();
  }else{
    String mess="";
    for(int i=0;i<max_value;i++){
      mess.concat(old_value[i]);
    }
    display_character(mess);
  }
  
  timer.run();
  
 }else{
  display_character("SET!");
  webserver.handleClient();
 }
}

void blink_led(){
  digitalWrite(second_led,1);
  delay(1);
  digitalWrite(second_led,0);
}

void mainpage(){
  webserver.send(200,"text/html",index_page);
}
void get_local_ssid(){
  String s="";
  int num=WiFi.scanNetworks();
  for(int i=0;i<num;i++){
    String k="<option>";
    k.concat(String(WiFi.SSID(i)));
    k.concat("</option>");
    s.concat(k);
  }
  webserver.send(200,"text/html",s);
}


void device_wifi(){
  String s="";
  
  String device_name=webserver.arg("device_name");
  String old_pass=webserver.arg("old_pass");
  String new_pass=webserver.arg("new_pass");
 
  String current_ssid=get_eeprom(2);
  String current_pass=get_eeprom(3);

  if(current_pass==old_pass){
    save_eeprom("device",device_name,new_pass,"no");
    s="UPDATE DEVICE'S CONFIG COMPLETED. PRESS 'RESET' TO RESTART!";
  }else{
    s="OLD PASSWORD ISN'T CORRECT";
  }
  webserver.send(200,"text/html",s);
}

void local_wifi(){
  String s="";
  
  String time_val=webserver.arg("time_zone");
  
  String device_pass=webserver.arg("device_pass");
  String home_ssid=webserver.arg("home_ssid");
  String home_pass=webserver.arg("home_pass");
 
 
  String current_ssid=get_eeprom(0);
  String current_pass=get_eeprom(1);

  String esp_pass=get_eeprom(3);

  if(device_pass==esp_pass){
    save_eeprom("home",home_ssid,home_pass,time_val);
    s="UPDATE HOME WIFI COMPLETED. PRESS 'RESET' TO RESTART!";
  }else{
    s="DEVICE PASSWORD ISN'T CORRECT";
  }
  webserver.send(200,"text/html",s);
}
void set_esp_mode(){
  
   bool check=check_home_access();
   
   if(check==true){
    
    String home_ssid=get_eeprom(0);
    String home_pass=get_eeprom(1);
    Serial.println("STA_MODE:");
    
    Serial.print("SSID: ");
    Serial.println(home_ssid);
    
    Serial.print("PASS: ");
    Serial.println(home_pass);
    
    WiFi.begin(home_ssid,home_pass);

    unsigned int esp_time=millis();
    
    while(WiFi.status()!=WL_CONNECTED){
      if(millis()-esp_time<=60*1000){
        for(int i=1;i<=500/(scan_speed*5);i++){
        display_character("HOME");
      }

      for(int i=1;i<=1000/(scan_speed*5);i++){
        display_character("WIFI");
      }

      for(int i=1;i<=1000/(scan_speed*5);i++){
        display_character("....");
      }
      
      for(int i=1;i<=1000/(scan_speed*5);i++){
        display_character("WAIT");
      }

      Serial.print(".");
      delay(1);
      
      }else{
        set_ap_mode();
        Serial.println("STA_MODE FAILED, AP_MODE ACTIVED!");
        break;
      }
    }
   }else{
    set_ap_mode();
   }
 }
  
 
ICACHE_RAM_ATTR void set_ap_mode(){
  WiFi.disconnect();
  IPAddress ip(192,168,1,8);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(ip, gateway, subnet);
  
  String esp_name=get_eeprom(2);
  String esp_pass=get_eeprom(3);
  Serial.println(esp_name);
  Serial.println(esp_pass);
  WiFi.softAP(esp_name,esp_pass);
  Serial.println("AP_MODE completed");
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.softAPIP());
  
}


bool check_home_access(){
  int ssid_size=EEPROM.read(0);
  int pass_size=EEPROM.read(1);

  if(ssid_size!=0 && pass_size!=0){
    return true;
  }else{
    return false;
  }
}

String get_eeprom(int size_pos){
  
  String home_ssid="";
  String home_password="";
  String esp_name="";
  String esp_password="";
  
  int ssid_size=EEPROM.read(0);
  int home_pass_size=EEPROM.read(1);
  int esp_name_size=EEPROM.read(2);
  int esp_pass_size=EEPROM.read(3);

  for(int i=start_read_eeprom;i<start_read_eeprom+ssid_size;i++){
  home_ssid.concat(char(EEPROM.read(i)));
 }

 for(int i=start_read_eeprom+ssid_size;i<start_read_eeprom+ssid_size+home_pass_size;i++){
  home_password.concat(char(EEPROM.read(i)));
 }

  for(int i=start_read_eeprom+ssid_size+home_pass_size;i<start_read_eeprom+ssid_size+home_pass_size+esp_name_size;i++){
  esp_name.concat(char(EEPROM.read(i)));
 }

  for(int i=start_read_eeprom+ssid_size+home_pass_size+esp_name_size;i<start_read_eeprom+ssid_size+home_pass_size+esp_name_size+esp_pass_size;i++){
   esp_password.concat(char(EEPROM.read(i)));
  }
 switch (size_pos){
  case 0:
    return home_ssid;
    break;

  case 1:
    return home_password;
    break;

 case 2:
    return esp_name;
    break;
 case 3:
    return esp_password;
    break; 
 default:
    return "error";
 }
 
}

void save_eeprom(String type,String ssid_value,String password_value,String time_val){
  String ssid,password,esp_name,esp_pass;

  if(type=="home"){
    ssid=ssid_value;
    password=password_value;
    esp_name=get_eeprom(2);
    esp_pass=get_eeprom(3);
  }else{
    ssid=get_eeprom(0);
    password=get_eeprom(1);
    esp_name=ssid_value;
    esp_pass=password_value;
  }
  
  int ssid_size=ssid.length();
  int pass_size=password.length();
  int esp_name_size=esp_name.length();
  int esp_pass_size=esp_pass.length();
  
  EEPROM.put(0,ssid_size);
  EEPROM.put(1,pass_size);
  EEPROM.put(2,esp_name_size);
  EEPROM.put(3,esp_pass_size);
  if(time_val!="no"){
     EEPROM.put(4,time_val.toInt());
  }
  
  Serial.println("...........................");
  for(int i=start_read_eeprom;i<start_read_eeprom+ssid_size;i++){
    char value=ssid.charAt(i-start_read_eeprom);
    EEPROM.write(i,value);
  }
  
  for(int i=start_read_eeprom+ssid_size;i<start_read_eeprom+ssid_size+pass_size;i++){
    char value=password.charAt(i-(start_read_eeprom+ssid_size));
    EEPROM.write(i,value);
  }

  for(int i=start_read_eeprom+ssid_size+pass_size;i<start_read_eeprom+ssid_size+pass_size+esp_name_size;i++){
    char value=esp_name.charAt(i-(start_read_eeprom+ssid_size+pass_size));
    EEPROM.write(i,value);
  }

 
  for(int i=start_read_eeprom+ssid_size+pass_size+esp_name_size;i<start_read_eeprom+ssid_size+pass_size+esp_name_size+esp_pass_size;i++){
    char value=esp_pass.charAt(i-(start_read_eeprom+ssid_size+pass_size+esp_name_size));
    EEPROM.write(i,value);
  }
  
  EEPROM.commit();


  Serial.print("HOME_SSID: ");
  Serial.println(String(get_eeprom(0)));

  Serial.println("-------------------------------------");
  
  Serial.print("HOME_PASS: ");
  Serial.println(String(get_eeprom(1)));
  
  Serial.println("-------------------------------------");

  Serial.print("ESP_SSID: ");
  Serial.println(String(get_eeprom(2)));

  Serial.println("-------------------------------------");

  Serial.print("ESP_PASS: ");
  Serial.println(String(get_eeprom(3)));

  Serial.print("TIME_ZONE: ");
  Serial.println(String(EEPROM.read(4)));
  }
  
 

String get_time(){

  ntp.update();
  int h=ntp.getHours();
  int m=ntp.getMinutes();
  
  if(h<10){
    hour="0";
    hour.concat(String(h));
  }else{
    hour=String(h);
  }

  if(m<10){
    minute="0";
    minute.concat(String(m));
  }else{
   minute=String(m);
  }
  
  hour.concat(minute);

  return hour;
}

void display_character(String message){
  int pos[max_value];
  for(int i=0;i<max_value;i++){
    pos[i]=-1;
  }
  for(int i=0;i<=message.length()-1;i++){
    for(int j=0;j<sizeof(character);j++){
      if(message.charAt(i)==character[j]){
        int count=0;
        for(int h=0;h<max_value;h++){
          if(pos[h]>-1){
            count++;
          }
        }
        if(count<4){
          pos[count]=j;
        }   
      }
    }
  }
  for(int k=0; k<=4;k++){
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[3]][k])); 
    shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[2]][k])); 
    shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[1]][k]));
    shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[0]][k]));
    shiftOut(dataPin, clockPin,MSBFIRST, row[k]);
    digitalWrite(latchPin, HIGH);
    delay(scan_speed);
     }
}

void animation(int chip_pos, String message){
  
  int pos[max_value];
  for(int i=0;i<max_value;i++){
    pos[i]=-1;
  }
  
  for(int i=0;i<=message.length()-1;i++){
    for(int j=0;j<sizeof(character);j++){
      if(message.charAt(i)==character[j]){
        int count=0;
        for(int h=0;h<max_value;h++){
          if(pos[h]>-1){
            count++;
          }
        }
        if(count<4){
          pos[count]=j;
        }   
      }
    }
  }

  int char_pos=pos[chip_pos-1];
  
  int count=4;
  for(int i=0;i<5;i++){
    
     char new_code[5];
     
     for(int j=4;j>=0;j--){
      if(j<count){
        new_code[j]=0x1F;
      }else{
        new_code[j]=lable[char_pos][j];
      }
     }
     
    for(int t=0;t<=50/(5*scan_speed);t++){
      for(int k=0;k<5;k++){
        
        digitalWrite(latchPin, LOW);
        
        switch (chip_pos){
          case 1:
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[3]][k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[2]][k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[1]][k]));
            shiftOut(dataPin, clockPin,MSBFIRST, ~(new_code[k]));
          break;
  
          case 2:
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[3]][k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[2]][k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(new_code[k]));
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[0]][k]));
          break;
  
          case 3:
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[3]][k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(new_code[k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[1]][k]));
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[0]][k]));
          break;
  
          case 4:
            shiftOut(dataPin, clockPin,MSBFIRST, ~(new_code[k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[2]][k])); 
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[1]][k]));
            shiftOut(dataPin, clockPin,MSBFIRST, ~(lable[pos[0]][k]));
          break;
        }
        shiftOut(dataPin, clockPin,MSBFIRST, row[k]);
        digitalWrite(latchPin, HIGH);
        delay(scan_speed);
     }

  
    }
    count--;
  } 
}
