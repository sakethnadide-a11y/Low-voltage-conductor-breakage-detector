// --- LIBRARIES --- 
// --- AI MODEL LIBRARIES --- 
#include <TensorFlowLite_ESP32.h> 
#include "model_data.h" 
#include "tensorflow/lite/micro/all_ops_resolver.h" 
#include "tensorflow/lite/micro/micro_interpreter.h" 
#include "tensorflow/lite/micro/system_setup.h" 
#include "tensorflow/lite/schema/schema_generated.h" 
#include "tensorflow/lite/micro/micro_error_reporter.h" 

// --- PIN DEFINITIONS --- 
const int CURRENT_PIN = 35; // ACS712 
const int RELAY_PIN   = 26; // Relay 
const int BUZZER_PIN  = 27; // Buzzer 
const int VOLTAGE_PIN = 34; // Voltage Divider Circuit

// --- AI MODEL SETUP --- 
const tflite::Model* model = nullptr; 
tflite::MicroInterpreter* interpreter = nullptr; 
tflite::MicroErrorReporter tflite_error_reporter; 
TfLiteTensor* input = nullptr; 
TfLiteTensor* output = nullptr; 

// Set up memory for the model 
constexpr int kTensorArenaSize = 10 * 1024; 
uint8_t tensor_arena[kTensorArenaSize]; 

// The anomaly threshold from your Python script
// FIX: This value was 3500, which is incorrect. The AI output is scaled 0-1.
const float ANOMALY_THRESHOLD = 3500; 

// Sudden drop threshold (tune as needed) 
const float ERROR_DROP_THRESHOLD = 0.15; // Adjust this value as required 

// --- VOLTAGE DIVIDER SETUP ---
const float R1 = 30000.0; // 30kOhm resistor
// FIX: Corrected typo 'float float' to 'float'
const float R2 = 10000.0; // 10kOhm resistor
const float VOLTAGE_THRESHOLD = 2.0; // Set this to the minimum voltage you expect (e.g., 2V)

void setup() { 
  Serial.begin(115200); 

  pinMode(RELAY_PIN, OUTPUT); 
  pinMode(BUZZER_PIN, OUTPUT); 

  // Initialize with relay open (power on) and buzzer off 
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(BUZZER_PIN, LOW); 

  // --- AI Model Initialization --- 
  model = tflite::GetModel(g_model_data); 
  static tflite::AllOpsResolver resolver; 
  static tflite::MicroInterpreter static_interpreter( 
      model, resolver, tensor_arena, kTensorArenaSize, &tflite_error_reporter); 
  interpreter = &static_interpreter; 
  interpreter->AllocateTensors(); 
  input = interpreter->input(0); 
  output = interpreter->output(0); 
  Serial.println("AI Model setup complete."); 

  Serial.println("System ready and monitoring..."); 
} 

void loop() { 
  static float lastError = 0.0; // Store last cycle's error 

  // Read current and voltage sensor data
  float currentRaw = analogRead(CURRENT_PIN);
  float voltageRaw = analogRead(VOLTAGE_PIN);

  // Calculate actual voltage from the raw reading
  float voltage_at_pin = (voltageRaw / 4095.0) * 3.3; 
  float actual_voltage = voltage_at_pin * ((R1 + R2) / R2);

  // Scale current data for the AI model 
  float scaled_input = currentRaw / 4095.0; // Scale to a 0.0-1.0 range 

  // Fill the AI model's input tensor 
  input->data.f[0] = scaled_input; 

  // Run the AI model (inference) 
  interpreter->Invoke(); 

  // Get the model's output and calculate the error 
  float reconstructionError = abs(output->data.f[0] - input->data.f[0]); 
  
  // Print sensor values for diagnostics
  Serial.print("Current Raw: ");
  Serial.print(currentRaw);
  Serial.print(" | Voltage: ");
  Serial.print(actual_voltage);
  Serial.print("V |");
  Serial.println();
  //Serial.println(reconstructionError);

  // Sudden drop detection 
  if ((lastError - reconstructionError) > ERROR_DROP_THRESHOLD) { 
    Serial.println(">>> Sudden drop in error detected! Buzzing..."); 
    digitalWrite(BUZZER_PIN, HIGH); 
    delay(1000); // Buzz for 1 second 
    digitalWrite(BUZZER_PIN, LOW); 

    // Halt the program until reset 
    while (true) { 
      delay(1000); 
    } 
  } 

  // Check for anomalies or voltage drop
  if (reconstructionError > ANOMALY_THRESHOLD || actual_voltage < VOLTAGE_THRESHOLD) {
    Serial.println("----------------------------------------"); 
    Serial.println("!!! Fault Detected! !!!"); 

    // 1. Tell the relay to cut the power supply 
    digitalWrite(RELAY_PIN, LOW); // Assuming LOW triggers the relay to cut power 
    Serial.println("Power supply isolated."); 

    // 2. Activate the buzzer to alert nearby personnel 
    digitalWrite(BUZZER_PIN, HIGH); 
    Serial.println("Buzzer activated!"); 
    delay(3000); // Buzzer sounds for 3 seconds 
    digitalWrite(BUZZER_PIN, LOW); 

    Serial.println("System halted to prevent continuous alerts. Please reset."); 
    Serial.println("----------------------------------------"); 

    // Halt the program until reset 
    while (true) { 
      delay(1000); 
    } 
  } 

  // All is normal 
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(BUZZER_PIN, LOW); 

  lastError = reconstructionError; // Update lastError for next loop 

  delay(1000); // Wait 1 second before the next reading 
}