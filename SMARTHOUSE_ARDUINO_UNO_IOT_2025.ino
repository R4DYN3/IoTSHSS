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


struct SensorData {
    uint32_t gas;
    uint32_t light;
    uint32_t water;
    uint32_t soil;
    uint32_t infrar;
};

// Function to read all sensor values and store them in a struct
SensorData readSensors() {
    SensorData data;
    data.gas = analogRead(A0);
    data.light = analogRead(A1);
    data.water = analogRead(A3);
    data.soil = analogRead(A2);
    data.infrar = digitalRead(2);
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
// Gas Light Water Soil Rain [THE REST ARE RESERVERED / FUTURE USE]
// (1 = Warning, 0 = No Warning)



// Function to process the sensor data and perform actions
void setWarningBitfield(SensorData& data, uint16_t& warnings) {
    // Reset all warning bits to clear previous state
    warnings = 0;

    // Gas sensor processing (set Gas Warning bit if gas exceeds threshold)
    if (data.gas > DANGER_GAS) {
        warnings |= (1 << 13);  // Set the Gas Warning bit (bit 13)
    }

    // Light sensor processing (set Light Warning bit if light is below threshold)
    if (data.light < LOW_LIGHT) {
        warnings |= (1 << 12);  // Set the Light Warning bit (bit 12)
    }

    // Water sensor processing (set Water Warning bit if water exceeds threshold)
    if (data.water > RAIN_WATER) {
        warnings |= (1 << 11);  // Set the Water Warning bit (bit 11)
    }

    // Soil sensor processing (set Soil Warning bit if soil moisture is below threshold)
    if (data.soil > SOIL_HYDROPENIA) {
        warnings |= (1 << 10);  // Set the Soil Warning bit (bit 10)
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
 // pinMode(2, INPUT);//set digital 2 to input
  //pinMode(3, OUTPUT);//set digital 3 to output
  pinMode(A0, INPUT);//set A0 to input
  pinMode(A1, INPUT);//set A1 to input
  pinMode(13, OUTPUT);//set digital 13 to input
  pinMode(A3, INPUT);//set A3 to input
  pinMode(A2, INPUT);//set A2 to input

  pinMode(12, OUTPUT);//set digital 12 to output
  pinMode(5, OUTPUT);//set digital 5 to output
}


/// ------------------------ songs and buzzer related stuff ------------------------ ///
enum NoteFrequency {
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
// enum class NoteDuration : float {
//     WHOLE = 1.0,
//     HALF = 0.5,
//     QUARTER = 0.25,
//     EIGHTH = 0.125,     // Fixed: EIGHTH should be 0.125, not 0.25...
//     SIXTEENTH = 0.0625  // Fixed: SIXTEENTH should be 0.0625, not 0.625...
// };


NoteFrequency tune[] = {
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
float durt[] =
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
int length = sizeof(tune) / sizeof(tune[0]); 

uint16_t warning_bitfield = 0;
void setup() {
    Serial.begin(9600); // Use hardware Serial on pins 0,1
    INITIALISE_LCD_SCREEN();
    INITIALISE_IO_PINS();

    // want to test all the sensors and make sure they work: (todo): perhaps bind to a key? make it's own func

} 

void loop() {
    SensorData data = readSensors();            
    setWarningBitfield(data, warning_bitfield);        
    sendData(data, warning_bitfield);
    active_sensor_mitigations(warning_bitfield);
}