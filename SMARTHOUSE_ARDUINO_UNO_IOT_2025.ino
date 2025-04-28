#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// sensor level warning nums
enum sensor_warnings
{
  DANGER_GAS = 1000,
  LOW_LIGHT = 300, 
  RAIN_WATER = 800,
  SOIL_HYDROPENIA = 50
};

//init servos:
Servo servo_10;
Servo servo_9;

#pragma pack(1)
struct SensorData {
    uint32_t gas;
    uint32_t light;
    uint32_t water;
    uint32_t soil;
    uint32_t PIR_motion;
    uint16_t warnings;
};
#pragma pop


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
void sendData(SensorData data)
{
    Serial.write(0xAA);  // Start marker
    Serial.write((uint8_t*)&data, sizeof(SensorData));  // Send struct as bytes
    Serial.write(0x55);  // End marker
    Serial.flush();  // Ensure all data is sent before delay
    delay(2000);  // Wait before sending data again

}
// Bit layout (16 bits):
//
// 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
//  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V  V
// Gas Light Rain Soil  motion[THE REST ARE RESERVERED / FUTURE USE]
// gas: 15
// Light: 14
// Rain: 13
// Soil: 12
// Motion: 11
// Incorrect pass: 10

// .
// .
// .
// relay status button: 04 
// Send water signal via pushover: 03
// Send soil signal via pushover: 02
// Send gas signal via pushover: 01
// Send light signal via pushover: 00
// (1 = Warning, 0 = No Warning)


// Function to process the sensor data and perform actions
void setWarningBitfield(SensorData& data, uint16_t notifications)
{
    // Reset all warning bits to clear previous state
    data.warnings = 0;
    //Serial.println("Notification inside set warning: " + String(notifications));

    // Gas sensor processing (set Gas Warning bit if gas exceeds threshold)
    if (data.gas > DANGER_GAS)
    {
        data.warnings |= (1 << 13);  // Set the Gas Warning bit (bit 13)
    }

    // Light sensor processing (set Light Warning bit if light is below threshold)
    if (data.light < LOW_LIGHT)
    {
        data.warnings |= (1 << 12);  // Set the Light Warning bit (bit 12)
    }

    // Water sensor processing (set Water Warning bit if water exceeds threshold)
    if (data.water > RAIN_WATER)
    {
        data.warnings |= (1 << 11);  // Set the Water Warning bit (bit 11)
    }

    // Soil sensor processing (set Soil Warning bit if soil moisture is below threshold)
    if (data.soil > SOIL_HYDROPENIA)
    {
        data.warnings |= (1 << 10);  // Set the Soil Warning bit (bit 10)
    }
    if (data.PIR_motion == 1) 
    {
        data.warnings |= (1 << 9); // set motion warning bit (bit 9)
    }
    // // or it with notifications to get any notifications to send:
    data.warnings |= notifications;
    return;
}
// ------------------------//
// helper funcs for adjustable things on the house
void set_roof_fan_on()
{
    digitalWrite(7, LOW);
    digitalWrite(6, HIGH);
}

void set_roof_fan_off()
{
    digitalWrite(7, LOW);
    digitalWrite(6, LOW);
    return;
}

void turn_white_led_on()
{
    digitalWrite(13, HIGH); 
    return;

}

void turn_white_led_off()
{
    digitalWrite(13, LOW); 
    return;
}
void turn_yellow_led_on()
{
    digitalWrite(5, HIGH); 
    return;
}
void turn_yellow_led_off()
{
    digitalWrite(5, LOW); 
    return;
}


void open_Window()
{
    servo_9.write(100);
    return;
}

void close_Window()
{
    servo_9.write(0);
    return;
}

void open_Door()
{
    servo_10.write(180);
    return;
}

void close_Door()
{
    servo_10.write(0);
    return;
}
// ------------------------//
// ------------------------//

void sound_alarm()
{
    tone(3, 440);
      delay(125);
      delay(100);
      noTone(3);
      delay(100);
      tone(3, 440);
      delay(125);
      delay(100);
      noTone(3);
      delay(300);
      return;
}

void sound_alarm_soil()
{
    tone(3, 999);
      delay(125);
      delay(100);
      noTone(3);
      delay(100);
      tone(3, 999);
      delay(125);
      delay(100);
      noTone(3);
      delay(300);
      return;
}


// Function: active_sensor_mitigations
// Purpose: This activates the self preserving features of the house
// thresholds: Thresholds set in the sensor_warnings enum at the start of the code.
// gas level above threshold: turn on fan and play warning noise 
// low light: turn on the white LED light
// high water: if the window is open, close it. 
// soil hydropenia: play another alarm. 

