
//imports for oled screen
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//imports for rtc
#include <virtuabotixRTC.h>
#include <TimeLib.h>
//imports for otp calculation
#include <Crypto.h>
#include <CryptoLegacy.h>
#include <SHA1.h>
#include <Hash.h>
#include <math.h>
#include <string.h>

//rtc defs
#define clk 2
#define dat 3
#define rst 5
virtuabotixRTC myRTC(clk, dat, rst);

//oled defs
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

//totp defs
SHA1 sha1;

//totp class with issuer, account name, secret and image
class Totp
{
public:
  String issuer;
  String accountName;
  u8 secret[10];
  String image;
  u8 period;
  u8 digits;
  String getOtp(u32 time)
  {
    return String(generateOtp(time, this->digits, this->period, this->secret));
  }
  Totp(String issuer, String account, u8 secret[10], String bitmap)
  {
    this->issuer = issuer;
    this->accountName = account;
    this->secret = secret;
    this->image = bitmap;
    this->period = 30;
    this->digits = 6;
  }
};

void setup()
{
  Serial.begin(9600);
  oledSetup();
}

char c;
void loop()
{
  //check if there is serial input
  // read unix time from serial
  if (Serial.available() > 0)
  {
    c = Serial.read();

    // 0 enters time config mode
    if (c == '0')
    {
      startUserTimeConfig();
    }
  }

  u32 unixTime = getUnixTime();
  u8 secret[10] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0xde, 0xad, 0xbe, 0xef};
  // instantiate totp object
  Totp totp(String("Google"), String("leonardo@gmail.com"), secret, String("Virtuabotix"));
  // calculate totp
  String totpCode = totp.getOtp(unixTime);

  drawScreen(totp.issuer, totpCode);
  delay(100);
}

void startUserTimeConfig()
{
  Serial.println("Enter unix time followed by #");
  Serial.println("Press e/E to exit");
  String input = "";
  while (true)
  {
    if (Serial.available() > 0)
    {
      c = Serial.read();
      if (c == 'e' || c == 'E')
      {
        Serial.println("Exiting time config mode");
        break;
      }
      else
      {
        if (c != '#')
        {
          input += c;
        }
        else
        {
          setUnixTime(input.toInt());
          input = "";
          Serial.println("Exiting time config mode");
          break;
        }
      }
    }
  }
}

void setUnixTime(u32 unixTime)
{
  Serial.println("Setting time");
  Serial.println(unixTime);
  Serial.println("Time set");
  setTime(unixTime);
  myRTC.setDS1302Time(second(), minute(), hour(), weekday(), day(), month(), year());

  Serial.println(String(hour()) + ":" + String(minute()) + ":" + String(second()));
}

void oledSetup()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();
}

void drawScreen(String label, String token)
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(label);

  display.setCursor(0, 16);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.print(token);

  display.display();
}

String generateOtp(u32 time, u8 digits, u32 period, u8 hmacKey[10])
{
  uint8_t hash[20];

  //counter
  long time_f = time;
  long period_f = period;
  long time_f_period = time_f / period_f;
  u32 counter = time_f_period;

  //array is size 8 with 4 bytes of padding
  byte counterBytes[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  //big endian
  for (int i = 0; i < 4; i++)
  {
    counterBytes[7 - i] = (counter >> (i * 8)) & 0xFF;
  }

  //get hmac
  sha1.resetHMAC(hmacKey, 10);
  sha1.update(counterBytes, 8);
  sha1.finalizeHMAC(hmacKey,10, hash, 20);


  // get the offset
  u8 offset = hash[19] & 0xF;

  // get 4 bytes of data
  u32 truncatedHash = 0;
  for (int i = 0; i < 4; i++)
  {
    truncatedHash <<= 8;
    truncatedHash |= hash[offset + i];
  }

  // get the otp
  truncatedHash &= 0x7FFFFFFF;
  truncatedHash %= 1000000;
  String otpString = String(truncatedHash);
  //add leading zeros
  while (otpString.length() < digits)
  {
    otpString = "0" + otpString;
  }
  return otpString;
}

u32 getUnixTime()
{
  myRTC.updateTime();
  setTime(myRTC.hours, myRTC.minutes, myRTC.seconds, myRTC.dayofmonth, myRTC.month, myRTC.year);
  return now();
}