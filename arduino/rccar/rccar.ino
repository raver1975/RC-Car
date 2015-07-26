#include <SoftwareSerial.h>
int SincomingByte = 0;
SoftwareSerial mySerial(10,9); // RX, TX
const int trigPin = 12;
const int echoPin = 13;
int stick='0';
bool flag=false;
int dd=200;
long duration, inches, cm;

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
  //if (!mySerial.available()) {
    
 
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
                // read the incoming byte:
  // }
  //else{
   
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
        if ((stick>=(int)'1' && stick<=(int)'9')){
        if (cm<40&&stick<(int)'4' &&stick>(int)'0'){flag=true;stick+=3;}
 
          mySerial.print((char)stick);
          mySerial.print("|");
          mySerial.println(cm);
        
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

        if (flag){stick-=3;flag=false;}
        delay(dd);
       digitalWrite(7,1);
   digitalWrite(6,1);
   digitalWrite(5,1);
   digitalWrite(4,1);
   stick='0';
        //}
        
        }
        
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}
 
long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
























