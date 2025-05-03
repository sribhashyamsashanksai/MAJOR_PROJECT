
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <Servo.h>

Servo myservo;
int statusCode = 0;

unsigned long lastTime = 0;
unsigned long timerDelay = 16000;
WiFiClient  client;
const int FieldNumber1 = 1;

String strs[14]={"0","0","2539688","JUQ9EO64JJ4I5JKH","slabs","12345687","0","0","0","0","0","0","0","0"};
int StringCount = 0;
int prv=0;
int led=D4;
int buz=D8;


void setup()
{
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Serial.begin(9600);
  pinMode(led,OUTPUT); 
  pinMode(buz,OUTPUT); 
  
  myservo.attach(D2);
  myservo.write(0);
  delay(1000);
  digitalWrite(led,1);
  digitalWrite(buz,0);
  
  
  
}

void loop()
{
  //----------------- Network -----------------//
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
    WiFi.begin(strs[4], strs[5]);
      for(int kk=0;kk<10;kk++)
      {
        digitalWrite(led,0);
        delay(300);
        digitalWrite(led,1);
        delay(300);
      }
   if(WiFi.status() == WL_CONNECTED)
    Serial.println("ok");
  }
 
  //---------------- Channel 1 ----------------//
  const char* string2 = strs[2].c_str();
  const char* string3 = strs[3].c_str();
  int temp = ThingSpeak.readLongField(atol(string2), FieldNumber1, string3);
  statusCode = ThingSpeak.getLastReadStatus();
  if (statusCode == 200)
  {
    if(temp !=prv)
    {
      prv=temp;
       Serial.print(temp);
       if(temp==1)
       {
        digitalWrite(buz,1);
       }
       if(temp==2)
       {
        digitalWrite(buz,0);
       }
       
       if(temp==3)
       {
        myservo.write(90);
       }
        if(temp==4)
       {
        myservo.write(0);
       }
    }
   
    
  }
  
  delay(100);
  

if (Serial.available()) 
{



  int x=Serial.read();
 if(x=='1')
       {
        digitalWrite(buz,1);
       }
       if(x=='2')
       {
        digitalWrite(buz,0);
       }
      
      if(x=='3')
       {
        myservo.write(90);
       }
        if(x=='4')
       {
        myservo.write(0);
       }
 
}
  delay(500);
 
}
