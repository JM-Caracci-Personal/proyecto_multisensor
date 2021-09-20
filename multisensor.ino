// Include Libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DHT_U.h>
#include <EEPROM.h>

// Define constants
#define time_out_test 10000             // time out for each test
#define time_interval_test 2000         // time between tests
#define time_warmup_gas_sensor 2000    //  time to warm up gas sensor Default 20000
#define time_screen_off 30000           // time to turn off screen
#define time_Mx_to_M0 5000              // time to return to M0 screen if nothing is pressed
#define min_t_btw_read 3000             // time between reading sensors
#define time_alarm_disabled 10000       // time alarms keep desabled no matter what
#define time_alarm_off 10000            // PEND time out for alarm Default 300000
#define alarm_period 500                // light and sound alarm duration Default 500
#define t_click_sensitivity 250          // in milliseconds
#define lcd_refresh_rate 200            // in milliseconds
#define T_increment 1
#define G_increment 5

//Define Pins and EEPROM addresses and other constants
#define pin_M 4
#define pin_PLUS 7
#define pin_MINUS 6
#define pin_LED 5
#define pin_DHT 3
#define pin_BUZZER 2
#define pin_GAS_SENSOR A3
#define address_threshold_temp 0    // increments of 1 [0-50] mapped from 0 C to 50 C with increments of "T_increment"
#define address_threshold_gas 1     // increments of 1 [0-200] mapped from 0 to 1000 with increments of "G_increment"
#define DHTTYPE DHT11

// Create  objects
LiquidCrystal_I2C lcd(0x27,16,2);   // Create LCD in the i2c address (my LCD ys 16x2)
DHT dht(pin_DHT, DHTTYPE);	        // Create dht


// Global variables
float temp, gas_level;
int t_threshold=EEPROM.read(address_threshold_temp)*T_increment;
int gas_threshold=EEPROM.read(address_threshold_gas)*G_increment;
int display_mode=0;   // 0:display temp&gas - 1: Change Temp threshold - 2: Change Gas Threshold - 3: Toggle screen on/off
bool test_on_setup,screen_on,alarm_on,disabling_alarm=0;
bool buzzer_sound_on, led_light_on=0;
long t_last_click,t_last_read,t_alarm, t_led_sound_control, t_alarm_was_disabled, t_last_lcd_refresh;

void setup() 
{
  Serial.begin(9600);
  Serial.println("Initialize Serial");

  pinMode(pin_M, INPUT_PULLUP);
  pinMode(pin_PLUS, INPUT_PULLUP);
  pinMode(pin_MINUS, INPUT_PULLUP);
  pinMode(pin_LED, OUTPUT);
  pinMode(pin_BUZZER, OUTPUT);
  
  if (!digitalRead(pin_M)) test_on_setup=true;  
  
  Serial.print("test_on_setup = ");
  Serial.println(test_on_setup);
  Serial.println("Thresholds:");
  Serial.print("Temp. Celsius = ");
  Serial.println(t_threshold);
  Serial.print("Gas = ");
  Serial.println(gas_threshold);
    
  lcd.init();                     // INITIALIZE LCD
  dht.begin();	                  // INITIALIZE DHT
  setup_gas_sensor();

  // Setup Thresholds
  // EEPROM.update(address_threshold_temp, 40/T_increment);	    // First Setup Ever for T threshold, its measured in "increments", not the real value
  // EEPROM.update(address_threshold_gas, 300/G_increment);	    // First Setup Ever for Gas threshold, its measured in "increments", not the real value
  t_threshold=EEPROM.read(address_threshold_temp)*T_increment;  
  gas_threshold=EEPROM.read(address_threshold_gas)*G_increment;

  if(test_on_setup) test_devices();
  
  screen_on=1;                      // Start device with on for time_screen_off seconds
  display_mode=0;
  t_last_click=millis();
  t_last_lcd_refresh=millis();
}

void loop() 
{
  read_thresholds_and_sensors();
  check_sensor_trigger_alarm();
  read_buttons_trigger_action();
  display_info();
}

