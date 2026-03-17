#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

#define CE_PIN   0   // CE pin
#define CSN_PIN  5   // CSN pin
RF24 radio(CE_PIN, CSN_PIN); // Create an RF24 object
const uint64_t pipeAddress = 0xE7E7E7E7E7; // Address for the pipe (transmitter and receiver should use the same address)
int i;
String command="b";
String command1;
char input='r';

bool role = false; // true = Sender, false = Receiver

void setup() {
  Serial.begin(9600);

  // Initialize the NRF24L01 module
  if (!radio.begin()) {
    Serial.println("NRF24L01 Module Not Detected!");
    while (1);
  }
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
 // updateRole();

}
void loop() {
  next:
    
    if (Serial.available()) {
    char input = Serial.read();
    if (input == 'S' || input == 's') {
      role = true;
      //updateRole();
     } 
     else if (input == 'R' || input == 'r') {
      
      role = false;
     // updateRole();
    }
  }
    if (role) {
    
    // Sender role
    //Serial.println("role sender");

  radio.setRetries(15, 15); // 15 retries and 15ms delay between retries
  radio.openWritingPipe(pipeAddress); // Set the pipe for transmitting
  radio.setPALevel(RF24_PA_HIGH); // Set the power level to high
  radio.setDataRate(RF24_1MBPS); // Set the data rate to 1 Mbps
  radio.stopListening();
  //String command1;

  //Serial.println("enter in serial");
  command = Serial.readStringUntil('\n');
  if((command=="n" || command=="y" || command=="d" || command=="p" || command=="i" || command=="t")){

command1=command;
  }
   else if(command=="r"){
     role = false; 
     command1="n";
    }

    while (!radio.write(&command1, sizeof(command1))) {
    }
    delay(100);
}   
  
   else {
    
    // Receiver role
   // Serial.println("role receiver");
  radio.openReadingPipe(1, pipeAddress); // Set the pipe for receiving
  radio.setPALevel(RF24_PA_HIGH); // Set the power level to high
  radio.setDataRate(RF24_1MBPS); // Set the data rate to 1 Mbps
  radio.startListening(); // Start listening for incoming messages

  if (radio.available()) {
    char receivedLine[64] = "";  // Buffer to hold each received line
    radio.read(&receivedLine, sizeof(receivedLine)); // Read the incoming data
    Serial.println(receivedLine);  // Print received line to serial monitor

    // If the end of the order summary (such as a marker like "Total: Rs") is received, process the full message
    if (String(receivedLine).indexOf("Total: Rs") >= 0) {
      // Serial.println("Full Order Summary Received!");
      Serial.println(" ");
}
  }}}

