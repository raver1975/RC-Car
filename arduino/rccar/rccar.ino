#include <SoftwareSerial.h>
int SincomingByte = 0;
SoftwareSerial mySerial(10,9); // RX, TX

const int trigPin1 = 12;
const int echoPin1 = 13;
long duration1, cm1;

const int trigPin2 = 8;
const int echoPin2 = 11;
long duration2, cm2;

int stick='0';
int dd=200;

void setup()
{
  mySerial.begin(9600);
  Serial.begin(9600);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  
  digitalWrite(7, 1);
  digitalWrite(6, 1);
  digitalWrite(5, 1);
  digitalWrite(4, 1);
  Serial.println("Ready");
  mySerial.println("MySerial Ready");
  
}
 
void loop()
{
  pinMode(trigPin1, OUTPUT);
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  pinMode(echoPin1, INPUT);
  duration1 = pulseIn(echoPin1, HIGH);
  cm1 = microsecondsToCentimeters(duration1);

  pinMode(trigPin2, OUTPUT);
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  pinMode(echoPin2, INPUT);
  duration2 = pulseIn(echoPin2, HIGH);
  cm2 = microsecondsToCentimeters(duration2);
 

  
  if (mySerial.available()){
  
      SincomingByte = mySerial.read();         
      if (SincomingByte>=(int)'0' && SincomingByte<=(int)'9'){
          stick=SincomingByte;
      }
      else   {
          SincomingByte=-1;
          mySerial.print((char)SincomingByte);
          mySerial.println('-');
      }
  }
      digitalWrite(7,1);
      digitalWrite(6,1);
      digitalWrite(5,1);
      digitalWrite(4,1);
      
  if ((stick>=(int)'1' && stick<=(int)'9')){
      //if (cm<40&&stick<(int)'4' &&stick>(int)'0'){flag=true;stick+=3;}
        mySerial.print((char)stick);
        mySerial.print("|");
        mySerial.print(cm1);
        mySerial.print(",");
        mySerial.println(cm2);
        
        if (stick=='1'){
                    digitalWrite(6,0);
                    digitalWrite(5,0);
        }

        if (stick=='2'){
                    digitalWrite(6,0);
        }

        if (stick=='3'){
                    digitalWrite(6,0);
                    digitalWrite(4,0);
                    
        }

        if (stick=='4'){
                    digitalWrite(5,0);
                    
        }

        if (stick=='0'||stick=='5'){
                    digitalWrite(7,1);
                    digitalWrite(6,1);
                    digitalWrite(5,1);
                    digitalWrite(4,1);
        }

        if (stick=='6'){
                    digitalWrite(4,0);
        }

        if (stick=='7'){
                    digitalWrite(5,0);
                    digitalWrite(7,0);
        }

        if (stick=='8'){
                    digitalWrite(7,0);
        }

        if (stick=='9'){
                    digitalWrite(4,0);
                    digitalWrite(7,0);
        }

        //if (flag){stick-=3;flag=false;}
        delay(dd);
       
        stick='0';
    }
        
}

long microsecondsToInches(long microseconds)
{
  return microseconds / 74 / 2;
}
 
long microsecondsToCentimeters(long microseconds)
{
   return microseconds / 29 / 2;
}
