void setup_gas_sensor()
{
  lcd.backlight();  //  TURN ON LCD BACKLIGHT
  lcd.print("Warming up");
  lcd.setCursor(0, 1);
  lcd.print("Gas Sensor");
  delay(time_warmup_gas_sensor);  // WARM UP GAS SENSOR
  lcd.clear();
  lcd.noBacklight();
}

void test_devices()
{
  Serial.println("Begin Testing...");   
  lcd.backlight();  //  TURN ON LCD BACKLIGHT
  lcd.clear();
  lcd.print("TESTING...");
  delay(time_interval_test); 

  test_lcd();             // TESTING LCD
  test_button(pin_M);     // TESTING BUTTONS
  test_button(pin_PLUS);
  test_button(pin_MINUS);
  test_led();             // TESTING LED
  test_buzzer();          // TESTING BUZZER
  test_dht();             // TESTING DHT
  test_gas_sensor();      // TESTING GAS SENSOR
  test_eeprom();         // TESTING EEPROM
  
  // END OF TESTS
  lcd.clear();
  lcd.print("END OF TESTS");
  delay(time_interval_test);
  lcd.clear();
  lcd.noBacklight();
  Serial.println("End of Test");   
}

void test_lcd()
{
  lcd.clear();
  lcd.print("Testing LCD...");
  delay(time_interval_test); 
  lcd.clear();
  lcd.print("HELLO WORLD");  
  delay(time_interval_test);
  lcd.setCursor(0, 1);
  lcd.print("LCD OK");  
  delay(time_interval_test);
}

void test_button(int pin)
{
  bool test_result=0;  // To store result of the test
  String buttonName = "Hello String";
  switch (pin)
    {
      case pin_M:
        buttonName="M";
        break;
      case pin_PLUS:
        buttonName="+";
        break;
      case pin_MINUS:
        buttonName="-";
        break;
      default:
        buttonName="ERROR";
        break;
    }
  lcd.clear();
  lcd.print("Press ");
  lcd.print(buttonName);
  lcd.print("...");
  unsigned long time_test_started=millis();
  while (test_result==0 && (millis()-time_test_started < time_out_test)) // Wait for button or time_out
  {
    if (!digitalRead(pin))
      test_result=1;    
  } 
  lcd.setCursor(0, 1);
  if (test_result==0)             // Test not OK or time out
    lcd.print("Button NOT OK");  
  else                            // Test OK
    lcd.print("Button OK");  
  delay(time_interval_test);   
}

void test_led()
{
  lcd.clear();
  lcd.print("Testing LED...");
  delay(time_interval_test); 
  lcd.clear();
  lcd.print("LED ON");  
  digitalWrite(pin_LED, HIGH);
  delay(time_interval_test);
  lcd.setCursor(0, 1);
  lcd.print("LED OFF");  
  digitalWrite(pin_LED, LOW);
  delay(time_interval_test);
}

void test_buzzer()
{
  lcd.clear();
  lcd.print("Testing Buzzer..");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Buzzer ON");
  tone(pin_BUZZER,440,time_interval_test*2);  // 2xTimes to test the "noTone" function
  delay(time_interval_test); 
  lcd.setCursor(0, 1);
  lcd.print("Buzzer OFF");
  noTone(pin_BUZZER);
  delay(time_interval_test);  
}

void test_dht()
{
  lcd.clear();
  lcd.print("Testing DHT...");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Reading Temp");
  delay(time_interval_test); 
  temp=dht.readTemperature();
  lcd.setCursor(0, 1);
  if (isnan(temp)) 	                // DHT Working OK?
    lcd.print("DHT NOT OK");   
  else
  {
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print(" C");
  }
  delay(time_interval_test);
}

void test_gas_sensor()
{
  lcd.clear();
  lcd.print("Testing Gas S...");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Reading Gas Lvl");
  delay(time_interval_test); 
  gas_level=analogRead(pin_GAS_SENSOR);
  lcd.setCursor(0, 1);
  lcd.print("Gas Lvl: ");
  lcd.print(gas_level);
  delay(time_interval_test);
}

