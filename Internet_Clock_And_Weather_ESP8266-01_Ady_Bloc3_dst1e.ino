#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> // http web access library
#include <ArduinoJson.h> // JSON decoding library
#include <WiFiUdp.h>
#include <NTPClient.h>               // include NTPClient library
#include <TimeLib.h>                 // include Arduino time library
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

//#include <Fonts/FreeSans9pt7b.h>
//#include <Fonts/FreeSerif9pt7b.h>

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

//#define PIN 2   //esp8266-01
#define PIN D3  //Mcu esp8266 v2-v3
#define DSTpin 12 // D6 -> https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

unsigned long timezoneOffset = 7200;

// set Wi-Fi SSID and password
const char *ssid = "HD86SAG";             //ADY
const char *password = "0000000000";    //88888888
// set location and API key https://openweathermap.org/city
String Location = "Simeria, RO";
String API_Key = "ffdf0053ad2b89f752fbfb664fcd136d";
WiFiUDP ntpUDP;
//The NTPClient library is configured to get time information (Unix epoch) from the server time.nist.gov (GMT time) and an offset of 1 hour ( ==> GMT + 1 time zone) which is equal to 3600 seconds, configuration line is below:
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
//กำหนดค่าที่จะได้รับข้อมูลเวลา (Unix epoch) จากเซิร์ฟเวอร์  time.nist.gov (GMT time) และชดเชย 1 ชั่วโมง 
//(==> ในเขตเวลา GMT โซนเวลา + 1) ซึ่งเท่ากับ 3600 วินาที เช่น TH+7 เท่ากับ 3200*7=25200
//NTPClient timeClient(ntpUDP);
NTPClient timeClient(ntpUDP, "time.nist.gov", timezoneOffset);

// MATRIX DECLARATION:
// Parameter 1 = width of EACH NEOPIXEL MATRIX (not total display)
// Parameter 2 = height of each matrix
// Parameter 3 = number of matrices arranged horizontally
// Parameter 4 = number of matrices arranged vertically
// Parameter 5 = pin number (most are valid)
// Parameter 6 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are
//     arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN
//     EACH MATRIX proceed in the same order, or alternate lines reverse
//     direction; pick one.
//   NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT:
//     Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick
//     two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.
//   NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY
//     are arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES
//     (tiles) in the OVERALL DISPLAY proceed in the same order for every
//     line, or alternate lines reverse direction; pick one.  When using
//     zig-zag order, the orientation of the matrices in alternate rows
//     will be rotated 180 degrees (this is normal -- simplifies wiring).
//   See example below for these values in action.
// Parameter 7 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 pixels)
//   NEO_GRB     Pixels are wired for GRB bitstream (v2 pixels)
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA v1 pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)

