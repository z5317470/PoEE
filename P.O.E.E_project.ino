//To do:
//Add timeout for each method.... At end 
//Add one method for the push button so its not repeated so much..... maybe call function and it returns 1,2 or 3 depending on button press
//Add sub functions for toggle/activate manual water and maybe auto water

#include <CAN.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <Constants.h>


char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
char dateBuffer[16];
char Ri = -1, Rii = -1, Li = 16, Lii = 0;
int result, count = 0;
float temp_arr[3] = {0, 0, 0}; int voltage[2] = {0,0}; // current , on , off
int moisture_arr[2] = {0, 0}, flow_Rate = 0;; // current, on
byte time_arr[4] = {0, 0, 0, 0}; // hour on, min on, hour off, min off
byte fin_Time_arr[4] = {0, 0, 0, 0}; // hour on, min on, hour off, min off
byte watering_Status, battery_Status, state = 0, hour_Set = 0, reset_Count = 0, min_Set = 1, month_Set = 1, date_Set = 1;
byte auto_Water_Temp_Act = 0, auto_Water_Moisture_Act = 0, moisture_Cal = 0, auto_Water_Trig = 0, manual_Water_trig = 0;

RTC_DS1307 rtc;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd( LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//------------------------------------------------------------------------------------------------------------------ -

void setup() {
  Serial.begin(9600);
  pinMode(9, OUTPUT); ///**************************************WTF IS THIS PIN SPEAKER?????????
  pinMode(Next_Pin, INPUT);
  pinMode(Cancel_Pin, INPUT);
  pinMode(Confirm_pin, INPUT);

  lcd.begin(16, 2); // set up the LCD's columns and rows

  lcd.setCursor(0, 0);
  lcd.print(F("SBC Watering"));
  lcd.setCursor(0, 1);
  lcd.print(F("System"));

  delay(2000);
  lcd.clear();

  //#ifndef ESP8266
  //  while (!Serial); // wait for serial port to connect. Needed for native USB
  //#endif

  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  }

  CAN.setClockFrequency(16e6);
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    lcd.print(F("CAN failed!"));
    delay(20000);
    while (1);
  }
  else {
    lcd.print(F("CAN successful"));
    delay(2000);
  }

}

//----------------------------------------------------------------------------------------------

