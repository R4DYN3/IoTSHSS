#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// sensor level warning nums
enum sensor_warnings
{
  DANGER_GAS = 700,
  LOW_LIGHT = 300, 
  RAIN_WATER = 800,
  SOIL_HYDROPENIA = 50
};

//init servos:
Servo servo_10;
Servo servo_9;


struct SensorData {
    uint32_t gas;
    uint32_t light;
    uint32_t water;
    uint32_t soil;
    uint32_t PIR_motion;
};

// Function to read all sensor values and store them in a struct
SensorData readSensors() {
    SensorData data;
    data.gas = analogRead(A0);
    data.light = analogRead(A1);
    data.water = analogRead(A3);
    data.soil = analogRead(A2);
    data.PIR_motion = digitalRead(2);
    return data;
}

// this is a function that will send data serially for the uno wifi board to pick up
// the uno wifi board has two wires connected to pins 1 and 0 which are the serial output pins
void sendData(SensorData data, uint16_t warnings)
{
    // Send structured sensor data in a readable format
    Serial.print("Gas:"); Serial.print(data.gas);
    Serial.print(", Light:"); Serial.print(data.light);
    Serial.print(", Water:"); Serial.print(data.water);
    Serial.print(", Soil:"); Serial.println(data.soil);

    // Print out warnings based on the warning bits
    if (warnings & (1 << 13)) {
        Serial.println("Warning: Gas Danger");
    }

    if (warnings & (1 << 12)) {
        Serial.println("Warning: Low Light");
    }

    if (warnings & (1 << 11)) {
        Serial.println("Warning: High Water Level");
    }

    if (warnings & (1 << 10)) {
        Serial.println("Warning: Dry Soil (Hydropenia)");
    }

    delay(2000);  // Wait before sending data again
}
// Bit layout (16 bits):
//
// 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
//  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V
// Gas Light Rain Soil  motion[THE REST ARE RESERVERED / FUTURE USE]
// (1 = Warning, 0 = No Warning)



// Function to process the sensor data and perform actions
void setWarningBitfield(SensorData& data, uint16_t& warnings)
{
    // Reset all warning bits to clear previous state
    warnings = 0;

    // Gas sensor processing (set Gas Warning bit if gas exceeds threshold)
    if (data.gas > DANGER_GAS)
    {
        warnings |= (1 << 13);  // Set the Gas Warning bit (bit 13)
    }

    // Light sensor processing (set Light Warning bit if light is below threshold)
    if (data.light < LOW_LIGHT)
    {
        warnings |= (1 << 12);  // Set the Light Warning bit (bit 12)
    }

    // Water sensor processing (set Water Warning bit if water exceeds threshold)
    if (data.water > RAIN_WATER)
    {
        warnings |= (1 << 11);  // Set the Water Warning bit (bit 11)
    }

    // Soil sensor processing (set Soil Warning bit if soil moisture is below threshold)
    if (data.soil > SOIL_HYDROPENIA)
    {
        warnings |= (1 << 10);  // Set the Soil Warning bit (bit 10)
    }
    if (data.PIR_motion == 1) 
    {
        warnings |= (1 << 9); // set motion warning bit (bit 9)
    }
}

void set_roof_fan_on()
{
  digitalWrite(7, LOW);
  digitalWrite(6, HIGH);
}

void set_roof_fan_off()
{
  digitalWrite(7, LOW);
  digitalWrite(6, LOW);
}

// this activates the self preserving features of the house
// gas: turn on fan and play warning noise 
// low light: turn on a light
// high water: [todo]: idk if this needs an alarm warning??? 
// soil hydropenia: [todo]: I also don't think this needs an alarm
void active_sensor_mitigations(uint16_t warnings)
{

  // gas:
  if (warnings & (1 << 13))
  {
    set_roof_fan_on();
    Serial.println("reached this control flow");
    // [todo]: play the alarm 
  }
  else
  {
    set_roof_fan_off();  
  }
  
  // low light:
  if (warnings & (1 << 12)) 
  {
      digitalWrite(13, HIGH); //set digital 13 to high level, LED is on
  } 
  else
  {
      digitalWrite(13, LOW); //set digital 13 to low level, LED is off 
  }
  // high water level:
  if (warnings & (1 << 11))
  {
    // [todo]: future feature...
  }
  // soil hydropenia: 
  if (warnings & (1 << 10)) 
  {
    // [todo]: future feature...
  }
  // soil hydropenia: 
  if (warnings & (1 << 9)) 
  {
    // [todo]: lock door / close window, maybe play alarm idk

  }
}


