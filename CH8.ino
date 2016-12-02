#include<SoftwareSerial.h>
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
int angle, c_angle;
double Setpoint, Input, Output, Gap, input_raw, Offset, c_output, d_diff;
double aggKp=100, aggKi=0.0005, aggKd=0.15; 
double consKp=8, consKi=0.00005, consKd=3;
double flag = 1, c_flag = 1;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, PID::DIRECT);
/* ----------------Sonar----------------- */
double distance;


/** -----------Setup Function----------- **/
void setup(){
  pulse_width_left = pulseIn(D3, HIGH);
  pulse_width_right = pulseIn(D4, HIGH);
  delay(50);
  Input = abs(pulse_width_left-pulse_width_right)/10;
  Offset = (pulse_width_left + pulse_width_right)/20;
  Setpoint = 0;
  Gap = abs(Setpoint-Input);
  myPID.SetMode(PID::AUTOMATIC);
  wheels.attach(0);
  esc.attach(1); // initialize ESC to Digital IO Pin #9
  wheels.write(90);
  pinMode(lidar_front, INPUT); // Set pin 3 as monitor pin
  pinMode(lidar_back, INPUT);
  Serial.begin(9600);
  mySerial.begin(9600);
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
    if (distance >= 300 && control == 1){
        esc.write(60);
        pulse_width_left = pulseIn(lidar_left, HIGH);
        pulse_width_right = pulseIn(lidar_right, HIGH);
        if ( pulse_width_left !=0 && pulse_width_right != 0 && pulse_width_front <= 5000 && pulse_width_front <= 5000){
            pulse_width_front = pulse_width_front/10; /* 10usec = 1 cm of distance for LIDAR-Lit */
            pulse_width_back = pulse_width_back/10;
            
            input_raw = pulse_width_front - pulse_width_back;
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
                if (pulse_width_front > Offset + 5)
                flag = -1.5;
                else flag = 0.25;
            }
            else if (pulse_width_front < Offset - 5){
                flag = 2;
            }
            else flag = -0.25;
            angle = flag*Output/255*10 + 94;
            wheels.write(angle);

            }
        }
    
    else {
     esc.write(90);
    }

  }                                                                                                                                                                                                              

void loop(){
    if (mySerial.available()) {
          char command;
    if (mySerial.available()) {
      command = mySerial.read();
      switch(command){
            case 'A':
            auto_drive();
            break;

        case 'C':
            control();
            break;

        case 'T':
            turn();
            break;

        case 'K':
            keep_straight();
            break;

        case 'S':
            set_speed();
            break;

        case 'W':
            adjust_wheel();
            break;

        case 'B':
            go_backward();
            break;

        default:
            print("Unknown Command");
      }
    }
    delay(50);
    oscillate();
}

int control(String str) {
    control = 0;
    return 1;
}

int auto(String str) {
  control =1 ;
  return 0;
}

