#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            2

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      64

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


int SincomingByte = 0;
SoftwareSerial mySerial(10,9); // RX, TX

const int trigPin1 = 12;
const int echoPin1 = 13;
long duration1, cm1;

const int trigPin2 = 8;
const int echoPin2 = 11;
long duration2, cm2;

int stick='0';
int dd=300;

boolean autopilot=false;

void setup()
{
  pixels.begin();
   
   for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
   }
   pixels.show();
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
 
   for(int i=0;i<cm1/30;i++){
    pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
   }
   for(int i=0;i<cm2/30;i++){
    pixels.setPixelColor(i+56, pixels.Color(0,150,0)); // Moderately bright green color.
   }
   for(int i=cm1/30;i<9;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
   }
   for(int i=cm2/30;i<9;i++){
    pixels.setPixelColor(i+56, pixels.Color(0,0,0)); // Moderately bright green color.
   }
    pixels.show(); // This sends the updated pixel color to the hardware.

    //delay(delayval); // Delay for a period of time (in milliseconds).

  //}
        
       
        stick='0';
  
  if (mySerial.available()){
  

      SincomingByte = mySerial.read();      
            if (SincomingByte=='a'){autopilot=!autopilot;mySerial.print("auto=");mySerial.println(autopilot);}   
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
      delay(dd);


if (autopilot&&cm1!=0&&cm2!=0){
  stick='8';
  if (cm1<50)stick='9';
  if (cm2<50)stick='7';
  if (cm1>50 && cm2>50)stick='2';
  if (cm1>50 && cm2<50)stick='3';
  if (cm1<50 && cm1>50)stick='1';
  }
        mySerial.print((char)stick);
        mySerial.print("|");
        mySerial.print(cm1);
        mySerial.print(",");
        mySerial.println(cm2);
      
  if ((stick>=(int)'1' && stick<=(int)'9')){
      //if (cm<40&&stick<(int)'4' &&stick>(int)'0'){flag=true;stick+=3;}
    
        
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
        delay(dd);
        //if (flag){stick-=3;flag=false;}

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
