//Set the communication address of I2C to 0x27, display 16 characters every line, two lines in total
LiquidCrystal_I2C mylcd(0x27, 16, 2);
void INITIALISE_LCD_SCREEN()
{
    mylcd.init();
    mylcd.backlight();//initialize LCD
    mylcd.setCursor(1 - 1, 1 - 1);
    mylcd.print("Enter password:");
}



//todo: document what each pin is connected to and what the sensors do.
void INITIALISE_IO_PINS()
{
  pinMode(7, OUTPUT);//set digital 7 to output
  pinMode(6, OUTPUT);//set digital 6 to output
  digitalWrite(7, HIGH); //set digital 7 to high level
  digitalWrite(6, HIGH); //set digital 6 to high level
  
  pinMode(4, INPUT);//set digital 4 to input
  pinMode(8, INPUT);//set digital 8 to input
  pinMode(2, INPUT);//set digital 2 to input [PIR motion sensor]
  pinMode(3, OUTPUT);//set digital 3 to output [buzzer: used for playing songs / alarms]
  pinMode(A0, INPUT);//set A0 to input
  pinMode(A1, INPUT);//set A1 to input
  pinMode(13, OUTPUT);//set digital 13 to input
  pinMode(A3, INPUT);//set A3 to input
  pinMode(A2, INPUT);//set A2 to input

  pinMode(12, OUTPUT);//set digital 12 to output
  pinMode(5, OUTPUT);//set digital 5 to output
}
void INITIALISE_SERVO_MOTORS()
{
  
  servo_9.attach(9);//make servo connect to digital 9
  servo_10.attach(10);//make servo connect to digital 10
  servo_9.write(0);//set servo connected digital 9 to 0°
  servo_10.write(0);//set servo connected digital 10 to 0°
}

/// ------------------------ songs and buzzer related stuff ------------------------ ///

enum zelda_NoteFrequency {
    NOTE_B0  = 31,
    NOTE_C1  = 33,  NOTE_CS1 = 35,  NOTE_D1  = 37,  NOTE_DS1 = 39,
    NOTE_E1  = 41,  NOTE_F1  = 44,  NOTE_FS1 = 46,  NOTE_G1  = 49,
    NOTE_GS1 = 52,  NOTE_A1  = 55,  NOTE_AS1 = 58,  NOTE_B1  = 62,

    NOTE_C2  = 65,  NOTE_CS2 = 69,  NOTE_D2  = 73,  NOTE_DS2 = 78,
    NOTE_E2  = 82,  NOTE_F2  = 87,  NOTE_FS2 = 93,  NOTE_G2  = 98,
    NOTE_GS2 = 104, NOTE_A2  = 110, NOTE_AS2 = 117, NOTE_B2  = 123,

    NOTE_C3  = 131, NOTE_CS3 = 139, NOTE_D3  = 147, NOTE_DS3 = 156,
    NOTE_E3  = 165, NOTE_F3  = 175, NOTE_FS3 = 185, NOTE_G3  = 196,
    NOTE_GS3 = 208, NOTE_A3  = 220, NOTE_AS3 = 233, NOTE_B3  = 247,

    NOTE_C4  = 262, NOTE_CS4 = 277, NOTE_D4  = 294, NOTE_DS4 = 311,
    NOTE_E4  = 330, NOTE_F4  = 349, NOTE_FS4 = 370, NOTE_G4  = 392,
    NOTE_GS4 = 415, NOTE_A4  = 440, NOTE_AS4 = 466, NOTE_B4  = 494,

    NOTE_C5  = 523, NOTE_CS5 = 554, NOTE_D5  = 587, NOTE_DS5 = 622,
    NOTE_E5  = 659, NOTE_F5  = 698, NOTE_FS5 = 740, NOTE_G5  = 784,
    NOTE_GS5 = 831, NOTE_A5  = 880, NOTE_AS5 = 932, NOTE_B5  = 988,

