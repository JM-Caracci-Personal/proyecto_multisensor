```cpp
// Include Libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DHT_U.h>

// Define constants
#define time_out_test 10000 // time out for each test
#define time_interval_test 2000 // time between tests

//Define Pins
# define pin_M 4
# define pin_PLUS 7
# define pin_MINUS 6
# define pin_LED 5
# define pin_DHT 3

// Others
#define DHTTYPE DHT11

// Create  objects
LiquidCrystal_I2C lcd(0x27,16,2);  // Create LCD in the i2c address (my LCD ys 16x2)
DHT dht(pin_DHT, DHTTYPE);	// Create dht


// Global variables
float temp;

void setup() 
{
  Serial.begin(9600);
  Serial.println("START!");

  lcd.init();             // INITIALIZE LCD
  dht.begin();	          // INITIALIZE DHT
  
  pinMode(pin_M, INPUT_PULLUP);
  pinMode(pin_PLUS, INPUT_PULLUP);
  pinMode(pin_MINUS, INPUT_PULLUP);
  pinMode(pin_LED, OUTPUT);


  test_devices();
}

void loop() 
{
}

void test_devices()
{
  lcd.backlight();  //  TURN ON LCD BACKLIGHT
  lcd.clear();
  lcd.print("TESTING...");
  delay(time_interval_test); 

  // TESTING LCD
  test_lcd();
    
  // TESTING BUTTONS
  test_button(pin_M);
  test_button(pin_PLUS);
  test_button(pin_MINUS);

  // TESTING LED (PEND)
  test_led();
  // TESTING DHT (PEND)
  test_dht();
  // TESTING GAS SENSOR (PEND)
  // TESTING BUZZER (PEND)
  // TESTING EEPROM (PEND)
  // END OF TESTS
  lcd.clear();
  lcd.print("END OF TESTS");
  delay(time_interval_test);
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
      case 4:
        buttonName="M";
        break;
      case 7:
        buttonName="+";
        break;
      case 6:
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
  Serial.println(time_test_started);
  while (test_result==0 && (millis()-time_test_started < time_out_test)) // Wait for button or time_out
  {
    if (!digitalRead(pin))
    {
      test_result=1;    
    }
  } 
  lcd.setCursor(0, 1);
  if (test_result==0) // Test not OK or time out
    lcd.print("Button NOT OK");  
  else // Test OK
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
  if (isnan(temp)) 	  // DHT Working OK?
    lcd.print("DHT NOT OK");   
  else
  {
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print(" C");
  }
  delay(time_interval_test);
}
```
