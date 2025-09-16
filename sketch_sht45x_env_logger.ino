// SPDX-FileCopyrightText: 2025 ManniX-ITA
//
// SPDX-License-Identifier: MIT
//
// Environment Logger v1s

#include <Adafruit_SHT4x.h>
#include <FlashStorage.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

typedef struct {
  boolean isset;
  char name[101];
} Sensor_data;

FlashStorage(sensor_tag, Sensor_data);

String mysensor_name = "SHT4x";
String mysensor_serial;

Sensor_data mysensor;

const int numReadings = 360; // 3 minutes for 500ms pooling time
const int pooling_delay = 500; // 500ms pooling time
int readIndex = 0;
int curIteration = 0;
float dew_readings[numReadings];
float dew_max;
float dew_avg;
float dew_total = 0;

void setup() {
  sensors_event_t humidity, temp;
  
  Serial.begin(115200);

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    dew_readings[thisReading] = 0;
  }

  mysensor = sensor_tag.read();

  if (mysensor.isset == true) {
    mysensor_name = mysensor.name;
  }

  while (!Serial) {
    delay(10);     // will pause until serial console opens
  }
  
  Serial.println("# Adafruit SHT4x Trinkey Environment Logger v1");
  if (! sht4.begin()) {
    Serial.println("# Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("# Found SHT4x sensor.");
  Serial.print("# Serial number 0x");
  mysensor_serial = sht4.readSerial(), HEX;
  Serial.println(mysensor_serial);

  Serial.print("# Dew point rolling buffer in minutes: ");
  Serial.println(numReadings / 60 / 2);

  sht4.setPrecision(SHT4X_HIGH_PRECISION);  
  sht4.setHeater(SHT4X_HIGH_HEATER_1S); // Heating to clean the sensor
  sht4.getEvent(&humidity, &temp);
  sht4.setHeater(SHT4X_NO_HEATER);
  delay(2500); // Get back to an almost normal temperature after heating
  
  Serial.println("# Temperature *C, Relative Humidity %, Dew Point *C, Dew Avg *C, Dew Max *C, VPD kPa, Serial, Name");
}

void loop() {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);

  curIteration++;

  // Calculate Dew point
  double gamma = log(humidity.relative_humidity / 100) + (17.62 * temp.temperature) / (243.12 + temp.temperature);
  double dew_point = (243.12 * gamma) / (17.62 - gamma);
  
  // Calculate Dew point average
  // subtract the last reading:
  dew_total = dew_total - dew_readings[readIndex];
  // store the Dew point temperature:
  dew_readings[readIndex] = dew_point;
  // add the reading to the total:
  dew_total = dew_total + dew_readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average in the rolling buffer:
  int divider = numReadings;
  if (curIteration < numReadings) divider = readIndex;
  dew_avg = dew_total / divider;

  // Find the Dew point Max in the rolling buffer
  dew_max = dew_readings[0];
  for (int i = 1; i < numReadings; i++)
  {
    if (dew_readings[i] > dew_max)
      dew_max = dew_readings[i];        
  }

  Serial.print(temp.temperature);
  Serial.print(", ");
  Serial.print(humidity.relative_humidity);

  Serial.print(", ");
  Serial.print(dew_point);
  Serial.print(", ");
  Serial.print(dew_avg);
  Serial.print(", ");
  Serial.print(dew_max);
  
  // saturation vapor pressure is calculated
  float svp = 0.6108 * exp(17.27 * temp.temperature / (temp.temperature + 237.3));
  // actual vapor pressure
  float avp = humidity.relative_humidity / 100 * svp;
  // VPD = saturation vapor pressure - actual vapor pressure
  float vpd = svp - avp;
  
  Serial.print(", ");
  Serial.print(vpd);
  Serial.print(", ");
  Serial.print(mysensor_serial);
  Serial.print(", ");
  Serial.println(mysensor_name);

  char inChar = Serial.read();
  if (inChar == 'n' or inChar == 'N')  // Set a new name
  {
    String newname = Serial.readStringUntil('\n');
    Serial.setTimeout(1000);
    if (!newname.length())
      Serial.println("# Name empty, not set");
    else if (newname.length() > 100)
      Serial.println("# Name too long, maximum 100 characters");
    else
      {
      mysensor = sensor_tag.read();
      newname.toCharArray(mysensor.name, 100);
      mysensor.isset = true;
      mysensor_name = newname;      
      sensor_tag.write(mysensor);
      Serial.print("# New name for sensor set: ");
      Serial.println(mysensor_name);
      }
  }
  if (inChar == 'r' or inChar == 'R')  // Reboot
  {
    Serial.println("# Rebooting");
    __disable_irq();
    NVIC_SystemReset();
    while (true);
  }

  // Pooling delay between readings
  delay(pooling_delay);  
}