void loop() {


  switch (state) {

    //------------------------------------Hold State--------------------------------------------------------------
    case HOLD_STATE:
      count = 0;

      Print_Date_Time();

      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Please press enter to begin")));


      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        if (digitalRead(Confirm_pin) == 1) {
          Delay(200);
          state = WATER_OPTIONS;

          break;
        }
        delay(20);
      }
      break;


    //--------------------------------------Water Options-----------------------------------------------------------
    case WATER_OPTIONS:
      Check_CAN();
      Check_Auto_Water();
      Print_Date_Time();

      lcd.setCursor(0, 1);
      lcd.print(F("Watering Options         "));

      // Moves to next options
      if (digitalRead(Next_Pin) == 1) {
        Delay(200);
        state = POWER_OPTIONS;
        break;
      }

      //Returns to hold state
      else if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        state = HOLD_STATE;
        break;
      }

      //Runs water options function to interact with other modules
      else if (digitalRead(Confirm_pin) == 1) {
        //Make sub cases
        Delay(200);

        state = SET_WATERING_TIME;
        break;
      }
      break;

    //------------------------------------Power Options--------------------------------------------------------------

    case POWER_OPTIONS:

      Check_CAN();
      Check_Auto_Water();
      Print_Date_Time();

      lcd.setCursor(0, 1);
      lcd.print(F("Power Options       "));

      if (digitalRead(Next_Pin) == 1) {
        Delay(200);
        state = SENSOR_OPTIONS;
        break;
      }
      else if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        state = HOLD_STATE;
        break;
      }
      else if (digitalRead(Confirm_pin) == 1) {
        //Make sub cases
        Delay(200);

        state = VOLTAGE_LEVEL;
        break;
      }

      break;







    //-----------------------------------Sensor Options---------------------------------------------------------------
    case SENSOR_OPTIONS:

      Check_CAN();
      Check_Auto_Water();
      Print_Date_Time();

      lcd.setCursor(0, 1);
      lcd.print(F("Sensor Options  "));

      if (digitalRead(Next_Pin) == 1) {
        Delay(200);
        state = WATER_OPTIONS;
        break;
      }
      else if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        state = HOLD_STATE;
        break;
      }
      else if (digitalRead(Confirm_pin) == 1) {
        //Make sub cases
        Delay(200);

        state = WATERING_SOIL_MOISTURE;
        break;
      }

      break;

    //--------------------------------------Set Watering Time-----------------------------------------------------------
    case SET_WATERING_TIME:

      Print_Date_Time();

      count = 0;
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Set Watering Time         ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();

        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = ACTIVATE_AUTO_WATER;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATER_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          Delay(200);
          reset_Count = 1;
          lcd.clear();

          if (reset_Count == 1) {
            lcd.print(F("Set begin time   "));
            Delay(2000);
            Set_Hour();
            time_arr[HOUR_ON] = hour_Set;

          }
          if (reset_Count == 1) {
            Set_Min();
            time_arr[MIN_ON] = min_Set;
          }

          lcd.clear();

          if (reset_Count == 1) {
            lcd.print(F("Set end time   "));
            Delay(2000);
            Set_Hour();
            time_arr[HOUR_OFF] = hour_Set;

          }
          if (reset_Count == 1) {
            Set_Min();
            time_arr[MIN_OFF] = min_Set;
          }
          if (reset_Count == 1) {

            Set_Watering_time();
          }


          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------Watering Status-----------------------------------------------------------
    case WATERING_STATUS:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Check Watering Status         ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = PUMP_RPM;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATER_OPTIONS;
          break;
        }


        else if (digitalRead(Confirm_pin) == 1) {

          lcd.clear();
          Send_CAN(WATER_MODULE_ID, WATER_STATUS);
          Delay(200);

          lcd.print("Watering is ");
          if (watering_Status == 0) {
            lcd.print("off ");
            Delay(2000);
            break;
          }
          else if (watering_Status == 1) {
            lcd.print("on ");
            Delay(2000);
            break;
          }

          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------Flow rate-----------------------------------------------------------
    case PUMP_RPM:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Check flow rate         ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = SET_WATERING_TIME;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATER_OPTIONS;
          break;
        }


        else if (digitalRead(Confirm_pin) == 1) {
          lcd.setCursor(0, 0);
          lcd.print("Flow rate:      ");
          lcd.setCursor(0, 1);
          lcd.print(flow_Rate);
          lcd.print(" L/M            ");
          Delay(2000);


          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------Activate Manual water-----------------------------------------------------------
    //Add sub function to display auto water status so tyhe user knows if auto water is on or off
    case ACTIVATE_MANUAL_WATER:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Toggle Manual water ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = ACTIVATE_AUTO_WATER;
          break;
        }

        //Returns state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATER_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {

          if (watering_Status  == 0) {
            Send_CAN(WATER_MODULE_ID, WATER_ON);
            manual_Water_trig = 1;

            break;
          }
          else if (watering_Status  == 1) {
            Send_CAN(WATER_MODULE_ID, WATER_OFF);
            manual_Water_trig = 0;

            break;
          }

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Checking");
          lcd.setCursor(0, 1);
          Send_CAN(WATER_MODULE_ID, WATER_STATUS);
          Delay(2000);

          lcd.print("Watering is ");
          if (watering_Status == 0) {
            lcd.print("off ");
            Delay(2000);
            break;
          }
          else if (watering_Status == 1) {
            lcd.print("on ");
            Delay(2000);
            break;
          }
        }
        delay(20);
      }

      break;

    //--------------------------------------Activate Auto water-----------------------------------------------------------
    //Add sub function to display auto water status so tyhe user knows if auto water is on or off
    case ACTIVATE_AUTO_WATER:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Toggle Auto-water ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = WATERING_STATUS;
          break;
        }

        //Returns state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATER_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          lcd.clear();
          Delay(200);

          if (auto_Water_Trig  == 0) {
            lcd.print(F("Auto-water ON"));
            Delay(2000);
            auto_Water_Trig = 1;
            break;
          }
          else if (auto_Water_Trig  == 1) {
            lcd.print(F("Auto-water OFF"));
            Delay(2000);
            auto_Water_Trig = 0;
            break;
          }
        }
        delay(20);
      }
      break;

    //--------------------------------------Voltage level-----------------------------------------------------------
    case VOLTAGE_LEVEL:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Check Voltage level         ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = BATTERY_STATUS;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = POWER_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          lcd.setCursor(0, 0);
          lcd.print("Voltage:          ");
          lcd.setCursor(0, 1);
          lcd.print(voltage[0]);lcd.print(".");lcd.print(voltage[1]);
          lcd.print(" V                ");
          Delay(2000);
          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------Battery Status-----------------------------------------------------------
    case BATTERY_STATUS:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Check Battery Status         ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = VOLTAGE_LEVEL;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = POWER_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          lcd.setCursor(0, 0);
          lcd.print("Battery Status:         ");
          lcd.setCursor(0, 1);
          if (battery_Status == 0) {
            lcd.print("Off                   ");
          }
          else if (battery_Status == 1) {
            lcd.print("On                    ");
          }
          else if (battery_Status == 1) {
            lcd.print("Error                 ");
          }
          Delay(2000);
          break;
        }
        delay(20);
      }
      break;


    //--------------------------------------water soil moisture options-----------------------------------------------------------
    case WATERING_SOIL_MOISTURE:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Soil moisture options ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = WATERING_TEMP;
          break;
        }

        //Returns state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = SENSOR_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {

          Delay(200);

          if (moisture_Cal == 0) {
            Cal_Soil_Moisture_Sensor();
            break;
          }
          if (moisture_Cal == 1) {
            state = WATERING_SOIL_MOISTURE_ACTIVATE;
            break;
          }
        }
        delay(20);
      }
      break;

    //--------------------------------------watering temp options-----------------------------------------------------------
    case WATERING_TEMP:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Watering Tempreture Options ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = SET_RTC_TIME;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = SENSOR_OPTIONS;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          //Make sub cases
          Delay(200);

          state = WATERING_TEMP_ACTIVATE;
          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------water soil moisture activate-----------------------------------------------------------
    case WATERING_SOIL_MOISTURE_ACTIVATE:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Activate soil moisture level Autowater ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = WATERING_SOIL_MOISTURE_SET;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATERING_SOIL_MOISTURE;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          Delay(200);
          Activate_Soil_Moisture_Limit();
          Delay(200);
        }
        delay(20);
      }
      break;

    //--------------------------------------Set water soil moisture trig-----------------------------------------------------------
    case WATERING_SOIL_MOISTURE_SET:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Set soil moisture trig levels ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = WATERING_SOIL_MOISTURE_ACTIVATE;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATERING_SOIL_MOISTURE;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          Delay(200);
          Set_Soil_Moisture_limit();

          Delay(200);

          //call function
          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------water Temp activate-----------------------------------------------------------
    case WATERING_TEMP_ACTIVATE:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Activate tempreture auto water levels ")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = WATERING_TEMP_SET;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATERING_TEMP;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          Delay(200);
          Activate_Temp_limits();
          Delay(200);
          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------Set water temp trig-----------------------------------------------------------
    case WATERING_TEMP_SET:

      count = 0;
      Print_Date_Time();
      lcd.setCursor(0, 1);
      lcd.print(Scroll_LCD_Left(F("Set temp trigger levels")));

      while (count <= 25) {
        count++;
        Check_CAN();
        Check_Auto_Water();
        // Moves to next options
        if (digitalRead(Next_Pin) == 1) {
          Delay(200);
          state = WATERING_SOIL_MOISTURE_ACTIVATE;
          break;
        }

        //Returns to hold state
        else if (digitalRead(Cancel_Pin) == 1) {
          Delay(200);
          state = WATERING_SOIL_MOISTURE;
          break;
        }

        //Runs water options function to interact with other modules
        else if (digitalRead(Confirm_pin) == 1) {
          Delay(200);
          reset_Count = 1;

          if (reset_Count == 1) {
            Set_Temp_Lower_limit();
          }
          if (reset_Count == 1) {
            Set_Temp_Upper_limit();
            reset_Count = 0;
          }

          break;
        }
        delay(20);
      }
      break;

    //--------------------------------------Set RTC-----------------------------------------------------------
    case SET_RTC_TIME:

      //Used to prevent the button being pressed unintended
      Delay(200);

      Check_CAN();
      Check_Auto_Water();
      Print_Date_Time();

      lcd.setCursor(0, 1);
      lcd.print(F("Set Time         "));

      // Moves to next options
      if (digitalRead(Next_Pin) == 1) {
        Delay(200);
        state = WATERING_SOIL_MOISTURE;
        break;
      }

      //Returns to hold state
      else if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        state = SENSOR_OPTIONS;
        break;
      }

      //Runs water options function to interact with other modules
      else if (digitalRead(Confirm_pin) == 1) {
        reset_Count = 1;
        Delay(200);



        if (reset_Count == 1) {
          Set_Hour();

        }
        if (reset_Count == 1) {
          Set_Min();

        }
        if (reset_Count == 1) {
          Set_Date();

        }
        if (reset_Count == 1) {
          Set_Month();

        }
        if (reset_Count == 1) {
          Set_Time();

        }


        break;

      }
      break;

  }
}