void active_sensor_mitigations(uint16_t warnings)
{

  // gas:
  if (warnings & (1 << 13))
  {
    set_roof_fan_on();
    sound_alarm();
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
    close_Window();
    // [todo]: future feature...
  }
  // soil hydropenia: 
  if (warnings & (1 << 10)) 
  {
    sound_alarm_soil();
    // [todo]: future feature...
  }
  // Motion detected: 
  if (warnings & (1 << 9)) 
  {
    Serial.print("Motion detected!!!!!!!!!!!!!!!!!");
    close_Door();
    close_Window();
  }
  // todo: incorrect password attempt: 
  if (warnings & (1 << 8)) 
  {
    Serial.print("Incorrect password detected!!!!!!!!!!!!!!!!!");
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
    mylcd.print("Good day :D");
}


const int BUTTON_INPUT = 4;
const int CONFIRM_BUTTON = 8;
//todo: document what each pin is connected to and what the sensors do.
void INITIALISE_IO_PINS()
{
  pinMode(7, OUTPUT);//set digital 7 to output
  pinMode(6, OUTPUT);//set digital 6 to output
  digitalWrite(7, HIGH); //set digital 7 to high level
  digitalWrite(6, HIGH); //set digital 6 to high level
  
  pinMode(BUTTON_INPUT, INPUT_PULLUP);
  pinMode(CONFIRM_BUTTON, INPUT_PULLUP);
  //pinMode(4, INPUT);//set digital 4 to input
  //pinMode(8, INPUT);//set digital 8 to input
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
  return;
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

    // Wait for the specific duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(tonepin);
  }
  return;
}


String serial_buffer;
uint16_t parse_serial_command(char input, SensorData* data) 
{
  uint16_t notifications = 0; // lower half of this used for notifications, will be combined with warnings
  char cmd = input;
  Serial.println("command: " + char(cmd));
  //String args = input.substring(1);  // everything after the command

// For slider commands with format: "<char> <value> #" ~ how the app sends data to the bluetooth module
  if (cmd == 'v' || cmd == 'w' || cmd == 't' || cmd == 'u') {
    Serial.println("Parsing slider command...");

    // Read characters until we hit '#'
    char buffer[6] = {0}; // to hold up to 5-digit values (safe margin)
    byte idx = 0;
    char chr;

    // Read until '#' or buffer is full
    while (idx < 5) 
    {
      while (!Serial.available()); // wait for input
      chr = Serial.read();
      if (chr == '#') break;
      if (chr >= '0' && chr <= '9') 
      {
        buffer[idx++] = chr;
      }
    }
    buffer[idx] = '\0'; // Null terminate
    int value = atoi(buffer);  // convert string to integer

    Serial.print("Parsed value: ");
    Serial.println(value);

    switch(input)
    {
      case 't': // door open angle
        // easy!
        servo_10.write(value);
        break;
      case 'u': // window open angle
        // easy!
        servo_9.write(value);
        break;
      case 'v': // yellow LED brightness
        analogWrite(5, value);
        break;
      case 'w': // Fan rotation speed
        analogWrite(6, value);
        break;
    }
    return 0;
  }

  switch (cmd)
  {
    case 'w':
      Serial.println("Turn the fans on slider");
    case 'a': // turn white led on
      turn_white_led_on();
      break;
    case 'b':
      turn_white_led_off();
      break;
    case 'h': //photocell on, show data
      Serial.println("Sending photocell data...");
      notifications |= (1 << 0);
      break;
    // case 's': //photocell off, no more data [note]: on the app they have mapped this to two separate funcs lol, won't use here. 
    //   break;
    case 'j':   //soil related. 
      Serial.println("Sending soil data...");
      notifications |= (1 << 2);
      break; 
    case 'S':
      Serial.println("Sending soil data...");   //atm just does the same thing, 
      notifications |= (1 << 2);
      break;    // soil related once again... 
    case 'i': // gas sensor info
      notifications |= (1 << 1);
      break;
    case 'k': // water sensor info
      notifications |= (1 << 3);
      break;
    case 'n':   // open window
      open_Window();
      break;
    case 'o':   // close window
      close_Window();
      break;
    case 'l':   // open door 
      open_Door();
      break;
    case 'm':   // close door. 
      close_Door();
      break;
    case 'p': // yellow light on
      turn_yellow_led_on();
      break;
    case 'q':   //yellow light off.
      turn_yellow_led_off();
      break;
    case 'f':
      Serial.println("play a song");
      Ode_to_Joy();  
      break;
    case 'g':
      zelda();  
      break;
    case 'r':
      Serial.println("turning fan on");
      set_roof_fan_on();
      break;
    case 's':
      set_roof_fan_off();
      break;
    case 'c':   
      notifications |= (1 << 4);
      break;
    case 'd':    
      notifications |= (1 << 5);
      break;
    default:
      Serial.print("Unknown command: ");
      Serial.println(cmd);
      break;
  }
  return notifications;
}