    NOTE_C6  = 1047, NOTE_CS6 = 1109, NOTE_D6  = 1175, NOTE_DS6 = 1245,
    NOTE_E6  = 1319, NOTE_F6  = 1397, NOTE_FS6 = 1480, NOTE_G6  = 1568,
    NOTE_GS6 = 1661, NOTE_A6  = 1760, NOTE_AS6 = 1865, NOTE_B6  = 1976,

    NOTE_C7  = 2093, NOTE_CS7 = 2217, NOTE_D7  = 2349, NOTE_DS7 = 2489,
    NOTE_E7  = 2637, NOTE_F7  = 2794, NOTE_FS7 = 2960, NOTE_G7  = 3136,
    NOTE_GS7 = 3322, NOTE_A7  = 3520, NOTE_AS7 = 3729, NOTE_B7  = 3951,

    NOTE_C8  = 4186, NOTE_CS8 = 4435, NOTE_D8  = 4699, NOTE_DS8 = 4978,

    REST = 0 // Used for pauses
};
int zelda_melody[] = {

  //Based on the arrangement at https://www.flutetunes.com/tunes.php?id=169
  
  NOTE_AS4,-2,  NOTE_F4,8,  NOTE_F4,8,  NOTE_AS4,8,//1
  NOTE_GS4,16,  NOTE_FS4,16,  NOTE_GS4,-2,
  NOTE_AS4,-2,  NOTE_FS4,8,  NOTE_FS4,8,  NOTE_AS4,8,
  NOTE_A4,16,  NOTE_G4,16,  NOTE_A4,-2,
  REST,1, 

  NOTE_AS4,4,  NOTE_F4,-4,  NOTE_AS4,8,  NOTE_AS4,16,  NOTE_C5,16, NOTE_D5,16, NOTE_DS5,16,//7
  NOTE_F5,2,  NOTE_F5,8,  NOTE_F5,8,  NOTE_F5,8,  NOTE_FS5,16, NOTE_GS5,16,
  NOTE_AS5,-2,  NOTE_AS5,8,  NOTE_AS5,8,  NOTE_GS5,8,  NOTE_FS5,16,
  NOTE_GS5,-8,  NOTE_FS5,16,  NOTE_F5,2,  NOTE_F5,4, 

  NOTE_DS5,-8, NOTE_F5,16, NOTE_FS5,2, NOTE_F5,8, NOTE_DS5,8, //11
  NOTE_CS5,-8, NOTE_DS5,16, NOTE_F5,2, NOTE_DS5,8, NOTE_CS5,8,
  NOTE_C5,-8, NOTE_D5,16, NOTE_E5,2, NOTE_G5,8, 
  NOTE_F5,16, NOTE_F4,16, NOTE_F4,16, NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,8, NOTE_F4,16,NOTE_F4,8,

  NOTE_AS4,4,  NOTE_F4,-4,  NOTE_AS4,8,  NOTE_AS4,16,  NOTE_C5,16, NOTE_D5,16, NOTE_DS5,16,//15
  NOTE_F5,2,  NOTE_F5,8,  NOTE_F5,8,  NOTE_F5,8,  NOTE_FS5,16, NOTE_GS5,16,
  NOTE_AS5,-2, NOTE_CS6,4,
  NOTE_C6,4, NOTE_A5,2, NOTE_F5,4,
  NOTE_FS5,-2, NOTE_AS5,4,
  NOTE_A5,4, NOTE_F5,2, NOTE_F5,4,

  NOTE_FS5,-2, NOTE_AS5,4,
  NOTE_A5,4, NOTE_F5,2, NOTE_D5,4,
  NOTE_DS5,-2, NOTE_FS5,4,
  NOTE_F5,4, NOTE_CS5,2, NOTE_AS4,4,
  NOTE_C5,-8, NOTE_D5,16, NOTE_E5,2, NOTE_G5,8, 
  NOTE_F5,16, NOTE_F4,16, NOTE_F4,16, NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,8, NOTE_F4,16,NOTE_F4,8
  
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes=sizeof(zelda_melody)/sizeof(zelda_melody[0])/2; 

// this calculates the duration of a whole note in ms (60s/tempo)*4 beats
int tempo=88; 
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;


enum ode_to_joy_NoteFrequency {
    D0 = -1,
    D1 = 262,
    D2 = 293,
    D3 = 329,
    D4 = 349,
    D5 = 392,
    D6 = 440,
    D7 = 494,
    M1 = 523,
    M2 = 586,
    M3 = 658,
    M4 = 697,
    M5 = 783,
    M6 = 879,
    M7 = 987,
    H1 = 1045,
    H2 = 1171,
    H3 = 1316,
    H4 = 1393,
    H5 = 1563,
    H6 = 1755,
    H7 = 1971
};

ode_to_joy_NoteFrequency ode_to_joy_tune[] = {
    M3, M3, M4, M5,
    M5, M4, M3, M2,
    M1, M1, M2, M3,
    M3, M2, M2,
    M3, M3, M4, M5,
    M5, M4, M3, M2,
    M1, M1, M2, M3,
    M2, M1, M1,
    M2, M2, M3, M1,
    M2, M3, M4, M3, M1,
    M2, M3, M4, M3, M2,
    M1, M2, D5, D0,
    M3, M3, M4, M5,
    M5, M4, M3, M4, M2,
    M1, M1, M2, M3,
    M2, M1, M1
};
//set music beat
float ode2joy_durt[] =
{
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1 + 0.5, 0.5, 1 + 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1 + 0.5, 0.5, 1 + 1,
  1, 1, 1, 1,
  1, 0.5, 0.5, 1, 1,
  1, 0.5, 0.5, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 0.5, 0.5,
  1, 1, 1, 1,
  1 + 0.5, 0.5, 1 + 1,
};
int length = sizeof(ode_to_joy_tune) / sizeof(ode_to_joy_tune[0]); 
int tonepin = 3;
void Ode_to_Joy()//play Ode to joy song
{
  for (int x = 0; x < length; x++)
  {
    tone(tonepin, ode_to_joy_tune[x]);
    delay(300 * ode2joy_durt[x]); // todo: switch to mili to avoid delaying rest of program 
  }
  noTone(3);
}

void zelda()
{
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = zelda_melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(tonepin, zelda_melody[thisNote], noteDuration*0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(tonepin);
  }
}

char serial_input;
void user_control() {
  switch (serial_input)
  {
    case 'f':
      Serial.println("play a song");
      Ode_to_Joy(); //todo make non blocking
      break;
    case 'g':
      zelda(); //todo: make non blocking 
      break;
    case 'r': 
      set_roof_fan_on();
      break;
    case 's':
      set_roof_fan_off();
      break;
    // case 't'://if val is 't'，program will circulate
    //   servo1 = Serial.readStringUntil('#');
    //   servo1_angle = String(servo1).toInt();
    //   //servo_9.write(servo1_angle);//set the angle of servo connected to digital 9 to servo1_angle
    //   delay(300);
    //   break;//exit loop
    // case 'u'://if val is 'u'，program will circulate
    //   servo2 = Serial.readStringUntil('#');
    //   servo2_angle = String(servo2).toInt();
    //   //servo_10.write(servo2_angle);//set the angle of servo connected to digital 10 to servo2_angle
    //   delay(300);
    //   break;//exit loop
    case 'v'://if val is 'v'，program will circulate
      String led2 = Serial.readStringUntil('#');
      volatile int value_led2 = String(led2).toInt();
      analogWrite(5, value_led2); //PWM value of digital 5 is value_led2
      break;//exit loop
    case 'w'://if val is 'w'，program will circulate
      Serial.println("reached fan pwm_control");
      String fans_char = Serial.readStringUntil('#');
      volatile int fans_val = String(fans_char).toInt();
      digitalWrite(7, LOW);
      analogWrite(6, fans_val); //set PWM value of digital 6 to fans_val，the larger the value, the faster the fan
      break;//exit loop
  }
}

// controls door with password on front 
void lock_door() {
  volatile int button1 = digitalRead(4);// assign the value of digital 4 to button1
  volatile int button2 = digitalRead(8);//assign the value of digital 8 to button2
  volatile int btn1_num;
  volatile int btn2_num;

  String passwd;
  String pass;

  if (button1 == 0)//if variablebutton1 is 0
  {
    delay(10);//delay in 10ms
    while (button1 == 0) //if variablebutton1 is 0，program will circulate
    {
      button1 = digitalRead(4);// assign the value of digital 4 to button1
      btn1_num = btn1_num + 1;//variable btn1_num plus 1
      delay(100);// delay in 100ms
    }

  }
  if (btn1_num >= 1 && btn1_num < 5) //1≤if variablebtn1_num<5
  {
    Serial.print(".");
    Serial.print("");
    passwd = String(passwd) + String(".");//set passwd 
    pass = String(pass) + String(".");//set pass
    //LCD shows pass at the first row and column
    mylcd.setCursor(2 - 1, 2 - 1);
    mylcd.print(pass);
  }
  if (btn1_num >= 5)
    //if variablebtn1_num ≥5
  {
    Serial.print("-");
    passwd = String(passwd) + String("-");//Set passwd 
    pass = String(pass) + String("-");//set pass
    //LCD shows pass at the first row and column
    mylcd.setCursor(1 - 1, 2 - 1);
    mylcd.print(pass);

  }
  if (button2 == 0) //if variablebutton2 is 0
  {
    delay(10);
    if (button2 == 0)//if variablebutton2 is 0
    {
      if (passwd == ".--.-.")//if passwd is ".--.-."
      {
        mylcd.clear();//clear LCD screen
        //LCD shows "open!" at first character on second row
        mylcd.setCursor(1 - 1, 2 - 1);
        mylcd.print("open!");
        servo_9.write(100);//set servo connected to digital 9 to 100°
        delay(300);
        delay(5000);
        passwd = "";
        pass = "";
        mylcd.clear();//clear LCD screen
        //LCD shows "password:"at first character on first row
        mylcd.setCursor(1 - 1, 1 - 1);
        mylcd.print("password:");

      } else //Otherwise
      {
        mylcd.clear();//clear LCD screen
        //LCD shows "error!"at first character on first row
        mylcd.setCursor(1 - 1, 1 - 1);
        mylcd.print("error!");
        passwd = "";
        pass = "";
        delay(2000);
        //LCD shows "again" at first character on first row
        mylcd.setCursor(1 - 1, 1 - 1);
        mylcd.print("again");
      }
    }
  }
  //int infrar_motion = digitalRead(2);//assign the value of digital 2 to infrar
  //if (infrar_motion == 0 && (val != 'l' && val != 't'))
    //if variable infrar is 0 and val is not 'l' either 't'
  // {
  //   //servo_9.write(0);//set servo connected to digital 9 to 0°
  //   delay(50);
  // }
  // if (button2 == 0)//if variablebutton2 is 0
  // {
  //   delay(10);
  //   while (button2 == 0) //if variablebutton2 is 0，program will circulate
  //   {
  //     button2 = digitalRead(8);//assign the value of digital 8 to button2
  //     btn2_num = btn2_num + 1;//variable btn2_num plus 1
  //     delay(100);
  //     if (btn2_num >= 15)//if variablebtn2_num ≥15
  //     {
  //       tone(3, 532);
  //       delay(125);
  //       mylcd.clear();//clear LCD screen
  //       //LCD shows "password:" at the first character on first row
  //       mylcd.setCursor(1 - 1, 1 - 1);
  //       mylcd.print("password:");
  //       //LCD shows "wait" at the first character on first row
  //       mylcd.setCursor(1 - 1, 1 - 1);
  //       mylcd.print("wait");
  //     } else//Otherwise
  //     {
  //       noTone(3);//digital 3 stops playing music
  //     }
   // }

  //}
  btn1_num = 0;//set btn1_num to 0
  btn2_num = 0;//set btn2_num to 0
}



void setup() {
    Serial.begin(9600); // Use hardware Serial on pins 0,1
    INITIALISE_LCD_SCREEN();
    INITIALISE_IO_PINS();
    INITIALISE_SERVO_MOTORS();

    // want to test all the sensors and make sure they work: (todo): perhaps bind to a key? make it's own func

} 

uint16_t warning_bitfield = 0; //todo put this in it's appropriate place
void loop() {
    if (Serial.available() > 0) //serial reads the characters
    {
      serial_input = Serial.read();//set val to character read by serial    Serial.println(val);//output val character in new lines
      Serial.println(serial_input);
      user_control(); // blutooth functionality + can send data via serial to communicate with device
      Serial.println("reached serial available");
    }
    SensorData data = readSensors();            
    setWarningBitfield(data, warning_bitfield);        
    sendData(data, warning_bitfield);
    active_sensor_mitigations(warning_bitfield);

    lock_door();
}