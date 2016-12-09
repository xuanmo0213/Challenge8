#include<Time.h>
#include<Servo.h>
#include <PID_v1.h>

/** -------------Init Values------------ **/
//SoftwareSerial Serial(0,1);
/* ----------------Servos---------------- */
Servo wheels; 
Servo esc; 
int startupDelay = 1000;
/* ----------------Lidars---------------- */

double pulse_width_left, pulse_width_right, pulse_width_difference;
int lidar_left = 10;
int lidar_right = 11;

/* -----------------PIDs----------------- */
int angle;
double Setpoint, Input, Output, Gap, input_raw, Offset;
double aggKp=100, aggKi=0.0005, aggKd=0.15; //aggKp=5, aggKi=0.01, aggKd=1; 
double consKp=8, consKi=0.00005, consKd=3; //consKp=5, consKi=0.01, consKd=1;
double flag = 1;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
/* ----------------Sonar----------------- */
double distance;

/* ----------General Flags--------------- */
int control_mode, turn_flag,init_flag;
  unsigned long old_sec = millis();
  unsigned long cur_sec,diff_sec;

/** -----------Setup Function----------- **/
void setup(){

  control_mode = 0;
  turn_flag = 1;
  init_flag = 1;
  /* Measure when setup */
  pulse_width_left = pulseIn(10, HIGH);
  pulse_width_right = pulseIn(11, HIGH);
  //delay(50);
  Offset = (pulse_width_left + pulse_width_right)/20;
  Setpoint = 0;
  myPID.SetMode(AUTOMATIC);
  /* Assign I/O pin */
  wheels.attach(6);
  esc.attach(5); // initialize ESC to Digital IO Pin #9
  /* Set wheels to default */
  wheels.write(90);
  /* Set sensor input */
  pinMode(lidar_left, INPUT); // Set pin 3 as monitor pin
  pinMode(lidar_right, INPUT);
  /* Start Serial Port */
  Serial.setTimeout(200);
  Serial.begin(9600);
  //mySerial.begin(9600);
  /* Init ESC */
  //Serial.print("calibrate");
  calibrateESC();
  //esc.write(80);
}

void calibrateESC(){
  esc.write(180);       
  delay(startupDelay);
  esc.write(0);
  delay(startupDelay);
  esc.write(90);
  delay(startupDelay);
  esc.write(90);
}

void drive_back(){     
 delay(500);
 //wheels.write(90);
 //esc.write(105);
 //delay(1500);
 pulse_width_left = pulseIn(lidar_left, HIGH);
 pulse_width_right = pulseIn(lidar_right, HIGH);
 if (pulse_width_left < pulse_width_right){
  wheels.write(105);
  esc.write(105);
  delay(1500);
  wheels.write(75);
  esc.write(75);
  delay(1500);
 }
 else {
  wheels.write(80);
  esc.write(105);
  delay(1500);
  wheels.write(125);
  esc.write(75);
  delay(1500);
 }

}

