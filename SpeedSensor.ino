// This #include statement was automatically added by the Particle IDE.
#include "SparkTime/SparkTime.h"
int speedsensor = D2;
int val =0;
time_t tst =0;
time_t ted =0;
bool flag =0;
time_t period =0;
double cons=78.5;
double s =0;

void setup() {
  pinMode(speedsensor, INPUT);
  val = digitalRead(speedsensor);
  
}

void loop() {
    int in = digitalRead(speedsensor);   // read the input pin
    if (val != in) {
        if(flag == 0) {
            val = in;
            tst = millis();
            //Particle.publish("start", String(tst));
          flag = 1;
        }
        else {
            val = in;
            ted = millis();
           // Particle.publish("end", String(ted));
            speed(tst, ted);
            flag=0;
        }
        
        
      
    } 
}

void speed(time_t tst, time_t ted) {
    period = ted - tst;
    s = cons/period;
    Particle.publish("speed", String(s));
    delay(1000);
}








