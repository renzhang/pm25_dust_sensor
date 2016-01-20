// V0.1
// Directly Read from PM2.5 sensor
// Print to serial port
// Refer to vs voltage when power off

// Declare Pin Used
int measure_Pin   = 0; // Analog  0 to be PM25_SENSOR_ANALOG_V
int ledpower_Pin  = 7; // Digital 7 to be PM25_SENSOR_LED_ON
int fanpower_Pin  = 8; // Digital 8 to be PM25_SENSOR_FAN_ON

// Sampling Period [us]
int sampling_Time = 280;  // delay time from on to sampling start edge: 280us
int delta_Time    = 40;   // extend to whole sampling pulse width 320us
int sleep_Time    = 9680; // sampling period 10ms, 10ms - 320us = 9680us: 9.68ms

int count_VsReading = 600; // Number of sensor readings reference Vs when fan off
int count_VoReading = 100; // Number of sensor readings reference Vo when fan on

float vs_ref_qr = 750; // vs reference value [from QR: 1562128BBB 0.75 **** 25.5][mV]

float vs_ref_sys; // store average vs when fan off, used as vs reference
float vs_rawvalue = 0; // store measured ADC analog output from sensor vout when fan off
float vs_voltage = 0; // calculate actual vs analog voltage level from ADC
float vs_total = 0; 

float vo_rawvalue   = 0; // store measured ADC analog output from sensor vout
float vo_voltage = 0; // calculate actual vs analog voltage level from ADC
float vo_total = 0; 

float DustDensity  = 0; // calculate dust density [ug/m3]


void setup() {
  // Arduino Setup
  Serial.begin(9600);
  pinMode(ledpower_Pin, OUTPUT);
  pinMode(fanpower_Pin, OUTPUT);
  
  // Function to store a vs referennce value when sensor fan is not running.
  // Reference value will be recalculated every time the system reboots.
  // The reference value will be calculated by 
  //   reading the sensor output voltage
  //   and averaged for "count_VsReading" seconds

  digitalWrite(fanpower_Pin,HIGH); // power off sensor fan
  delay(2000); // Wait 2seconds

  Serial.println("Preparing vs Reference voltage");
  Serial.print("Set time for vs ref value: ");
  Serial.print(count_VsReading * 10 / 1000);
  Serial.println(" second");

  
  for (int i=0; i<=count_VsReading; i++) {
	  // disable interrupt to increase timing accuracy
	  noInterrupts();
    digitalWrite(ledpower_Pin,LOW); // power on the LED
    delayMicroseconds(sampling_Time); // wait sampling_Time before reading Vo_Output
    vs_rawvalue = analogRead(measure_Pin); // read the dust value
    delayMicroseconds(delta_Time); //Wait delta_Time before shuttign  of LED
    digitalWrite(ledpower_Pin,HIGH); // turn the LED off
	  // enable interrupt
	  interrupts();
    vs_voltage = vs_rawvalue * 5.0 / 1024; // Calculate actual voltage [V]
    vs_total += vs_voltage; //Adding up all read voltages
    }

  // Calculate vs reference value
  vs_ref_sys = vs_total / count_VsReading * 1000; // Calculate average output voltage when fan is not running. REFERENCE VALUE [mV]
  Serial.print(" vs estimated [mV]: ");  // Print vs Total to see progress
  Serial.println(vs_ref_sys);
 
  digitalWrite(fanpower_Pin,LOW); // power on sensor fan
  delay(5000); // Wait 5 seconds before starting void loop to measure
}

void loop() {
  for (int i=0; i<=count_VoReading; i++) {
    // disable interrupt to increase timing accuracy
    noInterrupts();
    digitalWrite(ledpower_Pin, LOW); // Turn on sensor LED
    delayMicroseconds(sampling_Time); // Sampling Delay after LED on edge 280us
    vo_rawvalue = analogRead(measure_Pin); // Sampling Analog V from sensor pin2
    delayMicroseconds(delta_Time); // 40us delay for LED off
    digitalWrite(ledpower_Pin, HIGH); // Turn off sensor LED
    delayMicroseconds(sleep_Time);
    // enable interrupt
    interrupts();
    vo_voltage = vo_rawvalue * 5.0 / 1024; // Calculate actual voltage [V]
    vo_total += vo_voltage;
    }
  vo_voltage = vo_total / count_VoReading * 1000; // Calculate average output voltage [mV]
  // Calculate dust voltage according to sensor datasheet formula, BETA=1
  DustDensity  = 0.6 * (vo_voltage - vs_ref_qr); 
 
  Serial.print("vs [mV]: ");
  Serial.print(vs_ref_qr);
  
  Serial.print("; vo [mV]: ");
  Serial.print(vo_voltage);

  Serial.print("; Dust density [ug/m3]:");
  Serial.println(DustDensity);
 
  delay(5000); // Wait 5 seconds between each reading/printing
}
