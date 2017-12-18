#include <Servo.h> /* lib servo build-in rev. 1.1.2 */
#include <Wire.h>
#include "Rtc_Pcf8563.h"

/* declare two motor and one RTC module */
Servo myservoG;
Servo myservoD;
Rtc_Pcf8563 rtc;
/* Pinout */
#define potar_0 A0 /* potentiometer on pin A0 */
#define potar_1 A1 /* potentiometer on pin A1 */
#define potar_2 A2 /* potentiometer on pin A2 */
#define potar_3 A3 /* potentiometer on pin A3 */
#define on_off_0 7 /* on off button on pin D7 */
#define on_off_1 6 /* on off button on pin D6 */
#define on_off_2 5 /* on off button on pin D5 */
#define led     13 /* internal led of Nano board wired to D13 */
#define i2c_sda A4 /* i2c configuration for RTC module */
#define i2c_scl A5 /* i2c configuration for RTC module */
#define cmd_motor_left 9 /* motor left on pin D9 */
#define cmd_motor_right 10 /* motor right on pin D10 */

#define TEMPO_SCHEDULER 1    /* tempo in minute */
#define TEMPO_Hx_AVAILABLE 60 /* Must be twice bigger than TEMPO_Hx_AVAILABLE */
#define TEMPO_Hx_IS_TIME 15
#define TEMPO_H0_IS_TIME 5
#define MAX_RATION 10

#define POS_MOTORG_MIN 180
#define POS_MOTORG_MAX  0

unsigned long t0_main;
unsigned long t0_H1;
unsigned long t0_H2;
uint8_t H1_available;
uint8_t H2_available;
uint8_t H0_available;

uint8_t current;

void setup() {
  t0_main=0;
  t0_H1=0;
  t0_H2=0;
  H1_available=true;
  H2_available=true;
  H0_available=false;
  myservoG.attach(cmd_motor_left,700,2000); /* cmd motor left pin 9 */ 
  myservoD.attach(cmd_motor_right,700,2000); /* cmd motor right pin 10 */ 
  pinMode(on_off_0,INPUT_PULLUP);
  pinMode(on_off_1,INPUT_PULLUP);
  pinMode(on_off_2,INPUT_PULLUP);
  ServoInit();
}

void loop() {
 if(elapsedTime(&t0_main,TEMPO_SCHEDULER)==true){
  /*H1 management */
  if(H1_available==true){
    /* H1 is available */
    /* Does interruptor is ON */
    if(digitalRead(on_off_1)==(!true)){
     /* interruptor is On*/
     /* check if it's time to give launch */
     if(isItTime(potar_1,TEMPO_Hx_IS_TIME)==1){
      /* give launch */
      /* Servo activation reading potar 3 to konw how much should be given*/
      for(int i=0; i<(potar3GetFeed()*10);i++){
        ServoWrite(POS_MOTORG_MIN);
        ServoWrite(POS_MOTORG_MAX);
        delay(500);
        ServoWrite(POS_MOTORG_MIN);
      }
      H1_available=false;
      setElapsedTime(&t0_H1);  
     }
    }   
  }else{
    /* H1 not available */
    /* Check if time is elapsed */
    if(elapsedTime(&t0_H1,TEMPO_Hx_AVAILABLE)==true){
      H1_available=true;
    }
  }

  if(H2_available==true){
    /* H2 is available */
    /* Does interruptor is ON */
    if(digitalRead(on_off_2)==(!true)){
     /* interrputor is On*/
     /* check if it's time to give launch */
     if(isItTime(potar_2,TEMPO_Hx_IS_TIME)==1){
      /* give launch */
      /* Servo activation reading potar 3 to konw how much should be given*/
      for(int j=0; j<(potar3GetFeed()*10);j++){
        ServoWrite(POS_MOTORG_MIN);
        ServoWrite(POS_MOTORG_MAX);
        delay(500);
        ServoWrite(POS_MOTORG_MIN);
      }
      H2_available=false;
      setElapsedTime(&t0_H2);  
     }
    }   
  }else{
    /* H2 not available */
    /* Check if time is elapsed */
    if(elapsedTime(&t0_H2,TEMPO_Hx_AVAILABLE)==true){
      H2_available=true;
    }
  }
 }

 /* Turn on Nano led when potar_0 is synchronized with RTC module hour */
 if(isItTime(potar_0,TEMPO_H0_IS_TIME)==true){
  digitalWrite(led,1);
 }else{
  digitalWrite(led,0);
 }

 if(digitalRead(on_off_0)==(!true)){/* read btn ON */
  /* Set hour */
  if(H0_available==true){
   rtc.setTime(potarGethour(potar_0)/60,potarGethour(potar_0)%60,0);
   H0_available=false;        
  }
 }
 if(digitalRead(on_off_0)==(!false)){/* read btn OFF */
  H0_available=true;
 }
}