// Example with three 10x8 matrices (created using NeoPixel flex strip --
// these grids are not a ready-made product).  In this application we'd
// like to arrange the three matrices side-by-side in a wide display.
// The first matrix (tile) will be at the left, and the first pixel within
// that matrix is at the top left.  The matrices use zig-zag line ordering.
// There's only one row here, so it doesn't matter if we declare it in row
// or column order.  The matrices use 800 KHz (v2) pixels that expect GRB
// color data.

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(24,8, PIN,
  //NEO_TILE_TOP   + NEO_TILE_RIGHT   + NEO_TILE_ROWS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);
//https://htmlcolorcodes.com/
const uint16_t colors[] = {
  matrix.Color(255, 255, 255), //0 alb
  matrix.Color( 0, 255, 0), //1 verde
  matrix.Color(153, 51, 255),//2 indigo
  matrix.Color(255, 51, 51),   //3 rosu 2
  matrix.Color(0 , 0, 255),//4 albastru
  matrix.Color(153, 255, 153), //5 lime
  matrix.Color(255, 255, 0), //6 galben
  matrix.Color(255, 0, 0), //7 rosu
  matrix.Color( 0, 255, 255), //8 naval
  matrix.Color(255, 153, 51) //9 portocaliu
  };

char Time[] = "  :  :  ";
char Date[] = "  /  /20  ";
char weather[200],temp_[10],humi_[10],Wind_speed_[10],pressure_[10];
byte last_second, second_, minute_, hour_, wday, day_, month_, year_,mu=0,xload=0,cnt=0;
String wday_;
int xi = matrix.width(),x=matrix.width(),xu=0,xd=0; 
unsigned long time_now=0;

int DST0 = 7;
int DST = 0;
int decalaj =0;

void setup() {
  pinMode(DSTpin, INPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  timeClient.begin();
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(80);
  Serial.print("Connecting.");
  while ( WiFi.status() != WL_CONNECTED ) {
    matrix.clear();
    matrix.setCursor(0, 0);
    matrix.setTextColor(colors[1]);
    matrix.print("Int..");
    matrix.setCursor(1, 9);
    matrix.setTextColor(colors[6]);
    matrix.print("Con.. ");
 //   matrix.setCursor(32, 9);
    matrix.setTextColor(colors[4]);
    matrix.print(String(mu));
    matrix.show();
    delay(500);
    if(++mu > 8){
       matrix.clear();
       matrix.setCursor(1, 9);
       matrix.setTextColor(colors[4]);
       matrix.print("Conn");
       matrix.setCursor(1, 9);
       matrix.setTextColor(colors[7]);
       matrix.print("Failed..");
       matrix.show();
       delay(2000);
       mu=0;
      }
  }
  matrix.clear();
   matrix.setCursor(0, 0);
  matrix.setTextColor(colors[5]);
  matrix.print(" Ok");
  Serial.println("Con");
  matrix.show();
  delay(2000);
  matrix.setCursor(1, 9);
  matrix.setTextColor(colors[1]);
  matrix.print("Int");
  mu=0;
  weather_loop();

 if (digitalRead(DSTpin) == LOW)
   DST = 0;
  else
   DST = 1;
  timeClient.setTimeOffset(timezoneOffset + DST*3600);
  timeClient.begin();
  DST0 = DST;
  timeClient.update();
    Serial.print("DST = ");
    Serial.println(DST); 
}

void loop() {
 if (digitalRead(DSTpin) == LOW)
   DST = 0;
  else
   DST = 1;
if (DST0 != DST)
{
  timeClient.setTimeOffset(timezoneOffset + DST*3600);
  timeClient.begin();
  DST0 = DST;
  timeClient.update();
    Serial.print("DST = ");
    Serial.println(DST); 
}
  
  time_loop();

  if(mu<7) Step_A();
  //if(mu>6) Step_B();
  
  delay(90);
}  // end main loop

void Step_A(){
  matrix.fillScreen(0);
  matrix.setRotation(0);
  matrix.setFont(NULL);
  //matrix.setFont(&FreeSerif9pt7b);
  matrix.clear();
  matrix.setTextSize(1);


  if(mu>=0<=2){   
  matrix.setCursor(0-xu, 0);
  matrix.setTextColor(colors[0]);
  matrix.print("     Str.Piata Unirii  ");
  matrix.setTextColor(colors[0]);
  matrix.print(" Bloc 3   Ap.1-20 ");
  
  
  //matrix.print(String(Time));//       + - -xu
  matrix.setCursor(0, 0-xu);
  matrix.setTextColor(colors[7]);     
  PrintData0(hour_);
  matrix.setCursor(9, 0-xu);
  matrix.setTextColor(colors[0]);
  if (second_%2 == 0)
      matrix.print(F(":"));
 else
      matrix.print(F(" "));
  //matrix.print(":");
  matrix.setCursor(10, 0-xu);
  matrix.setTextColor(colors[0]);
  if (second_%2 == 0)
      matrix.print(F(":"));
 else
      matrix.print(F(" "));
  //matrix.print(":");
  matrix.setCursor(13, 0-xu);
  matrix.setTextColor(colors[7]);
  PrintData0(minute_);
  
  //matrix.setTextColor(colors[4]);
  //matrix.print(":");
  //matrix.setTextColor(colors[0]);
  //PrintData0(second_);
 /* 
  matrix.setCursor(1, 0-xu);
  matrix.setTextColor(colors[1]);
  matrix.print(temp_);
  // matrix.setCursor(13, 9-xu);
  matrix.setTextColor(colors[7]);
  matrix.print("C ");
  if(int(humi_) >= 100)
   matrix.setCursor(21, 9-xu);
  matrix.setTextColor(colors[8]);
  matrix.print("H");
  //matrix.setCursor(22, 9-xu);
  matrix.setTextColor(colors[1]);
  matrix.print(humi_);
 // matrix.setCursor(40, 9-xu);
  matrix.setTextColor(colors[7]);
  matrix.print("%");
  }
  if(mu>=2<=3){
    matrix.setCursor(0, 18-xu);
  matrix.setTextColor(colors[0]);
  matrix.print(wday_);
  matrix.setCursor(7, 27-xu);
  matrix.setTextColor(colors[4]);
  matrix.print(String("Day "));
  matrix.setCursor(30, 27-xu);
  matrix.setTextColor(colors[6]);
  PrintData0(day_);
 // matrix.print(String(day_));
  }
  if(mu>=3<=4){
  matrix.setCursor(0, 37-xu);
  matrix.setTextColor(colors[0]);
  matrix.print(String("Month "));
  matrix.setCursor(33, 37-xu);
  matrix.setTextColor(colors[1]);
  PrintData0(month_);
 // matrix.print(String(month_));
  matrix.setCursor(0, 46-xu);
  matrix.setTextColor(colors[4]);
  matrix.print(String("Year"));
    matrix.setCursor(25, 46-xu);
  matrix.setTextColor(colors[7]);
  matrix.print(String("20")+String(year_));
   }
   if(mu>=4<=5){
  matrix.setCursor(0, 54-xu);
  matrix.setTextColor(colors[7]);
  matrix.print(String("Wind"));
    matrix.setCursor(25, 54-xu);
    matrix.setTextColor(colors[0]);
  matrix.print(String("speed"));
  matrix.setCursor(6, 63-xu);
  matrix.setTextColor(colors[4]);
  matrix.print(String(Wind_speed_));
    matrix.setCursor(24, 63-xu);
  matrix.setTextColor(colors[1]);
  matrix.print(String("m/s"));
   }
   if(mu>=5<=6){
  matrix.setCursor(1, 72-xu);
  matrix.setTextColor(colors[6]);
  matrix.print(String("Pressure"));
  matrix.setCursor(0, 81-xu);
  matrix.setTextColor(colors[4]);
  matrix.print(String(pressure_));
   matrix.setCursor(26, 81-xu);
  matrix.setTextColor(colors[1]);
  matrix.print(String("hpa"));  --  ++  + 5000
*/

  }
  
  matrix.show();
  if(xd > 1&& mu > 1){xd;xu++;}
  if(mu == 6 && xd == 5) mu=7;                            //4600
  if(millis() > time_now + 4900) {
    time_now = millis();
    xd=8; mu++;
    Serial.println(mu);
	
	
	matrix.setTextColor(colors[8]);
  matrix.print(wday_);
  matrix.setTextColor(colors[8]);
  PrintData0(day_);
  matrix.setTextColor(colors[6]);
  matrix.print("/");
  matrix.setTextColor(colors[8]);
  PrintData0(month_);
  matrix.setTextColor(colors[6]);
  matrix.print("/");
  matrix.setTextColor(colors[8]);
  matrix.print(String("20")+String(year_));

  
 
 
  
  matrix.setCursor(xi, 0);
  //Serial.println(xi);
  matrix.setTextColor(colors[4]);
   matrix.print("Temp.:");
   matrix.setTextColor(colors[1]);
   matrix.print(temp_);
   matrix.setTextColor(colors[7]);
   matrix.print(" *C ");
   matrix.setTextColor(colors[4]);
   matrix.print(" Umid.:");
   matrix.setTextColor(colors[1]);
   matrix.print(humi_);
   matrix.setTextColor(colors[7]);
   matrix.print(" %RH ");
   matrix.setTextColor(colors[4]);
   matrix.print("  Pres.:");
   matrix.setTextColor(colors[1]);
   matrix.print(pressure_);
   matrix.setTextColor(colors[7]);
   matrix.print(" mmHg ");
   matrix.setTextColor(colors[4]);
   matrix.print("  Vant:");
   matrix.setTextColor(colors[1]);
   matrix.print(Wind_speed_);
   matrix.setTextColor(colors[7]);
   matrix.print(" Km/h ");
  
//  matrix.print(String(weather));
//  matrix.setTextColor(matrix.Color(R, Wheel(R+B), 255));
  if(x > -131) x--;                  //131
  if(x < -130 && xi > -401) xi--;     //130   401
  if(xi < -345 ){                     //308
    mu=0;xu=0;xi=matrix.width();x=xi;
    if(++xload >= 2 ){weather_loop();xload=0;}
    if(++cnt>9){
       //Serial.println("Reset..");
       ESP.restart();
  }

  }

/*
void Step_B(){
  matrix.clear();
  matrix.setTextSize(1);
  matrix.setCursor(x, 0);

  
  
  matrix.setTextColor(colors[8]);
  matrix.print(wday_);
  matrix.setTextColor(colors[8]);
  PrintData0(day_);
  matrix.setTextColor(colors[6]);
  matrix.print("/");
  matrix.setTextColor(colors[8]);
  PrintData0(month_);
  matrix.setTextColor(colors[6]);
  matrix.print("/");
  matrix.setTextColor(colors[8]);
  matrix.print(String("20")+String(year_));

  
 
 
  
  matrix.setCursor(xi, 0);
  //Serial.println(xi);
  matrix.setTextColor(colors[4]);
   matrix.print("Temp.:");
   matrix.setTextColor(colors[1]);
   matrix.print(temp_);
   matrix.setTextColor(colors[7]);
   matrix.print(" *C ");
   matrix.setTextColor(colors[4]);
   matrix.print(" Umid.:");
   matrix.setTextColor(colors[1]);
   matrix.print(humi_);
   matrix.setTextColor(colors[7]);
   matrix.print(" %RH ");
   matrix.setTextColor(colors[4]);
   matrix.print("  Pres.:");
   matrix.setTextColor(colors[1]);
   matrix.print(pressure_);
   matrix.setTextColor(colors[7]);
   matrix.print(" mmHg ");
   matrix.setTextColor(colors[4]);
   matrix.print("  Vant:");
   matrix.setTextColor(colors[1]);
   matrix.print(Wind_speed_);
   matrix.setTextColor(colors[7]);
   matrix.print(" Km/h ");
  
//  matrix.print(String(weather));
//  matrix.setTextColor(matrix.Color(R, Wheel(R+B), 255));
  if(x > -131) x--;                  //131
  if(x < -130 && xi > -401) xi--;     //130   401
  if(xi < -345 ){                     //308
    mu=0;xu=0;xi=matrix.width();x=xi;
    if(++xload >= 2 ){weather_loop();xload=0;}
    if(++cnt>9){
       //Serial.println("Reset..");
       ESP.restart();
      }
    }
   */ 

  matrix.show();
  }


void time_loop()
{
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
 
  second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
  if (last_second != second_)
  {
    minute_ = minute(unix_epoch);      // get minutes (0 - 59)
    hour_   = hour(unix_epoch);        // get hours   (0 - 23)
    wday    = weekday(unix_epoch);     // get minutes (1 - 7 with Sunday is day 1)
    day_    = day(unix_epoch);         // get month day (1 - 31, depends on month)
    month_  = month(unix_epoch);       // get month (1 - 12 with Jan is month 1)
    year_   = year(unix_epoch) - 2000; // get year with 4 digits - 2000 results 2 digits year (ex: 2018 --> 18)
 
    Time[7] = second_ % 10 + '0';
    Time[6] = second_ / 10 + '0';
    Time[4] = minute_ % 10 + '0';
    Time[3] = minute_ / 10 + '0';
    Time[1] = hour_   % 10 + '0';
    Time[0] = hour_   / 10 + '0';
    Date[9] = year_   % 10 + '0';
    Date[8] = year_   / 10 + '0';
    Date[4] = month_  % 10 + '0';
    Date[3] = month_  / 10 + '0';
    Date[1] = day_    % 10 + '0';
    Date[0] = day_    / 10 + '0';
 
    display_wday();
    //draw_text(4,  29, Date, 2);        // display date (format: dd-mm-yyyy)
    //draw_text(16, 50, Time, 2);        // display time (format: hh:mm:ss)
    //display.display();
 
    last_second = second_;
 
  }
 
//  delay(200);
 
}
 
void display_wday()
{
  switch(wday){
    case 1:  draw_text(40, 15, " Duminica   ", 1); break;
    case 2:  draw_text(40, 15, " Luni   ", 1); break;
    case 3:  draw_text(40, 15, " Marti   ", 1); break;
    case 4:  draw_text(40, 15, " Miercuri   ", 1); break;
    case 5:  draw_text(40, 15, " Joi   ", 1); break;
    case 6:  draw_text(40, 15, " Vineri   ", 1); break;
    default: draw_text(40, 15, " Sambata   ", 1);
  }
}
 
void draw_text(byte x_pos, byte y_pos, char *text, byte text_size)
{
 // display.setCursor(x_pos, y_pos);
 // display.setTextSize(text_size);
 // display.print(text);
    wday_ = text;
}



void weather_loop()
{
if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
{
HTTPClient http; //Declare an object of class HTTPClient

// specify request destination
http.begin("http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key); // !!

int httpCode = http.GET(); // send the request

if (httpCode > 0) // check the returning code
{
String payload = http.getString(); //Get the request response payload

DynamicJsonBuffer jsonBuffer(512);

// Parse JSON object
JsonObject& root = jsonBuffer.parseObject(payload);
if (!root.success()) {
Serial.println(F("Parsing failed!"));
return;
}

float temp = (float)(root["main"]["temp"]) - 273.15; // get temperature in °C
int humidity = root["main"]["humidity"]; // get humidity in %
int pressure = (float)(root["main"]["pressure"]) *0.75006; // 1000; // get pressure in mmHg
float wind_speed = (float)(root["wind"]["speed"]) *3.6; // get wind speed in m/s)*3.6 Km/H
//int wind_degree = root["wind"]["deg"]; // get wind degree in °
sprintf(weather, "Temperature:%.0fC Humidity:%d%% Pressure:%u hpa Wind speed:%.1fm/s",temp,humidity,pressure,wind_speed);
sprintf(temp_, "%.0f",temp);
sprintf(humi_, "%d",humidity);
sprintf(Wind_speed_, "%.1f",wind_speed);
sprintf(pressure_, "%u",pressure);


// print data
//Serial.printf("Temperature = %.1f°C\r\n", temp);
//Serial.printf("Humidity = %d %%\r\n", humidity);
//Serial.printf("Pressure = %u bar\r\n", pressure);
//Serial.printf("Wind speed = %.1f m/s\r\n", wind_speed);
//Serial.printf("Wind degree = %d°\r\n\r\n", wind_degree);
}

http.end(); //Close connection

}

//delay(60000); // wait 1 minute

}

void PrintData0(int digits_)
{
  // utility for digital clock display: prints preceding colon and leading 0
  if (digits_ < 10)
    matrix.print('0');
    matrix.print(digits_);
}
