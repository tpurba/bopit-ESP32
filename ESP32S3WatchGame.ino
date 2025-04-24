#include <Arduino.h>
#include "LCD_Test.h"
//screen set up
UWORD Imagesize = LCD_1IN28_HEIGHT * LCD_1IN28_WIDTH * 2;// set the screen size 
UWORD *BlackImage;//declares a pointer

//touch screen set up 
CST816S touch(6, 7, 13, 5);

//game setup
String gestures[] = {"SWIPE UP", "SWIPE DOWN", "SWIPE LEFT", "SWIPE RIGHT", "SINGLE CLICK"};
enum GameState {
  STATE_WELCOME,
  STATE_WAIT_FOR_TAP,
  STATE_COUNTDOWN,
  STATE_PLAY,
  STATE_GAME_OVER
};
GameState gameState = STATE_WELCOME;
String currentCommand = "";
String lastGesture = "NONE"; // Tracks the last detected gesture
unsigned long lastGestureTime = 0; // Timestamp for last valid gesture
unsigned long debounceDelay = 1300; // Debounce delay in milliseconds
int score = 0;
unsigned long commandStartTime = 0;
unsigned long commandTimeLimit = 3000; // 3 seconds to respond
bool waitingToStart = true;
unsigned long welcomeStartTime = 0; // Timestamp for welcome message
// bool welcomeDisplayed = false; // Flag for the welcome message
unsigned long resetStartTime = 0; // Timestamp for reset delay
bool resetting = false; // Flag for resetting the game
int countdownValue = 3;
unsigned long countdownStartTime = 0;




void setup() {
  Serial.begin(115200);
  touch.begin();//initializes touch input 
  randomSeed(analogRead(0));
  //some initializations 
  psramInit();
  if ((BlackImage = (UWORD *)ps_malloc(Imagesize)) == NULL){//Memory Allocation for black image 
        Serial.println("Failed to apply for black memory...");
        exit(0);
  }
  DEV_Module_Init();//need for the lcd 1 in 28 thing 
  LCD_1IN28_Init(HORIZONTAL);
  LCD_1IN28_Clear(WHITE);
  Paint_NewImage((UBYTE *)BlackImage, LCD_1IN28.WIDTH, LCD_1IN28.HEIGHT, 0, WHITE); // Create a buffer and fill it with white
  Paint_SetScale(65); 
  Paint_SetRotate(ROTATE_0);
  Paint_Clear(WHITE);
  displayWelcomeMessage();
  welcomeStartTime = millis();
  gameState = STATE_WELCOME;
}

void loop() {
  switch (gameState) {
    case STATE_WELCOME:
      if (millis() - welcomeStartTime >= 3000) {
        // LCD_1IN28_Clear(WHITE);
        gameState = STATE_WAIT_FOR_TAP;
        displayStartMessage();
      }
      break;

    case STATE_WAIT_FOR_TAP:
      if (touch.available() && touch.gesture() == "SINGLE CLICK") {
        countdownValue = 3;
        countdownStartTime = millis();
        displayCountdown(countdownValue);
        gameState = STATE_COUNTDOWN;
      }
      break;

    case STATE_COUNTDOWN:
      if (millis() - countdownStartTime >= 1000) {
        countdownValue--;
        countdownStartTime = millis();
        if (countdownValue > 0) {
          displayCountdown(countdownValue);
        } else {
          gameState = STATE_PLAY;
          currentCommand = "";
          score = 0;
          touch.available();//if the user taps something during count down this will clear it 
        }
      }
      break;

    case STATE_PLAY:
      if (currentCommand == "") {
        startNewCommand();
      }

      if (millis() - commandStartTime > commandTimeLimit) {
        LCD_1IN28_Clear(RED);
        delay(100);
        displayGameOver(score);
        resetStartTime = millis();
        gameState = STATE_GAME_OVER;
      }

      if (touch.available()) {
        String gesture = touch.gesture();
        if (gesture != lastGesture || millis() - lastGestureTime > debounceDelay) {
          lastGesture = gesture;
          lastGestureTime = millis();

          if (gesture == currentCommand) {
            score++;
            // Serial.println("Correct!");
            LCD_1IN28_Clear(GREEN);
            delay(100);
            if(score % 10 == 0){
              Serial.println("lowering the time");
              if(commandTimeLimit > 500){
                commandTimeLimit = commandTimeLimit - 50;
                Serial.println("new time");
                Serial.println(commandTimeLimit);
              }
              
            }
            currentCommand = "";
          } else if (gesture != "NONE") {
            // Serial.println("Wrong gesture!");
            LCD_1IN28_Clear(RED);
            delay(100);
            displayGameOver(score);
            resetStartTime = millis();
            gameState = STATE_GAME_OVER;
          }
        }
      }
      break;
    case STATE_GAME_OVER:
      if (millis() - resetStartTime >= 2000) {
        displayGameOver(score);
        welcomeStartTime = millis();
        gameState = STATE_WELCOME;
        commandTimeLimit = 3000;
      }
      break;
  }
  // if (welcomeDisplayed) {
  //   // Wait 3 seconds before clearing the welcome message
  //   if (millis() - welcomeStartTime >= 3000) {
  //     LCD_1IN28_Clear(WHITE);
  //     welcomeDisplayed = false;
  //   }
  //   return; // Exit loop while showing the welcome message
  // }
  
  // if (resetting) {
  //   // Wait 2 seconds before restarting the game
  //   if (millis() - resetStartTime >= 2000) {
  //     resetting = false;
  //     currentCommand = "";
  //     score = 0;
  //   }
  //   return; // Exit loop while resetting the game
  // }

  // if (currentCommand == "") {
  //   startNewCommand(); // Generate and display a new command
  // }

  // if (millis() - commandStartTime > commandTimeLimit) {
  //   endGame(); // Time's up!
  // }

  // if (touch.available()) {
  //   String gesture = touch.gesture(); // Read the player's gesture
  //   // Add the debounce mechanism
  //   if (gesture != lastGesture || millis() - lastGestureTime > debounceDelay) {
  //     lastGesture = gesture;
  //     lastGestureTime = millis();
  //     // Serial.print("Player gesture: ");
  //     // Serial.println(gesture);

  //     if (gesture == currentCommand) {
  //       score++;
  //       Serial.println("Correct!");
  //       currentCommand = ""; // Prepare for the next command
  //     } else if (gesture != "NONE") { // Only end the game if a wrong gesture is detected (ignore "NONE")
  //       Serial.println("Wrong gesture!");
  //       endGame();
  //     }
  //   }
  // }
}

