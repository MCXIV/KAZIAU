#include <Arduino.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
#define BUILTIN 13
#define BP_VEILLE 10
#define BP_USER 11

Adafruit_SSD1306 display(OLED_RESET);

unsigned long counter_sec = millis();
unsigned long counter_screen = 0;
int second = 0;
int minute = 0;
int hour = 0;
char flag_bp = 0;
char flag_screen = 0;
char flag_set_h = 0;
char flag_set_m = 0;
char flag_set_s = 0;
char flag_div;
int one_sec = 1000;
int three_sec = 3000;
unsigned char serial_data[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

void wakeLcd()
{
   clock_prescale_set(clock_div_1);
   flag_div = 0;
   digitalWrite(9, HIGH);
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 128x32)
   display.clearDisplay();
}

void sleepLcd()
{
   clock_prescale_set(clock_div_8);
   flag_div = 1;
   digitalWrite(9, LOW);
}

void setText(char setting)
{
   display.clearDisplay();
   if (setting == 1)
   {
      display.drawRect(12, 6, 30, 22, WHITE);
   }
   if (setting == 2)
   {
      display.drawRect(48, 6, 30, 22, WHITE);
   }
   if (setting == 3)
   {
      display.drawRect(84, 6, 30, 22, WHITE);
   }
   display.drawRect(1, 1, 126, 31, WHITE);
   display.setTextSize(2);
   display.setTextColor(WHITE);
   display.setCursor(16, 10);
   if (hour < 10)
   {
      display.print("0");
   }
   display.print(hour);
   display.print(':');
   if (minute < 10)
   {
      display.print("0");
   }
   display.print(minute);
   display.print(':');
   if (second < 10)
   {
      display.print("0");
   }
   display.println(second);
   display.display();
}

void setup()
{
   Serial.begin(9600);
   pinMode(9, OUTPUT);               // ON/OFF LCD
   digitalWrite(9, LOW);             // ON LCD
   pinMode(BP_VEILLE, INPUT_PULLUP); // BP SORTIE VEILLE
   pinMode(BP_USER, INPUT_PULLUP);   // BP USER
   // pinMode(BUILTIN, OUTPUT);                    // LED BUILTIN
   power_adc_disable();
   power_spi_disable();
   power_timer1_disable();
   power_timer2_disable();
   power_timer3_disable();
   power_usb_disable();
   power_usart0_disable();
   ADCSRA = 0;
   // CLKSEL0 |= (1 << RCE); // CLKSEL0.RCE = 1; Enable_RC_clock()
   // while ((CLKSTA & (1 << RCON)) == 0)
   // {
   // }                        // while (CLKSTA.RCON != 1);  while (!RC_clock_ready())
   // CLKSEL0 &= ~(1 << CLKS); // CLKSEL0.CLKS = 0; Select_RC_clock()
   // CLKSEL0 &= ~(1 << EXTE); // CLKSEL0.EXTE = 0; Disable_external_clock
   clock_prescale_set(clock_div_8);
   flag_div = 1;
}

void loop()
{
   if (flag_div == 1)
   {
      one_sec = 125;
      three_sec = 375;
   }
   else
   {
      one_sec = 1000;
      three_sec = 3000;
   }
   ///// BLOC TEMPS /////
   if (millis() - counter_sec >= one_sec)
   {
      counter_sec = millis();
      second++;
   }
   if (second == 60)
   {
      second = 0;
      minute++;
   }
   if (minute == 60)
   {
      minute = 0;
      hour++;
   }
   if (hour == 24)
   {
      hour = 0;
   }
   ///// FIN BLOC TEMPS /////

   ///// BLOC ECRAN /////
   if (!digitalRead(BP_VEILLE))
   {
      if (flag_bp == 0)
      {
         flag_screen = 1;
         counter_screen = millis();
         wakeLcd();
      }
      flag_bp = 1;
   }
   else
   {
      flag_bp = 0;
   }
   if (flag_screen == 1)
   {
      setText(0);
      if (millis() - counter_screen >= three_sec)
      {
         flag_screen = 0;
         sleepLcd();
      }
   }
   ///// FIN BLOC ECRAN /////

   ///// BLOC MISE A L HEURE BP /////
   if (!digitalRead(BP_VEILLE) && !digitalRead(BP_USER))
   {
      wakeLcd();
      flag_set_h = 1;
      flag_set_m = 1;
      flag_set_s = 1;
      for (int i = 0; i <= 10; i++)
      {
         digitalWrite(BUILTIN, !digitalRead(BUILTIN));
         delay(150);
      }
      setText(1);
      while (flag_set_h == 1)
      {
         if (!digitalRead(BP_USER))
         {
            hour++;
            if (hour == 24)
            {
               hour = 0;
            }
            setText(1);
         }
         else if (!digitalRead(BP_VEILLE))
         {
            delay(250); // anti rebond software
            flag_set_h = 0;
         }
      }
      setText(2);
      while (flag_set_m == 1)
      {
         if (!digitalRead(BP_USER))
         {
            minute++;
            if (minute == 60)
            {
               minute = 0;
            }
            setText(2);
         }
         else if (!digitalRead(BP_VEILLE))
         {
            delay(250); // anti rebond software
            flag_set_m = 0;
         }
      }
      setText(3);
      while (flag_set_s == 1)
      {
         if (!digitalRead(BP_USER))
         {
            second++;
            if (second == 60)
            {
               second = 0;
            }
            setText(3);
         }
         else if (!digitalRead(BP_VEILLE))
         {
            counter_sec = millis();
            flag_set_s = 0;
         }
      }
      digitalWrite(LED_BUILTIN, LOW);
   }
   ///// FIN BLOC MISE A L HEURE BP /////
}

// TODO
// - Calculer facteur de dérèglement pour correction de la bdt