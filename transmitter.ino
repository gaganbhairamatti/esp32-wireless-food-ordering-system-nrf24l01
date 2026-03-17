#include <SPI.h>
#include <RF24.h>
#include <TFT_eSPI.h>            // Graphics library for TFT display
#include <XPT2046_Touchscreen.h>  // Library for touchscreen functionality

// Pin definitions for touchscreen and buzzer
#define CS_PIN  21      // Chip Select for touchscreen
#define TIRQ_PIN 255    // No interrupt pin used
#define BUZZER_PIN 22   // Pin for buzzer

TFT_eSPI tft = TFT_eSPI();                // Create TFT object
XPT2046_Touchscreen touch(CS_PIN, TIRQ_PIN); // Create touchscreen object

// Screen dimensions
const int screenWidth = 320;
const int screenHeight = 480;

String receivedText;

int i=1;

// Menu items and prices
const char* items[] = {"DOSA", "IDLI", "PURI", "TEA "};
const int prices[] = {60, 45, 60, 20};
int selectedItems[4] = {0, 0, 0, 0}; // Tracks selected items
int role=1;
String receivedLine;

// NRF24L01 configuration
#define CE_PIN   0   // CE pin
#define CSN_PIN  5   // CSN pin
RF24 radio(CE_PIN, CSN_PIN); // Create an RF24 object
const uint64_t pipeAddress = 0xE7E7E7E7E7; // Address for the pipe (transmitter and receiver should use the same address)

bool isSummaryScreen = false;
bool isOrderPlaced = false;

unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 300;

// Setup function
void setup() {
  Serial.begin(9600);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  touch.begin();

  pinMode(BUZZER_PIN, OUTPUT);  // Set buzzer pin as output

  drawMenu(); // Draw initial menu

  if (!radio.begin()) {
    Serial.println("NRF24L01 Module Not Detected!");
    while (1);
  }


}

void drawMenu() {



  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(120, 10);
  tft.print("MENU LIST");

  for (int i = 0; i < 4; i++) {
    tft.fillRect(10, i * 40 + 40, 180, 25, TFT_BLUE);
    tft.drawRect(10, i * 40 + 40, 180, 25, TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, i * 40 + 45);
    tft.print(items[i]);
    tft.print("-Rs-");
    tft.print(prices[i]);

    tft.setCursor(140, i * 40 + 45);
    tft.print(" x ");
    tft.print(selectedItems[i]);

    tft.fillRect(200, i * 40 + 40, 40, 25, TFT_GREEN);
    tft.drawRect(200, i * 40 + 40, 40, 25, TFT_WHITE);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(215, i * 40 + 45);
    tft.print("+");

    tft.fillRect(250, i * 40 + 40, 40, 25, TFT_RED);
    tft.drawRect(250, i * 40 + 40, 40, 25, TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(265, i * 40 + 45);
    tft.print("-");
  }

  tft.fillRect(10, 200, 300, 25, TFT_GREEN);
  tft.drawRect(10, 200, 300, 25, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(130, 205);
  tft.print("NEXT");
}

void showSummary() {
  tft.fillScreen(TFT_BLACK);

  int total = 0;
  int y = 20;
  for (int i = 0; i < 4; i++) {
    if (selectedItems[i] > 0) {
      tft.setCursor(20, y);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.print(items[i]);
      tft.print(" x ");
      tft.print(selectedItems[i]);
      tft.print("= Rs ");
      tft.print(prices[i] * selectedItems[i]);
      total += prices[i] * selectedItems[i];
      y += 30;
    }
  }

  tft.setCursor(20, y + 20);
  tft.print("TOTAL:Rs ");
  tft.print(total);

  tft.fillRect(10, 190, 140, 25, TFT_RED);
  tft.drawRect(10, 190, 140, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(50, 195);
  tft.print("BACK");

  tft.fillRect(160, 190, 140, 25, TFT_BLUE);
  tft.drawRect(160, 190, 140, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(165, 195);
  tft.print("ORDER PLACE");

  isSummaryScreen = true;
}

void showOrderPlacedScreen1() {
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 100);
  tft.print("ORDER PLACED");

  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.print("ORDER RECEIVED");

    // Main Menu button
  tft.fillRect(10, 190, 130, 25, TFT_BLUE);
  tft.drawRect(10, 190, 130, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 195);
  tft.setTextSize(2);
  tft.print("MAIN MENU");
  delay(500);
}
void showOrderPlacedScreen2() {

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 100);
  tft.print("ORDER NOT PLACED");

  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.print("PURI NOT AVAILABLE");

    // Main Menu button
  tft.fillRect(10, 190, 130, 25, TFT_BLUE);
  tft.drawRect(10, 190, 130, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 195);
  tft.setTextSize(2);
  tft.print("MAIN MENU");
  delay(500);
}
void showOrderPlacedScreen3() {

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 100);
  tft.print("ORDER NOT PLACED");

  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.print("DOSA NOT AVAILABLE");

    // Main Menu button
  tft.fillRect(10, 190, 130, 25, TFT_BLUE);
  tft.drawRect(10, 190, 130, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 195);
  tft.setTextSize(2);
  tft.print("MAIN MENU");
  delay(500);
}
void showOrderPlacedScreen4() {

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 100);
  tft.print("ORDER NOT PLACED");

  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.print("IDLI NOT AVAILABLE");

    // Main Menu button
  tft.fillRect(10, 190, 130, 25, TFT_BLUE);
  tft.drawRect(10, 190, 130, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 195);
  tft.setTextSize(2);
  tft.print("MAIN MENU");
  delay(500);
}
void showOrderPlacedScreen5() {

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 100);
  tft.print("ORDER NOT PLACED");

  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.print("TEA NOT AVAILABLE");

    // Main Menu button
  tft.fillRect(10, 190, 130, 25, TFT_BLUE);
  tft.drawRect(10, 190, 130, 25, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 195);
  tft.setTextSize(2);
  tft.print("MAIN MENU");
  delay(500);
}