void test_eeprom()
{
  lcd.clear();
  lcd.print("Testing EEPROM");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Reading EEPROM");
  t_threshold=EEPROM.read(address_threshold_temp)*T_increment;  
  gas_threshold=EEPROM.read(address_threshold_gas)*G_increment;
  delay(time_interval_test); 
  lcd.clear();
  lcd.print("T Thresh: ");
  lcd.print(t_threshold);
  lcd.setCursor(0, 1);
  lcd.print("G Thresh: ");
  lcd.print(gas_threshold);
  delay(time_interval_test);
}

void read_thresholds_and_sensors()
{
  t_threshold=EEPROM.read(address_threshold_temp)*T_increment;  // PEND Refactor, no es necesario leerlo siempre
  gas_threshold=EEPROM.read(address_threshold_gas)*G_increment; // PEND Refactor, no es necesario leerlo siempre
  if (millis()-t_last_read>min_t_btw_read)
  {
    Serial.println("---------------------------------------");
    temp=dht.readTemperature();
    if (isnan(temp)) 	                // DHT Working OK?
      Serial.println("DHT NOT OK");   
    gas_level=analogRead(pin_GAS_SENSOR);
    t_last_read=millis();
    
    Serial.print("Display Mode = ");
    Serial.println(display_mode);
    Serial.print("Temp real - threshold = ");
    Serial.print(temp);
    Serial.print(" - ");
    Serial.println(t_threshold);
    Serial.print("Gas real - threshold = ");
    Serial.print(gas_level);
    Serial.print(" - ");
    Serial.println(gas_threshold);
    Serial.print("Alarm is = ");
    Serial.println(alarm_on);
    Serial.print("Screen is = ");
    Serial.println(screen_on);
    Serial.println("---------------------------------------");
  }
}

void check_sensor_trigger_alarm()
{
  if((temp>t_threshold || gas_level>gas_threshold) && millis()-t_alarm_was_disabled>time_alarm_disabled)
  {
    Serial.println("Trigger Alarm");
    t_alarm=millis();
    alarm_on=true;
  }
  if(alarm_on)
  {
    if(millis()-t_alarm<time_alarm_off)     // IF alarm is on and there is no Time out
    { 
      if (millis()-t_led_sound_control>alarm_period)   // IF its time to toggle the alarm sound/light
      {
        if(!(buzzer_sound_on || led_light_on))  // IF Buzzer or light is off
        {
          tone(pin_BUZZER,440,alarm_period);
          digitalWrite(pin_LED, HIGH);
          buzzer_sound_on=1;
          led_light_on=1;
        }
        else
        {
          noTone(pin_BUZZER);
          digitalWrite(pin_LED, LOW);
          buzzer_sound_on=0;
          led_light_on=0;
        }
        t_led_sound_control=millis();
      }
    }
    else // Deactivating alarm because of timeout
    {
      Serial.println("Time Out Alarm");
      disable_alarm();
    }
  }
  return;
}