void startNewCommand() {
  currentCommand = gestures[random(0, 5)]; // Pick a random gesture
  commandStartTime = millis();
  displayCommand(currentCommand); // Show the command on the screen
}

// void endGame() {
//   Serial.print("Game Over! Your score: ");
//   Serial.println(score);
//   displayGameOver(score); // Display "Game Over!" message
//   resetting = true;
//   resetStartTime = millis();
// }

void displayCommand(String command) {
  Paint_Clear(WHITE);
  // Code to display the command on your smartwatch screen
  // Serial.print("New Command: ");
  // Serial.println(command);
  Paint_DrawString_EN(50, 50, "New Command:", &Font16, WHITE, BLACK);
  Paint_DrawString_EN(50, 100, command.c_str(), &Font20, WHITE, BLACK);
  char scoreText[20];
  snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
  Paint_DrawString_EN(50, 150, scoreText, &Font20, WHITE, BLACK);
  LCD_1IN28_Display(BlackImage);
  
}

void displayWelcomeMessage() {
  // Code to display a welcome screen
  Serial.print("Welcome to Bop It! V2.0");
  Paint_DrawString_EN(50, 100, "Welcome", &Font16, WHITE, BLACK);
  Paint_DrawString_EN(50, 125, "to Bop It! V2.0", &Font16, WHITE, BLACK);
  LCD_1IN28_Display(BlackImage);
  // delay(3000);
  // LCD_1IN28_Clear(WHITE);
}
void displayStartMessage(){
  Paint_Clear(WHITE);
  Paint_DrawString_EN(50, 150, "Tap to Start!", &Font16, WHITE, BLACK);
  LCD_1IN28_Display(BlackImage);
}

void displayGameOver(int finalScore) {
  Paint_Clear(WHITE);
  // Code to display "Game Over!" and the final score
  // Serial.println("Game Over! Try again!");
  char gameOverText[100];
  Paint_DrawString_EN(50, 100, "Game Over!", &Font20, WHITE, BLACK);
  char scoreText[20];
  snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
  Paint_DrawString_EN(50, 150, scoreText, &Font20, WHITE, BLACK);
  LCD_1IN28_Display(BlackImage);
}

// void resetGame() {
//   currentCommand = "";
//   score = 0;
//   delay(2000); // Small delay before restarting
// }

void displayCountdown(int value) {
  Paint_Clear(WHITE);
  char buffer[5];
  snprintf(buffer, sizeof(buffer), "%d", value);
  Paint_DrawString_EN(110, 110, buffer, &Font24, WHITE, BLACK);
  LCD_1IN28_Display(BlackImage);
}


