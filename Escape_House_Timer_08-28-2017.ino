#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 20 chars and 4 line display

//Hall effect
const int HallEffectIn = A1;
float HallEffect = 0;


const int ResetButton = 2;

int hours = 0; // start hours
int minutes = 0; //start min
int seconds = 30; //start seconds --> there is a second value below in the reset

int DelayTime = 0;
int Buzzing = 0;

int SecondsInc = 7;
int SecondsDec = 8;
int Buzzer = 11;
int ButtonLight = 12;

int ClearDisplay = 0;

// initialize the RTC library and setup calendar
RTC_DS1307 RTC;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//handles the change in frequency for the loudest possible Piezo

void setPwmFrequency(int pin, int divisor) 
{
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) 
  {
    switch(divisor) 
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) 
    {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } 
    else 
    {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } 
  else if(pin == 3 || pin == 11) 
  {
    switch(divisor) 
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}// end void setPwmFrequency(int pin, int divisor)

void setup()
{
   // initialize the lcd 
  lcd.init();                      
  lcd.backlight();
  lcd.print("! Escape House !");
  lcd.setCursor(0,1);
  lcd.print("Is this a clue?");
  

  //Serial.begin(57600);  //setup the serial port
  Wire.begin();         //setup I2C
  RTC.begin();          //setup RTC
  
  if (! RTC.isrunning())
  {
    lcd.print("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
     RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); //-->with battery installed this should not be necessary
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  } //end if (! RTC.isrunning())
  
  pinMode(ResetButton, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ResetButton), ResetTimer, LOW);

  pinMode(Buzzer,OUTPUT);
  pinMode(SecondsInc,INPUT);
  pinMode(SecondsDec,INPUT);
  DelayTime = 1000;
  delay(DelayTime); //this should display ("!! DTL  Timer !!") longer at start up
}//end void setup()

void loop()
{
  lcd.clear();
//  lcd.print("Count Down Timer ");
  delay(250);
  
  while (hours > 0 || minutes > 0 || seconds >= 0) 
  {
    if (ClearDisplay == 1)
    {
      lcd.clear();
      lcd.noAutoscroll();
//      lcd.setCursor(0,0);
//      lcd.print("Count Down Timer");
      ClearDisplay = 0;
    }

  if (SecondsInc == 1)
     {
        seconds += 10;
     }
  
     if (SecondsDec == 1)
     {
        seconds -= 10;
     }
 
    UpdateTimer();
    //GetTime();
    GetHallEffect();
    delay(DelayTime);
    
    if ( hours >= 0 && minutes >= 0 && seconds >0)
    {
        //other code can go here at the end of the count down
    }
  }//end while (hours > 0 || minutes > 0 || seconds >= 0)
  
}//end void loop()

void UpdateTimer()
{
   if (seconds == 99)
   {
    lcd.clear();
    }
   
   lcd.setCursor(4,1);
   (hours < 10) ? lcd.print("0") : NULL;
   lcd.print(hours);
   lcd.print(":");
   (minutes < 10) ? lcd.print("0") : NULL;
   lcd.print(minutes);
   lcd.print(":");
   (seconds < 10) ? lcd.print("0") : NULL;
   lcd.print(seconds);
   stepDown();

}//end void UpdateTimer()
 
void stepDown() 
{
  if (seconds > 0) 
  {
    seconds -= 1;


  }
  else 
  {
    if (minutes > 0) 
    {
      seconds = 59;
      minutes -= 1;
    } 
    else
    {
      if (hours > 0) 
      {
        seconds = 59;
        minutes = 59;
        hours -= 1;
      } 
      else 
      {
        trigger();
      }
    }//end else !(if (minutes > 0))
  }//end else !(if (seconds > 0))
}//end void stepDown()
 
void ResetTimer()
{
   //lcd.clear();
   setPwmFrequency(11,64);//this should set the timer back to default 
   hours = 0; // start hours
   minutes = 0; //start min
   seconds = 30; //start seconds
   ClearDisplay = 1;
   Buzzing = 0;
   analogWrite(Buzzer, Buzzing);
   
   DelayTime = 1000;
}//end void ResetTimer()

void trigger() 
{
  DelayTime = 250;

  setPwmFrequency(11,8);
  Buzzing = 125;
  analogWrite(Buzzer, Buzzing);
    
  lcd.clear(); // clears the screen and buffer
  //lcd.setCursor(0,10); // set timer position on lcd for end.
  //lcd.println(Buzzing);
  lcd.setCursor(0,0); // set timer position on lcd for end.
  lcd.println("Your Time Is Up! ");
  lcd.setCursor(0,1);
  lcd.print("You need 2 Reset");
  lcd.noBacklight();
  delay(DelayTime);
  setPwmFrequency(11,32);
  lcd.backlight();
  delay(DelayTime);
}//end void trigger()


void GetTime()
{
  DateTime now = RTC.now();         //grab the date and time from RTC
  lcd.setCursor(0, 0);              // set the cursor (column, line)
  //    lcd.print(now.year(), DEC);
  //    lcd.print('/');
  //    lcd.print(now.month(), DEC);
  //    lcd.print('/');
  //    lcd.print(now.day(), DEC);
  //    lcd.print(' ');
 
  if (now.hour() <= 9)              //add a space if hour is less than or equal to 9
  {
    lcd.print(' ');
  }
  lcd.print(now.hour(), DEC);       //send hour to LCD
  //lcd.setCursor(2, 0);              // set the cursor (column, line)
  lcd.print(':');                   //send : to LCD
  //lcd.setCursor(3, 0);              // set the cursor (column, line)
  
  if (now.minute() <= 9)            //add a 0 if minute is less than or equal to 9
  {
    lcd.print('0');
  }
  lcd.print(now.minute(), DEC);     //send minute to LCD
  //lcd.setCursor(5, 0);              // set the cursor (column, line)
  lcd.print(':');                   //send : to LCD
  //lcd.setCursor(6, 0);              // set the cursor (column, line)
  if (now.second() <= 9)            //add a 0 if second is less than or equal to 9
  {
    lcd.print('0');
  }
  lcd.print(now.second(), DEC);     //send second to LCD
}//end void GetTime()

void GetHallEffect()
{
  HallEffect = analogRead(HallEffectIn);
  lcd.setCursor(0,0); 
  lcd.print ("Chess Timer  ");
  lcd.print(HallEffect);
    if ((HallEffect >= 650) && (Buzzing <= 0 ))
    {
      lcd.clear();
      lcd.setCursor(0,1); 
      lcd.print("!Je Bry p.64128!"); 
      delay(750); 
      lcd.clear();
    }
  delay(250);
}//end void GetHallEffect()


