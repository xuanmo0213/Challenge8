#include<SoftwareSerial.h>
#include<Servo.h>
#include <PID_v1.h>

/** -------------Init Values------------ **/
SoftwareSerial mySerial(0,1);
/* ----------------Servos---------------- */
Servo wheels; 
Servo esc; 
int startupDelay = 1000;
/* ----------------Lidars---------------- */

double pulse_width_left, pulse_width_right;
int lidar_left = 10;
int lidar_right = 11;

/* -----------------PIDs----------------- */
int angle;
double Setpoint, Input, Output, Gap, input_raw, Offset;
double aggKp=100, aggKi=0.0005, aggKd=0.15; 
double consKp=8, consKi=0.00005, consKd=3;
double flag = 1;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
/* ----------------Sonar----------------- */
double distance;

/* ----------General Flags--------------- */
int control_mode, turn_flag;

/** -----------Setup Function----------- **/
void setup(){
  control_mode = 0;
  turn_flag = 0;
  /* Measure when setup */
  pulse_width_left = pulseIn(10, HIGH);
  pulse_width_right = pulseIn(11, HIGH);
  delay(50);
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
  Serial.setTimeout(250);
  Serial.begin(9600);
  mySerial.begin(9600);
  /* Init ESC */
  //Serial.print("calibrate");
  calibrateESC();
  //esc.write(80);
}

void calibrateESC(){
  //Serial.print("enter");
  esc.write(180); // full backwards
  delay(startupDelay);
  esc.write(0);
  delay(startupDelay);
  esc.write(90);
  delay(startupDelay);
  esc.write(90);
 }

 void drive_back(){
//Serial.print("drive_back");
 delay(500);
 wheels.write(90);
 esc.write(105);
 delay(1500);
 wheels.write(125);
 esc.write(75);
 delay(1500);
 }
 

 
  void oscillate(){
    if (control_mode != 1) return;
    distance = analogRead(A0);
    //Serial.print(control_mode);
    //Serial.print(distance);
    /* Safe Driving */
    if (distance >= 40){
      //Serial.print("dis  > 40 ");
       //Serial.print(distance);
         //Serial.print("\n");
   
      /* Auto_mode */
      
        //Serial.print("Auto mode\n");
        //delay(1000);
        Serial.print(pulse_width_left);
        Serial.print("\n");
        Serial.print(pulse_width_right);
        Serial.print("\n");                 
        pulse_width_left = pulseIn(lidar_left, HIGH);
        pulse_width_right = pulseIn(lidar_right, HIGH);
        if (pulse_width_left !=0 && pulse_width_right !=0){
          if (pulse_width_left >= 5000 && turn_flag == 1){
            //Serial.print("Turn right\n");
            
            esc.write(70);
            wheels.wirte(160);
            delay(2000);
            /*esc.write(75);
            pulse_width_left = pulse_width_left/10; /* 10usec = 1 cm of distance for LIDAR-Lit */
            /*pulse_width_right = pulse_width_right/10;
            input_raw = pulse_width_right - Offset;
            Input =0 - abs(input_raw);
            Gap = abs( Input - Setpoint ); 
            if (Gap > 60){
                Output = 0;
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
              if (pulse_width_right > Offset + 5){
                    //flag = -9;
                    angle = 0;
              }
              else angle = 70; //flag = 0.5;
              }
            else if (pulse_width_right < Offset - 5){
                angle = 180;//flag = 9;
            }
            else angle = 110; //flag = -0.5;
            //angle = flag*Output/255*10 + 90;
            wheels.write(angle);
            //Serial.print(angle);
            //Serial.print("\n");*/
            
          }
          else{
            //Serial.print("Go straight\n");
            esc.write(65);
            pulse_width_left = pulse_width_left/10; /* 10usec = 1 cm of distance for LIDAR-Lit */
            pulse_width_right = pulse_width_right/10;
            //Serial.print(pulse_width_left);
            //Serial.print("\n");
            //Serial.print(pulse_width_right);
            //Serial.print("\n");
            //delay(1000);

            input_raw = pulse_width_left - pulse_width_right;
            Input =0 - abs(input_raw);
            Gap = abs( Input - Setpoint ); 
            if (Gap > 60){
                Output=0;
            }
            else if(Gap>25) 
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
              if (pulse_width_right < Offset + 5){
                    flag = 2.5;
              }
              else flag = -0.25;
              }
            else if (pulse_width_right > Offset - 5){
                flag = -2;
            }
            else flag = 0.25;
            angle = flag*Output/255*10 + 90;
            wheels.write(angle);
            //Serial.print(angle);
            //Serial.print("\n");
            /*Serial.print(Output);
            Serial.print("\n");*/
            }
          
        }
      }
    
    else {
      //Serial.print("dis  < 40 ");
       //  Serial.print(distance);
      //Serial.print("\n");
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
  Serial.print("Turn_flag");
  Serial.print("/n");
  Serial.print(turn_flag);

}

void keep_straight() {
 
    turn_flag = 0;
    Serial.print("Turn_flag");
    Serial.print("/n");
    Serial.print(turn_flag);

}

 void set_crawler(String str){
    distance = analogRead(A0);
   //pulse_width_left = pulseIn(lidar_left, HIGH)/10;
   //pulse_width_right = pulseIn(lidar_right, HIGH)/10;
  int index = str.indexOf(',');
    String ss = str.substring(0, index);
    String aa = str.substring(index+1);
    
    
    int set_speed = ss.toInt();
    int set_angle = aa.toInt();
    /*Serial.print("speed ");
     Serial.print(set_speed);
    Serial.print("\n");
    Serial.print("angle ");
     Serial.print(set_angle);
    Serial.print("\n");*/
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

    if (Serial.available()) {
      String str = mySerial.readStringUntil(';');
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

        case 'T':
            turn();
            Serial.print("t");
            break;

        case 'K':
            keep_straight();
           // Serial.print("k");
            break;

        case 'S':
            
            str = str.substring(1);
            //Serial.print("String : " + str);
            //Serial.print("\n");
            set_crawler(str);
            
            break;

        default:
          Serial.print("Unknown Command");
      }
      //Serial.print("h");
       //Serial.print(command);
       //Serial.print("/n");
       Serial.print("control_mode"); 
      Serial.print(control_mode); 
    }
    
    delay(50);
    oscillate();

}