void oscillate(){
  if (control_mode != 1) return;
  
  distance = analogRead(A1);     //Auto_mode  
  //Serial.print("distance");
  //Serial.print("\n");
  //Serial.print(distance);
  
  if (distance >= 30){   
    esc.write(60);                           
    pulse_width_left = pulseIn(lidar_left, HIGH);
    pulse_width_right = pulseIn(lidar_right, HIGH);
    pulse_width_left = pulse_width_left/10; /* 10usec = 1 cm of distance for LIDAR-Lit */
    pulse_width_right = pulse_width_right/10;
    Serial.print("\n");
    Serial.print("left");
    Serial.print(pulse_width_left);
    Serial.print("\n");
    Serial.print("right");
    Serial.print(pulse_width_right);
    
    if (pulse_width_left !=0 && pulse_width_right !=0){
    Serial.print(diff_sec);
    if ( pulse_width_right >= 500)//|| pulse_width_left >= 500)
      { //&& turn_flag == 1){
          cur_sec = millis();
          diff_sec = cur_sec - old_sec;
          if (init_flag == 1){
            diff_sec = 15000;
            init_flag = 0;
           }
          if (diff_sec > 10000){
            old_sec = cur_sec;
            esc.write(90);
            delay(500);
            while(pulse_width_right >=200)
            {
              wheels.write(100);
              pulse_width_right = pulseIn(lidar_right, HIGH)/10;
              esc.write(110);
              delay(200);
            }
            wheels.write(90);
            esc.write(70);
            delay(500);
            wheels.write(25);
              esc.write(70);
              delay(2500);
            /*while(pulse_width_left >= 150){
              wheels.write(110);
              pulse_width_right = pulseIn(lidar_right, HIGH)/10;
              esc.write(70);
              delay(300); 
              
            }*/
          
            wheels.write(95);
            //old_sec = cur_sec;
        }
        else{
          if (pulse_width_left>=220){
            angle = 100;
          }
          else if (pulse_width_left <160){
            angle = 80;
          }
          else angle = 100;
          wheels.write(angle);
        }
      }            
      else if(pulse_width_left <=500 && pulse_width_right <=500){
        input_raw = pulse_width_left - pulse_width_right;
            Input =0 - abs(input_raw);
            Gap = abs( Input - Setpoint ); 
            if (Gap > 60){
                myPID.SetTunings(aggKp, aggKi, aggKd); 
            }
             else if(Gap<25) 
            {  //we're close to setpoint, use conservative tuning parameters 
                myPID.SetTunings(consKp, consKi, consKd); 
            } 
            else 
            { 
            //we're far from setpoint, use aggressive tuning parameters 
            myPID.SetTunings(aggKp, aggKi, aggKd); 
            } 
    
            myPID.Compute(); 
            if (input_raw >= 0){
                if (pulse_width_right < 115) {
                  //Serial.print("\n HERE! 1 ;\n");
                  flag = 2.5;
                  Output = 255;
                }
                else {
                  //Serial.print("\n HERE! 222222 ;\n");
                  flag = -0.25;
                  Output = 255;
                }
            }
            else {
              if (pulse_width_right > 100){
            
                flag = -2;
            }
            else {
              flag = 0.5;
            }
            }
            angle = flag*Output/255*15 + 95;
            wheels.write(angle);
            Serial.print("Angle:");
            Serial.print(angle);
            Serial.print("\n");
      }
    }
  }
  else
  {
    esc.write(90);
    drive_back();
  }
  
}

void control_drive() {
    esc.write(90);
    wheels.write(90);
    control_mode = 0;
}

  
void auto_drive() {
    control_mode = 1;
}

void turn() {
  turn_flag = 1;
}

void keep_straight() {
    turn_flag = 0;

}

void set_crawler(String str){  
  int index = str.indexOf(',');
  String ss = str.substring(0, index);
  String aa = str.substring(index+1);   
  int set_speed = ss.toInt();
  int set_angle = aa.toInt();
    set_speed = 90 - set_speed * 0.5;
    //if(distance > 40 && pulse_width_left > 30 && pulse_width_right >30) {
     // if(distance > 40 || set_angle >180){
        if(set_angle >= 180){
      set_angle = 360 - set_angle; 
      set_speed = 180 - set_speed;
    }
    esc.write(set_speed);
    wheels.write(set_angle); 
      //}
     
   // }
   // else {
     // esc.write(90);
      
    //}
    
    
}


void loop(){
    /*Serial.print("loop");
    if (Serial.available()) {
      Serial.print("open");
      Serial.print("READ" + mySerial.readStringUntil(';'));
    }*/
    //Serial.print(wheels.read());
    if (Serial.available()) {
      String str = Serial.readStringUntil(';');
      char command = str.charAt(0);
      switch(command){
        case 'A':
            auto_drive();
           //Serial.print("a");
            break;
            
        case 'C':
            control_drive();
            //Serial.print("c");
            break;

        /*case 'T':
            turn();
            //Serial.print("t");
            break;

        case 'K':
            keep_straight();
           // Serial.print("k");
            break;*/

        case 'S':
            
            str = str.substring(1);
            //Serial.print("String : " + str);
            //Serial.print("\n");
            set_crawler(str);
            
            break;

        default:
         //oscillate();
         break;
          //Serial.print("Unknown Command");
      }
      //Serial.print("h");
       //Serial.print(command);
       //Serial.print("/n");
       //Serial.print("control_mode"); 
      //Serial.print(control_mode); 
    }
    
    //delay(50);
    oscillate();

}