//String password = "..";
#define MAX_PASSWORD_LENGTH 16

char password[] = "..";
char userInput[MAX_PASSWORD_LENGTH + 1] = "";       // +1 for null terminator
char displayInput[MAX_PASSWORD_LENGTH + 1] = "";

void listenForPasswordInput(uint16_t* notifications) {
    static int pressDuration = 0;
    int inputIndex = 0;

    if (digitalRead(CONFIRM_BUTTON) == LOW) {
        delay(20);
        while (digitalRead(CONFIRM_BUTTON) == LOW);  // wait for release

        mylcd.clear();
        mylcd.setCursor(0, 1);
        mylcd.print("Enter a password!");

        inputIndex = 0;
        userInput[0] = '\0';
        displayInput[0] = '\0';

        Serial.println("Listening...");
        while (1) {
            // Listen for dot/dash input
            if (digitalRead(BUTTON_INPUT) == LOW) 
            {
                delay(10);
                pressDuration = 0;
                while (digitalRead(BUTTON_INPUT) == LOW) 
                {
                    pressDuration++;
                    delay(100);
                }
                if (inputIndex < MAX_PASSWORD_LENGTH) {
                    if (pressDuration < 5) {
                        userInput[inputIndex] = '.';
                        displayInput[inputIndex] = '.';
                        Serial.println(".");
                    } else {
                        userInput[inputIndex] = '-';
                        displayInput[inputIndex] = '-';
                        Serial.println("-");
                    }
                    inputIndex++;
                    userInput[inputIndex] = '\0';      // null-terminate string
                    displayInput[inputIndex] = '\0';   // null-terminate display
                    
                    mylcd.clear();
                    mylcd.setCursor(0, 1);
                    mylcd.print(displayInput);


                    //updateLCD(displayInput);
                    Serial.print("user input: ");
                    Serial.println(displayInput);
                }
            }
            // Confirm password entry
            if (digitalRead(CONFIRM_BUTTON) == LOW) {
                delay(20);
                while (digitalRead(CONFIRM_BUTTON) == LOW);
                break;
            }
        }
        validatePassword(userInput, notifications); // Pass userInput to validate
    }
}

void updateLCD(String content) 
{
  mylcd.clear();
  mylcd.setCursor(0, 1);
  Serial.println("content: " + content);
  mylcd.print(content);
}

void validatePassword(const char* input, uint16_t* notifications)
{
    if (strcmp(input, password) == 0) {
        mylcd.clear();
        mylcd.setCursor(0, 1);
        mylcd.print("open!");
        open_Door();
        delay(5000);  
        close_Door();
    } 
    else {
        mylcd.clear();
        mylcd.setCursor(0, 0);
        mylcd.print("error!");
        delay(2000);
        mylcd.clear();
        mylcd.setCursor(0, 0);
        mylcd.print("try again");
        close_Door();
        sound_alarm();
        *notifications |= (1 << 8);   // Incorrect password bit
    }

    // Reset LCD prompt for next password entry
    delay(1000);
    mylcd.clear();
    mylcd.setCursor(0, 0);
    mylcd.print("password:");
}


void setup() 
{
    Serial.begin(9600); // Use hardware Serial on pins 0,1
    INITIALISE_LCD_SCREEN();
    INITIALISE_IO_PINS();
    INITIALISE_SERVO_MOTORS();
} 


uint16_t warning_bitfield = 0; 
void loop() 
{
    uint16_t notifications = 0;
    listenForPasswordInput(&notifications);
    SensorData data = readSensors(); 
    while (Serial.available() > 0) 
    {
        char c = Serial.read();  // Read one character at a time
        Serial.print("Received: ");
        Serial.println((int)c);
        notifications = parse_serial_command(c, &data);  
        Serial.println("Notification after parse: " + String(notifications));
    }
    Serial.println("Notification before set warning: " + String(notifications));
    setWarningBitfield(data, notifications);        
    Serial.println("Warning bit field: " + String(data.warnings));
    sendData(data);
    Serial.println("gas level: " + String(data.gas));
    active_sensor_mitigations(data.warnings);
}