void  read_buttons_trigger_action()
{
  if (millis()-t_last_click>t_click_sensitivity)
  {
    if (!digitalRead(pin_M) || !digitalRead(pin_PLUS) || !digitalRead(pin_MINUS))
    {
      t_last_click=millis();    
      if (alarm_on)
        disable_alarm();
      else
      {
        if (!screen_on)
        {
          screen_on=1;
          display_mode=0;
        }
        else
        {
          // if M, M++, if M resultante=4 entonces M=0
          if(!digitalRead(pin_M))
            {
              Serial.print("Cambiar de display mode "); 
              Serial.print(display_mode); 
              Serial.print(" al "); 
              display_mode++;
              display_mode=display_mode%4;
              Serial.println(display_mode);
            }
            
          switch(display_mode)           
            {
              // Case M=0 do nothing
              case 0:
              break;
              
              // Case M=1, up or down temp threshold
              case 1:
                if(EEPROM.read(address_threshold_temp)!=0 && EEPROM.read(address_threshold_temp)!=999)
                {              
                  if(!digitalRead(pin_PLUS))
                  {
                    Serial.println("Subir threshold temp en 1 incremento"); 
                    EEPROM.update(address_threshold_temp, EEPROM.read(address_threshold_temp)+1);                
                  }
                  if(!digitalRead(pin_MINUS))
                  {
                    Serial.println("Bajar threshold temp en 1 incremento");   
                    EEPROM.update(address_threshold_temp, EEPROM.read(address_threshold_temp)-1);               
                  }
                }
              break;
              
              // Case M=2, up or down gas threshold
              case 2:
                if(EEPROM.read(address_threshold_gas)!=0 && EEPROM.read(address_threshold_gas)!=999)
                {              
                  if(!digitalRead(pin_PLUS))
                  {
                    Serial.println("Subir threshold gas en 1 incremento"); 
                    EEPROM.update(address_threshold_gas, EEPROM.read(address_threshold_gas)+1);                
                  }
                  if(!digitalRead(pin_MINUS))
                  {
                    Serial.println("Bajar threshold gas en 1 incremento");   
                    EEPROM.update(address_threshold_gas, EEPROM.read(address_threshold_gas)-1);               
                  }
                }             
              break;
              
              // Case M=3, toggle on/off screen
              case 3:
                if(!digitalRead(pin_PLUS) || !digitalRead(pin_MINUS))
                {
                  Serial.println("Toggle screen"); 
                  screen_on=!screen_on;
                  display_mode=0;                                  
                }
              break;
            }
        }
      }
    }
  }
  else 
    Serial.println("Too soon to read buttons"); 
  
  return;
}


void display_info()
{
  if (alarm_on==1)
  {
    // Display ALARM MSG
    lcd.backlight();  //  TURN ON LCD BACKLIGHT
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALARM! ALARM!");
    lcd.setCursor(0, 1);
    if (gas_level>gas_threshold)
    {
      lcd.print("DANGER GAS");
    }
    else if(temp>t_threshold)
    {
      lcd.print("DANGER Temp");      
    }
  }
  else
  {
    if (millis()-t_last_click > time_screen_off || screen_on==0)
    {
      screen_on=0;
      display_mode=0;
      lcd.clear();
      lcd.noBacklight(); 
    }
    else if (millis()-t_last_click > time_Mx_to_M0)
    {
      screen_on=1;
      display_mode=0;
    }    
    if (screen_on==1 && millis()-t_last_lcd_refresh > lcd_refresh_rate)
    {
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(0, 0);
      /* PEND Flickering screen, idealmente no hacer backlight y clear cada vez... */
      switch (display_mode)
      {
        case 0:
          lcd.print("T=");
          lcd.print((int)round(temp));
          lcd.print("     Lmt=");
          lcd.print(t_threshold);
          lcd.setCursor(0, 1);
          lcd.print("Gas=");
          lcd.print((int)round(gas_level));
          lcd.print("  Lmt=");
          lcd.print(gas_threshold);          
          break;
        case 1:
          lcd.print("Temp Limit");
          lcd.setCursor(0, 1);
          lcd.print("   -   ");
          lcd.print(t_threshold);
          lcd.print("   +   ");
          break;
        case 2:
          lcd.print("Gas Limit");
          lcd.setCursor(0, 1);
          lcd.print("   -  ");
          lcd.print(gas_threshold);
          lcd.print("   +   ");
          break;
        case 3:
          lcd.print("Screen is");
          lcd.setCursor(0, 1);
          lcd.print("   -  ");
          if(screen_on==1) lcd.print("ON ");
          else lcd.print("OFF");
          lcd.print("   +   ");   
          break;
        default:
          screen_on=0;
          display_mode=0;
          break;
      }
      t_last_lcd_refresh=millis();
    }
  }
  return;
}

void disable_alarm()
{
  Serial.println("Disabling Alarm");
  alarm_on=0;
  disabling_alarm=0;
  display_mode=0;
  digitalWrite(pin_LED, LOW);
  noTone(pin_BUZZER);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DISABLING ALARM");
  delay(2000);
  lcd.clear();
  screen_on=1;
  lcd.setCursor(0, 0);
  t_alarm_was_disabled=millis();
}