void sendOrderSummary() {
  radio.setRetries(15, 15); // 15 retries and 15ms delay between retries
  radio.openWritingPipe(pipeAddress); // Set the pipe for transmitting
  radio.setPALevel(RF24_PA_HIGH); // Set the power level to high
  radio.setDataRate(RF24_1MBPS); // Set the data rate to 1 Mbps
  radio.stopListening();
  int total = 0;
  String orderSummary = "Table No-1 Order Details:\n";
  
  // Construct the order summary string
  for (int i = 0; i < 4; i++) {
    if (selectedItems[i] > 0) {
      orderSummary += String(items[i]) + " x " + String(selectedItems[i]) + " - RS " + String(prices[i] * selectedItems[i]) + "\n";
      total += prices[i] * selectedItems[i];
    }
  }
  orderSummary += "Total: Rs " + String(total);

  // Send the order line by line with a delay
  for (unsigned int i = 0; i < orderSummary.length(); i++) {
    String line = "";
    while (i < orderSummary.length() && orderSummary[i] != '\n') {
      line += orderSummary[i];
      i++;
    }

    // Wait for the radio to be ready before sending the next line
    while (!radio.write(line.c_str(), line.length())) {
      // Wait until the transmission is complete
    }

    // Print sent data for debugging
    Serial.println(line);

    // Small delay to prevent overwhelming the receiver
    delay(200);  // Adjust this delay if needed
  }
}


void beep(bool isLongBeep = false) {
  if (isLongBeep) {
    tone(BUZZER_PIN, 100, 1000);  // Long beep at 1kHz for 1000ms
  } else {
    tone(BUZZER_PIN, 50, 100);   // Short beep at 1kHz for 200ms
  }
}


void loop() {

  
  radio.openReadingPipe(1, pipeAddress); // Set the pipe for receiving
  radio.setPALevel(RF24_PA_HIGH); // Set the power level to high
  radio.setDataRate(RF24_1MBPS); // Set the data rate to 1 Mbps
  radio.startListening(); // Start listening for incoming messages

  if (radio.available()) {
    char receivedLine[64] = "";  // Buffer to hold each received line
    radio.read(&receivedLine, sizeof(receivedLine)); // Read the incoming data
    Serial.println(receivedLine);  // Print received line to serial monitor
    if (strcmp(receivedLine, "y") == 0) { 

        showOrderPlacedScreen1();
    }
      else if (strcmp(receivedLine, "p") == 0) { 

        showOrderPlacedScreen2();
    }
      else if (strcmp(receivedLine, "d") == 0) { 

        showOrderPlacedScreen3();
    }
      else if (strcmp(receivedLine, "i") == 0) { 

        showOrderPlacedScreen4();
    }
      else if (strcmp(receivedLine, "t") == 0) { 

        showOrderPlacedScreen5();
    }
  }

  if (touch.touched()) {
    unsigned long currentTime = millis();
    if (currentTime - lastTouchTime > debounceDelay) {
      lastTouchTime = currentTime;

      TS_Point point = touch.getPoint();
      int x = map(point.x, 120, 4095, screenWidth, 0);
      int y = map(point.y, 200, 7595, 0, screenHeight);

      if (isSummaryScreen) {
        if (x > 10 && x < 150 && y > 190 && y < 215) {
          beep(); // Beep on "Back" button
          drawMenu();
          isSummaryScreen = false;
        }

        if (x > 160 && x < 300 && y > 190 && y < 215) {
          beep(true); // Beep on "Order Place" button
          sendOrderSummary();
        //  if(receivedLine=="p"){
         //  Serial.println("need open"); 
          //showOrderPlacedScreen1();
         // }
          //showOrderPlacedScreen1();
          memset(selectedItems, 0, sizeof(selectedItems)); // Clear order after placing
        }
      } else {
        for (int i = 0; i < 4; i++) {
          if (y > i * 40 + 40 && y < i * 40 + 65) {
            if (x > 200 && x < 240) {
              selectedItems[i]++;
              beep();  // Beep on "+" button for selecting an item
            } else if (x > 250 && x < 290 && selectedItems[i] > 0) {
              selectedItems[i]--;
              beep();  // Beep on "-" button for deselecting an item
            }
            drawMenu();
          }
        }

        if (x > 10 && x < 310 && y > 190 && y < 215) {
          beep(); // Beep on "Next" button
          showSummary();
        }
      }

      // Main Menu Button in the Order Placed screen
      if (isOrderPlaced && x > 10 && x < 310 && y > 190 && y < 215) {
        beep(); // Beep on Main Menu button
        drawMenu();  // Go back to the main menu
        isSummaryScreen = false;
      }
    }
  }
}
