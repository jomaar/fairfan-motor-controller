#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Test build successful!");
}

void loop() {
    delay(1000);
}
