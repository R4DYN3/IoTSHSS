#include <SoftwareSerial.h>

SoftwareSerial softSerial(2, 3);  

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


void receiveData() {
    static bool receiving = false;
    static uint8_t buffer[sizeof(SensorData)];
    static uint8_t index = 0;

    while (softSerial.available()) {
        uint8_t byte = softSerial.read();

        if (!receiving) {
            if (byte == 0xAA) {  
                Serial.println("Start byte detected.");
                receiving = true;
                index = 0;
            }
            continue;
        }

        // Store bytes in buffer
        if (index < sizeof(SensorData)) {
            buffer[index++] = byte;
        } 
        else if (index == sizeof(SensorData)) {
            if (byte == 0x55) {  // End marker
                Serial.println("End marker detected.");
                
                // Copy struct from buffer
                SensorData data;
                memcpy(&data, buffer, sizeof(SensorData));

                // Print values
                Serial.print("Gas: "); Serial.println(data.gas);
                Serial.print("Light: "); Serial.println(data.light);
                Serial.print("Water: "); Serial.println(data.water);
                Serial.print("Soil: "); Serial.println(data.soil);
                Serial.print("PIR Motion: "); Serial.println(data.PIR_motion);
                Serial.print("Warnings: "); Serial.println(data.warnings, BIN);

                if (data.warnings & (1 << 13)) Serial.println("Warning: Gas Danger");
                if (data.warnings & (1 << 12)) Serial.println("Warning: Low Light");
                if (data.warnings & (1 << 11)) Serial.println("Warning: High Water Level");
                if (data.warnings & (1 << 10)) Serial.println("Warning: Dry Soil (Hydropenia)");
                
                Serial.println("----------------------");
            } else {
                Serial.println("Error: End marker missing. Data corrupted.");
            }

            // Reset for next message
            receiving = false;
            index = 0;
            continue;
        } else {
            // Buffer overflow prevention
            Serial.println("Error: Buffer overflow. Resetting.");
            receiving = false;
            index = 0;
            continue;
        }
    }
}

void setup() {
    Serial.begin(9600);    
    softSerial.begin(9600); 
}

void loop() {
    receiveData();
}