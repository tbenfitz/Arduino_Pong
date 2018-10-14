/************************************
 * Arduino Pong
 * Written by: Thomas B. Fitzgerald
 * GNU Public License
 */


/*______Import Libraries_______*/
// SPFD5408 - Version: Latest 
#include <pin_magic.h>
#include <registers.h>
#include <SPFD5408_Adafruit_GFX.h>
#include <SPFD5408_Adafruit_TFTLCD.h>
#include <SPFD5408_TouchScreen.h>
#include <SPFD5408_Util.h>
/*______End of Libraries_______** */

/*______Define LCD pins (I have asigned the default values)_______*/
#define YP A3 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 9 // can be a digital pin
#define XP 8 // can be a digital pin
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
/*_______End of defanitions______*/

#define WHITE   0x000000
#define YELLOW    0x001F 
#define CYAN     0xF800 
#define PINK   0x07E0 
#define RED    0x07FF 
#define GREEN 0xF81F  
#define BLUE  0x0000FF 
#define BLACK   0x000000

#define PDLWIDTH 30
#define PDLHEIGHT 5
#define BALLDIAM 4
 
#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define MINX 0
#define MINY 0
#define MAXX 235
#define MAXY 315

/*____Calibrate TFT LCD_____*/
#define TS_MINX -187
#define TS_MINY 0
#define TS_MAXX 620
#define TS_MAXY 315
/*______End of Calibration______*/

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); //300 is the sensitivity
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET); //Start communication with LCD

int pd1X, pd1Y, pd2X, pd2Y;
int topV = 0, bottomB = 0;
int bX, bY;
int bXV = 2, bYV = 1;
int last1X;
int last2X;
int topScore = 0, bottomScore = 0;
bool scoreChange = true;
bool firstScore = true;
bool startTop = true;
int loopCount = 0;
int startBallCnt = 0;
int pd1Cnt = 0;

void setup() {
  Serial.begin(9600); //Use serial monitor for debugging
  tft.reset(); //Always reset at start
  tft.begin(0x9341); // My LCD uses LIL9341 Interface driver IC
  tft.setRotation(0); 

  tft.fillScreen(BLUE);
  delay(500);
  tft.fillScreen(BLACK);

  IntroScreen();
  bX = MAXX / 2;
  bY = 30;

  pd1X = (MAXX / 2) - 15; 
  last1X = pd1X;
  pd2X = (MAXX / 2) - 15;
  pd1Y = PDLHEIGHT;
  pd2Y = MAXY - PDLHEIGHT;
  last2X = pd2X;
}

void loop() {  
  bool tmpScoreChange = scoreChange;
  
  if (scoreChange == true) {
    
    tft.fillRect(2, MAXY / 2 - 20, 25, 15, BLACK);
    tft.fillRect(2, MAXY / 2 + 8, 25, 15, BLACK);
    tft.setCursor(2,MAXY / 2 - 20);
    tft.print(topScore);
    tft.setCursor(2, MAXY / 2 + 8);
    tft.print(bottomScore);
    scoreChange = false;

    if (startTop) {
      bX = MAXX / 2;
      bY = 30;
      bXV = 3;
      bYV = 3;
    }
    else {
      bX = MAXX / 2;
      bY = MAXY - 30;
      bXV = -3;
      bYV = -3;
    }

    if (firstScore == false) {
      tft.fillRect(last2X, MAXY, PDLWIDTH, PDLHEIGHT, BLACK); // --- Bottom Paddle
      last2X = pd2X;
      tft.fillRect(pd2X, MAXY, PDLWIDTH, PDLHEIGHT, BLUE); // --- Bottom Paddle          
    }
  }
  
  if (startBallCnt < 600) {    
    HandlePaddles();
    startBallCnt++;
    //Serial.println(startBallCnt);
  }  
  else {
    HandlePaddles();
    HandleBall();    

    Serial.println(tmpScoreChange);
    
    if (tmpScoreChange == true)
      startBallCnt = 0;        
  }
}

void IntroScreen() {
  tft.setCursor(55, 100);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.println("ARDUINO");
  tft.setCursor (80, 140);
  tft.println("PONG");
  tft.setCursor(70, 220);
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.println("TBENFITZ");
  delay(1800);
  tft.fillScreen(BLACK);
}

void HandlePaddles() {  
  TSPoint p;

  // --- Top Paddle  
  if (pd1Cnt == 13) {
    if (bX < 300)
      pd1X = bX - 15;
   
    if (pd1X < 5)
      pd1X = 5;
  
    if (pd1X > MAXX - 15)
      pd1X = MAXX - 30;
  
    pd1Cnt = 0;

    tft.fillRect(last1X, MINY, PDLWIDTH, PDLHEIGHT, BLACK);
    last1X = pd1X;
    tft.fillRect(pd1X, MINY, PDLWIDTH, PDLHEIGHT, BLUE);
  }
  else {
    pd1Cnt++;
  }   

  // --- Middle line
  tft.fillRect(0, (MAXY / 2) - 2, MAXX, BALLDIAM, BLUE); 

  if (loopCount == 3) {
    p = ts.getPoint(); 
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
     
    if(p.z > MINPRESSURE && p.z < MAXPRESSURE) {  
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, MAXX);
      pd2X = p.x - (PDLWIDTH / 2) - 5;
      //Serial.println(p.x);
    } 
    
    loopCount = 0;    
  }

  tft.fillRect(last2X, MAXY, PDLWIDTH, PDLHEIGHT, BLACK); // --- Bottom Paddle
  last2X = pd2X;
  tft.fillRect(pd2X, MAXY, PDLWIDTH, PDLHEIGHT, BLUE); // --- Bottom Paddle
  loopCount++;
}

// -- If return value is 1 collision occured. if return is 0, no collision.
int checkCollision() {
  if ((bX > pd1X && bX < pd1X + PDLWIDTH + 2 && bY < pd1Y) || (bX > pd2X && bX < pd2X + PDLWIDTH + 2 && bY >= pd2Y)) {   
    Serial.print(pd2X);
    return 1;
  }
  else {
    return 0;
  }
}

void HandleBall() {
  tft.fillCircle(bX, bY, 2, BLACK);  
  
  bX += bXV;
  bY += bYV;

  /* Turn the ball around if it hits the edge of the screen. */
  if (bX < 0 || bX > MAXX) {            
    bXV = -bXV;
  }  

  // --- Check for paddle collision
  if (checkCollision() == 1) {
    //Serial.println(bYV);

    bYV = -bYV;      
  }
  else {
    if (bY > MAXY - 4) {
      topScore++;
      scoreChange = true;  
      startTop = true;  
      firstScore = false;
    }
  
    if (bY < 3) {
      bottomScore++;
      scoreChange = true;
      startTop = false;
      firstScore = false;
    }
  
    if (scoreChange == false) {
      tft.fillCircle(bX, bY, 2, BLUE);  
    }
  }
}