void ServoWrite(uint16_t data){
  if((data>=0) && (data<=180)){
    int16_t temp;
    if(data>current){
      while(data>current){
        current++;
        myservoG.write(current);
        temp=(int16_t)((uint16_t)current);
        temp=(temp-180)*(-1);
        if(temp<0){temp=0;}
        if(temp>180){temp=180;}
        myservoD.write(temp);
        delay(30);
      }
    }else{
      while(data<current){
        current--;
        myservoG.write(current);
        temp=(int16_t)((uint16_t)current);
        temp=(temp-180)*(-1);
        if(temp<0){temp=0;}
        if(temp>180){temp=180;}
        myservoD.write(temp);
        delay(30);
      }
    } 
    current=data;
  }
}

void ServoInit(void){
 myservoG.write(POS_MOTORG_MIN);
 myservoD.write(POS_MOTORG_MAX);
 current=POS_MOTORG_MIN;
 delay(1000);  
}

uint32_t potar3GetFeed(void){
  uint32_t  read_analog;
  uint32_t min_r=1026;
  uint32_t max_r=9935;
  max_r=max_r-min_r;
  read_analog=((uint32_t)analogRead(potar_3)*500*21/1023);
  if(read_analog>=min_r){
    read_analog=read_analog-min_r;
  }else{
    read_analog=0;
  }
  if(read_analog>=max_r){read_analog=max_r;}
  read_analog=((uint64_t)((uint64_t)read_analog*MAX_RATION)/max_r);
  return (read_analog+1);
}

/* return in minute the time of corresponding potentiometer */
/* return value is in the range [360..1320], which means 6am to 10pm*/
/* input value : {A0,A1,A2} -> {potar_0, potar_1, potar_2} */
uint32_t potarGethour(uint8_t pin_number){
  uint8_t   pin;
  uint32_t  read_analog;
  uint32_t min_r=1026;
  uint32_t max_r=9935;
  max_r=max_r-min_r;
  read_analog=((uint32_t)analogRead(pin_number)*500*21/1023);
  if(read_analog>=min_r){
    read_analog=read_analog-min_r;
  }else{
    read_analog=0;
  }
  if(read_analog>=max_r){read_analog=max_r;}
  read_analog=((uint64_t)((uint64_t)read_analog*960)/max_r)+360;
  return read_analog;
}

uint32_t isItTime(uint8_t pin_number,uint32_t delta_time){
  uint32_t potar_value;
  uint32_t time_value;
  uint32_t delta;
  potar_value=potarGethour(pin_number);
  time_value=rtc.getMinute()+(rtc.getHour()*60);
  if(potar_value>=time_value){
    delta=potar_value-time_value;
  }else{
    delta=time_value-potar_value;
  }
  if(delta<=delta_time){
    return 1;
  }else{
    return 0;
  }
}

uint8_t elapsedTime(unsigned long* t0,unsigned long minute){
  unsigned long t1;
  uint8_t return_value;
  t1=millis();
  if((t1-*t0)>=(minute*60*1000)){
    *t0=millis(); /* needed? */
    return_value=1;
  }else{
    return_value=0;
  }
  return return_value;
}

void setElapsedTime(unsigned long * to){
  *to=millis();
}

