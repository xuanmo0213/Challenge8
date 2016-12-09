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
int lidar_left = 3;
int lidar_right = 4;

/* -----------------PIDs----------------- */
int angle, control_mode;
double Setpoint, Input, Output, Gap, input_raw, Offset, c_output, d_diff;
double aggKp=100, aggKi=0.0005, aggKd=0.15; 
double consKp=8, consKi=0.00005, consKd=3;
double flag = 1;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
/* ----------------Sonar----------------- */
double distance;


/** -----------Setup Function----------- **/
void setup(){
  control_mode = 1;
  /*Serial.print(control_mode);*/
  /* Measure when setup */
  pulse_width_left = pulseIn(3, HIGH);
  pulse_width_right = pulseIn(4, HIGH);
  delay(50);
  /*Input = abs(pulse_width_left-pulse_width_right)/10;*/
  Offset = (pulse_width_left + pulse_width_right)/20;
  Setpoint = 0;
  /*Gap = abs(Setpoint-Input);*/
  myPID.SetMode(AUTOMATIC);
  /* Assign I/O pin */
  wheels.attach(6);
  esc.attach(7); // initialize ESC to Digital IO Pin #9
  /* Set wheels to default */
  wheels.write(90);
  /* Set sensor input */
  pinMode(lidar_left, INPUT); // Set pin 3 as monitor pin
  pinMode(lidar_right, INPUT);
  /* Start Serial Port */
  Serial.begin(9600);
  mySerial.begin(9600);
  /* Init ESC */
  calibrateESC();
}

 void calibrateESC(){
    esc.write(180); // full backwards
    delay(startupDelay);
    esc.write(0);
    delay(startupDelay);
    esc.write(90);
    delay(startupDelay);
    esc.write(90);
 }
 
  void oscillate(){
    distance = analogRead(A0);
    /*Serial.print(distance);
    Serial.print(control_mode);*/
    
    if (distance >= 30 && control_mode == 1){
        esc.write(75);
        pulse_width_left = pulseIn(lidar_left, HIGH);
        pulse_width_right = pulseIn(lidar_right, HIGH);
            //Serial.print(pulse_width_left);
            //Serial.print("\n");
            //Serial.print(pulse_width_right);
            //Serial.print("\n");
            //delay(1000);
        if ( pulse_width_left !=0 && pulse_width_right != 0 && pulse_width_left <= 5000 && pulse_width_right <= 5000){
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
                if (pulse_width_right < Offset + 5)
                flag = 2.5;
                else flag = -0.25;
            }
            else if (pulse_width_right > Offset - 5){
                flag = -2;
            }
            else flag = 0.25;
            angle = flag*Output/255*10 + 90;
            wheels.write(angle);
            Serial.print(angle);
            Serial.print("\n");
            /*Serial.print(Output);
            Serial.print("\n");*/
            }
        }
    
    else {
     esc.write(90);
    }

  }
  
void control_drive() {
    control_mode = 0;

}

  
void auto_drive() {
    control_mode = 1;

}

void turn() {
 // if(angle<180) {
  //  esc.write(75);
  //}
  //else {
    //esc.write(100);
 // }


}

void keep_straight() {
    

}

void set_crawler(){
    
}

void loop(){
    char command;
    if (mySerial.available()) {
      command = mySerial.read();
      
      switch(command){
            case 'A':
            auto_drive();
            break;

        case 'C':
            control_drive();
            break;

        case 'T':
            turn();
            break;

        case 'K':
            keep_straight();
            break;

        case 'S':
            set_crawler();
            break;

        default:
            Serial.print("Unknown Command");
      }
    }
    delay(50);
    oscillate();

}
