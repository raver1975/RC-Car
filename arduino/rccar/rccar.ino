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
int dd=1;
boolean flicker=false;

boolean autopilot=true;
boolean backup=false;

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
for (int j=0;j<1;j++){
//  for(int i=0;i<64;i++){
//    pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
//  }
//  pixels.show();
//  delay(100);
//
//  for(int i=0;i<64;i++){
//    pixels.setPixelColor(i, pixels.Color(0,255,0)); // Moderately bright green color.
//  }
//  pixels.show();
//  delay(100);
//
//  for(int i=0;i<64;i++){
//    pixels.setPixelColor(i, pixels.Color(0,0,255)); // Moderately bright green color.
//  }
//  pixels.show();
//  delay(100);

//  for(int i=0;i<64;i++){
//    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
//  }
//  pixels.show();

  for (stick='0';stick<='9';stick++){
         for (int i=8;i<56;i++){
             pixels.setPixelColor(i, pixels.Color(0,0,0));
         }
         if (stick=='1'){
                    pixels.setPixelColor(45, pixels.Color(150,0,0));
                    pixels.setPixelColor(46, pixels.Color(150,0,0));
                    pixels.setPixelColor(53, pixels.Color(150,0,0));
                    pixels.setPixelColor(54, pixels.Color(150,0,0));            
        }

        if (stick=='2'){
                    pixels.setPixelColor(29, pixels.Color(150,0,0));
                    pixels.setPixelColor(30, pixels.Color(150,0,0));
                    pixels.setPixelColor(37, pixels.Color(150,0,0));
                    pixels.setPixelColor(38, pixels.Color(150,0,0));           
        }

        if (stick=='3'){
                    pixels.setPixelColor(13, pixels.Color(150,0,0));
                    pixels.setPixelColor(14, pixels.Color(150,0,0));
                    pixels.setPixelColor(21, pixels.Color(150,0,0));
                    pixels.setPixelColor(22, pixels.Color(150,0,0));        
                    
        }

        if (stick=='4'){
                    pixels.setPixelColor(43, pixels.Color(150,0,0));
                    pixels.setPixelColor(44, pixels.Color(150,0,0));
                    pixels.setPixelColor(51, pixels.Color(150,0,0));
                    pixels.setPixelColor(52, pixels.Color(150,0,0));            
                    
        }

        if (stick=='0'||stick=='5'){
                    pixels.setPixelColor(27, pixels.Color(150,0,0));
                    pixels.setPixelColor(28, pixels.Color(150,0,0));
                    pixels.setPixelColor(35, pixels.Color(150,0,0));
                    pixels.setPixelColor(36, pixels.Color(150,0,0));          
        }

        if (stick=='6'){
                    pixels.setPixelColor(11, pixels.Color(150,0,0));
                    pixels.setPixelColor(12, pixels.Color(150,0,0));
                    pixels.setPixelColor(19, pixels.Color(150,0,0));
                    pixels.setPixelColor(20, pixels.Color(150,0,0));           
        }

        if (stick=='7'){
                    pixels.setPixelColor(41, pixels.Color(150,0,0));
                    pixels.setPixelColor(42, pixels.Color(150,0,0));
                    pixels.setPixelColor(49, pixels.Color(150,0,0));
                    pixels.setPixelColor(50, pixels.Color(150,0,0));            
        }

        if (stick=='8'){
                    pixels.setPixelColor(25, pixels.Color(150,0,0));
                    pixels.setPixelColor(26, pixels.Color(150,0,0));
                    pixels.setPixelColor(33, pixels.Color(150,0,0));
                    pixels.setPixelColor(34, pixels.Color(150,0,0));            
        }

        if (stick=='9'){
                    pixels.setPixelColor(9, pixels.Color(150,0,0));
                    pixels.setPixelColor(10, pixels.Color(150,0,0));
                    pixels.setPixelColor(17, pixels.Color(150,0,0));
                    pixels.setPixelColor(18, pixels.Color(150,0,0));            
        }
        pixels.show();

        delay(dd);
  }
          stick=0;
           for (int i=0;i<64;i++){
             pixels.setPixelColor(i, pixels.Color(0,0,0));
         }
}

  

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
  duration1 = pulseIn(echoPin1, HIGH,50000);
  cm1 = microsecondsToCentimeters(duration1);

  pinMode(trigPin2, OUTPUT);
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  pinMode(echoPin2, INPUT);
  duration2 = pulseIn(echoPin2, HIGH,50000);
  cm2 = microsecondsToCentimeters(duration2);
  Serial.print(cm1);
  Serial.print(",");
  Serial.println(cm2);
  
 
   for(int i=0;i<cm1/30&&i<8;i++){
    pixels.setPixelColor(i, pixels.Color(flicker?150:0,0,flicker?0:150)); // Moderately bright green color.
   }
   for(int i=0;i<cm2/30&&i<8;i++){
    pixels.setPixelColor(i+56, pixels.Color(flicker?150:0,0,flicker?0:150)); // Moderately bright green color.
   }
   for(int i=cm1/30;i<8;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately black.
   }
   for(int i=cm2/30;i<8;i++){
    pixels.setPixelColor(i+56, pixels.Color(0,0,0)); // Moderately black.
   }
   if (cm1/30<8)pixels.setPixelColor(cm1/30,pixels.Color(flicker?0:150,0,flicker?150:0));
   if (cm2/30<8)pixels.setPixelColor(cm2/30+56,pixels.Color(flicker?0:150,0,flicker?150:0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    flicker=!flicker;
    //delay(delayval); // Delay for a period of time (in milliseconds).

  //}
        
       
       
  
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


//  autopilot-------------------------------------------------------------------------------------------------------------------       
if (autopilot){
    int diff=cm1-cm2;
    if (diff>2&&cm1>150)stick='3';
    else if (diff<-2&&cm2>150)stick='1';
    else stick='2';
    int bb=0;
    if (cm1<100)bb+=100-cm1;
    if (cm2<100)bb+=100-cm2;
    if (bb>0)
   if (backup||(bb>0&&random(200*100)<bb)){
    backup=true;
    stick='8';
    if (diff>0)stick='7';
    else if (diff<-0)stick='9';
    if (cm1>100||cm2>100){backup=false;}
if (cm1>100&&cm2>100)stick='2';
    }
      //  autopilot-------------------------------------------------------------------------------------------------------------------   
  //if (cm1<60 || cm2<60)stick='8';
  }
        mySerial.print((char)stick);
        mySerial.print("|");
        mySerial.print(cm1);
        mySerial.print(",");
        mySerial.println(cm2);
      
  if ((stick>=(int)'0' && stick<=(int)'9')){
      
     //stick='0';
      //if (cm<40&&stick<(int)'4' &&stick>(int)'0'){flag=true;stick+=3;}
        for (int i=8;i<56;i++){
          pixels.setPixelColor(i, pixels.Color(0,0,0));}
        if (stick=='1'){
                    digitalWrite(7,1);  
                    digitalWrite(4,1);
                    digitalWrite(6,0);
                    digitalWrite(5,0);
                    //pixels.setPixelColor(45, pixels.Color(0,150,0));
                    pixels.setPixelColor(46, pixels.Color(0,150,0));
                    pixels.setPixelColor(53, pixels.Color(0,150,0));
                    pixels.setPixelColor(54, pixels.Color(0,150,0));            
        }

        if (stick=='2'){
                    digitalWrite(7,1);
                    digitalWrite(5,1);
                    digitalWrite(4,1);
                    digitalWrite(6,0);
                    pixels.setPixelColor(29, pixels.Color(0,150,0));
                    pixels.setPixelColor(30, pixels.Color(0,150,0));
                    pixels.setPixelColor(37, pixels.Color(0,150,0));
                    pixels.setPixelColor(38, pixels.Color(0,150,0));           
        }

        if (stick=='3'){
                    digitalWrite(7,1);
                    digitalWrite(5,1);
                    digitalWrite(6,0);
                    digitalWrite(4,0);
                    pixels.setPixelColor(13, pixels.Color(0,150,0));
                    pixels.setPixelColor(14, pixels.Color(0,150,0));
                    //pixels.setPixelColor(21, pixels.Color(0,150,0));
                    pixels.setPixelColor(22, pixels.Color(0,150,0));        
                    
        }

        if (stick=='4'){
                    digitalWrite(7,1);
                    digitalWrite(6,1);
                    digitalWrite(4,1);
                    digitalWrite(5,0);
                    pixels.setPixelColor(43, pixels.Color(100,100,0));
                    pixels.setPixelColor(44, pixels.Color(100,100,0));
                    pixels.setPixelColor(51, pixels.Color(100,100,0));
                    pixels.setPixelColor(52, pixels.Color(100,100,0));            
                    
        }

        if (stick=='0'||stick=='5'){
                    digitalWrite(7,1);
                    digitalWrite(6,1);
                    digitalWrite(5,1);
                    digitalWrite(4,1);
                    pixels.setPixelColor(27, pixels.Color(150,150,150));
                    pixels.setPixelColor(28, pixels.Color(150,150,150));
                    pixels.setPixelColor(35, pixels.Color(150,150,150));
                    pixels.setPixelColor(36, pixels.Color(150,150,150));          
        }

        if (stick=='6'){
                    digitalWrite(7,1);
                    digitalWrite(6,1);
                    digitalWrite(5,1);
                    digitalWrite(4,0);
                    pixels.setPixelColor(11, pixels.Color(100,100,0));
                    pixels.setPixelColor(12, pixels.Color(100,100,0));
                    pixels.setPixelColor(19, pixels.Color(100,100,0));
                    pixels.setPixelColor(20, pixels.Color(100,100,0));           
        }

        if (stick=='7'){
                    digitalWrite(6,1);
                    digitalWrite(4,1);
                    digitalWrite(5,0);
                    digitalWrite(7,0);
                    pixels.setPixelColor(41, pixels.Color(0,150,150));
                    //pixels.setPixelColor(42, pixels.Color(0,150,150));
                    pixels.setPixelColor(49, pixels.Color(0,150,150));
                    pixels.setPixelColor(50, pixels.Color(0,150,150));            
        }

        if (stick=='8'){
                    digitalWrite(6,1);
                    digitalWrite(5,1);
                    digitalWrite(4,1);
                    digitalWrite(7,0);
                    pixels.setPixelColor(25, pixels.Color(0,150,150));
                    pixels.setPixelColor(26, pixels.Color(0,150,150));
                    pixels.setPixelColor(33, pixels.Color(0,150,150));
                    pixels.setPixelColor(34, pixels.Color(0,150,150));            
        }

        if (stick=='9'){
                    digitalWrite(6,1);
                    digitalWrite(5,1);
                    digitalWrite(4,0);
                    digitalWrite(7,0);
                    pixels.setPixelColor(9, pixels.Color(0,150,150));
                    pixels.setPixelColor(10, pixels.Color(0,150,150));
                    pixels.setPixelColor(17, pixels.Color(0,150,150));
                   // pixels.setPixelColor(18, pixels.Color(0,150,150));            
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
























