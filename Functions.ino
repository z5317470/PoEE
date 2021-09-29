
//-----------------------------------------LCD---------------------------------------------------------

String Scroll_LCD_Left(String StrDisplay) {

  String lcd_result;
  String StrProcess = " " + StrDisplay + "      ";
  lcd_result = StrProcess.substring(Li, Lii);
  Li++;
  Lii++;
  if (Li > StrProcess.length()) {
    Li = 16;
    Lii = 0;
  }
  return lcd_result;
}

void Clear_Scroll_LCD_Left() {
  Li = 16;
  Lii = 0;
}

//------------------------------------------------MISC----------------------------------------------------------------


void Delay(int del) {
  count = 0;
  while (count < del) {
    Check_CAN();
    Check_Auto_Water();
    delay(20);
    count = count + 20;
  }
}

void Set_Watering_time() {
  Delay(200);
  int loop_count = 0;
  lcd.clear();

  Serial.println(time_arr[HOUR_ON]);
  Serial.println(time_arr[MIN_ON]);
  Serial.println(time_arr[HOUR_OFF]);
  Serial.println(time_arr[MIN_OFF]);
  while (loop_count == 0) {

    lcd.setCursor(0, 0);
    lcd.print(F("Watering time:"));
    lcd.setCursor(0, 1);

    sprintf(dateBuffer, "%02d:%02d to %02d:%02d ", time_arr[HOUR_ON], time_arr[MIN_ON], time_arr[HOUR_OFF], time_arr[MIN_OFF]);
    Serial.print(dateBuffer);
    lcd.print(dateBuffer);

    if (digitalRead(Cancel_Pin) == 1) {
      Delay(200);

      for (byte i = 0; i < sizeof(time_arr); i++) {
        time_arr[i] = 0;
      }
      loop_count = 1;
    }

    else if (digitalRead(Confirm_pin) == 1) {

      Delay(200);
      for (byte i = 0; i < sizeof(time_arr); i++) {
        fin_Time_arr[i] = time_arr[i];
      }
      for (byte i = 0; i < sizeof(time_arr); i++) {
        time_arr[i] = 0;
      }
      loop_count = 1;
    }

  }
}


//-----------------------------------------CAN-----------------------------------------------------------------------

void Check_CAN() {

  result = CAN.parsePacket();     // see if any packets are available
  if (result) {

    int ID = CAN.packetId();

    if (ID == RETURN_VOLTAGE) {
      int arr_Count=0;
      while (CAN.available()) {
        voltage[arr_Count] = (int)CAN.read();
        arr_Count++;
      }
    }

    else if (ID == RETURN_BATTERY_STATUS) {
      while (CAN.available()) {

        battery_Status = (byte)CAN.read();

      }
    }

    else if (ID == RETURN_MOTOR_RPM) {
      while (CAN.available()) {

        flow_Rate = (int)CAN.read();

      }
    }

    else if (ID == RETURN_WATERING_STATUS) {
      while (CAN.available()) {

        watering_Status = (byte)CAN.read();

      }
    }

    else if (ID == RETURN_SOIL_MOISTURE) {
      while (CAN.available()) {

        moisture_arr[CURRENT_MOISTURE] = (int)CAN.read();
        Serial.print(moisture_arr[CURRENT_MOISTURE]);
      }
    }

    else if (ID == RETURN_CAL_SENSOR ) {
      while (CAN.available()) {

        moisture_Cal = (int)CAN.read();
      }
    }

    else if (ID == RETURN_TEMP) {
      while (CAN.available()) {

        moisture_Cal = (byte)CAN.read();
      }
    }
  }
}

void Send_CAN(int address, int msg) {
  CAN.beginPacket(address);
  CAN.write(msg);
  result = CAN.endPacket();

  if (!result) {
    lcd.print(F("CAN failed!"));
    Delay(2000);
    lcd.clear();
  }
}


//-------------------------------------------Auto water---------------------------------------------------------------------

void Check_Auto_Water() {
  byte water_function = 0;
  DateTime  now = rtc.now();
  if (auto_Water_Trig == 1 && manual_Water_trig == 0) {

    if (watering_Status == 0 && now.hour() >= fin_Time_arr[HOUR_ON] && now.minute() > fin_Time_arr[MIN_ON]
        && now.hour() <= fin_Time_arr[HOUR_OFF] && now.minute() < fin_Time_arr[MIN_OFF]) {

      water_function = 1;

    }

    if (watering_Status == 1 || water_function == 1) {

      if ((now.hour() <= fin_Time_arr[HOUR_ON] && now.minute() < fin_Time_arr[MIN_ON])
          || (now.hour() >= fin_Time_arr[HOUR_OFF] && now.minute() > fin_Time_arr[MIN_OFF])) {

        water_function = 2;
      }
      if (moisture_arr[CURRENT_MOISTURE] > moisture_arr[MOISTURE_TRIG] && auto_Water_Moisture_Act == 1) {

        water_function = 2;
      }
      if ((temp_arr[CURRENT_TEMP] < temp_arr[TEMP_LOW] || temp_arr[CURRENT_TEMP] > temp_arr[TEMP_HIGH]) && auto_Water_Temp_Act == 1) {

        water_function = 2;
      }
    }
  }

  if (water_function != 0) {

    if (water_function == 1) {
      Send_CAN(WATER_MODULE_ID, WATER_ON);
    }
    else if (water_function == 2) {
      Send_CAN(WATER_MODULE_ID, WATER_OFF);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Autowater toggle"));   //Maybe remove to prevent soft locking the system
    lcd.setCursor(0, 1);
    lcd.print(F("Please wait"));
    Delay(2000);
  }
}


void Activate_Temp_limits() {

  bool loop_count = 0;
  lcd.clear();
  while (loop_count == 0) {
    count = 0;
    Print_Date_Time();
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Left(F("Toggle Tempreture limits  ")));
    lcd.setCursor(0, 1);
    lcd.print(F("Limit is: "));

    if (auto_Water_Temp_Act == 0) {
      lcd.print(F("off"));
    }
    else if (auto_Water_Temp_Act == 1) {
      lcd.print(F("on"));
    }
    else {
      lcd.print(F("error"));
    }
    while (count <= 25) {
      count++;
      Check_CAN();
      Check_Auto_Water();

      //Returns to hold state
      if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        loop_count = 1;

      }


      else if (digitalRead(Confirm_pin) == 1) {

        auto_Water_Temp_Act = 1 - auto_Water_Temp_Act;

      }
      delay(20);
    }
  }
}

void Activate_Soil_Moisture_Limit() {

  bool loop_count = 0;
  lcd.clear();

  while (loop_count == 0) {
    count = 0;
    Print_Date_Time();
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Left(F("Toggle Soil Moisture limit  ")));
    lcd.setCursor(0, 1);
    lcd.print(F("Limit is: "));

    if (auto_Water_Moisture_Act == 0) {
      lcd.print(F("off"));
    }
    else if (auto_Water_Moisture_Act == 1) {
      lcd.print(F("on  "));
    }
    else {
      lcd.print(F("error"));
    }
    while (count <= 25) {
      count++;
      Check_CAN();
      Check_Auto_Water();

      //Returns to hold state
      if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        loop_count = 1;

      }


      else if (digitalRead(Confirm_pin) == 1) {

        auto_Water_Moisture_Act = 1 - auto_Water_Moisture_Act;
        loop_count = 1;
      }
      delay(20);
    }
  }
}

//--------------------------------------------maybe combind upper and lower set into one function
void Set_Temp_Lower_limit() {


  bool loop_count = 0;

  while (loop_count == 0) {
    count = 0;
    Print_Date_Time();
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Left(F("Set Lower Tempreture Limits  ")));
    lcd.setCursor(0, 1);

    while (count <= 25) {
      count++;
      Check_CAN();
      Check_Auto_Water();
      lcd.print(F("Lower limit: "));
      lcd.print(temp_arr[TEMP_LOW]);
      lcd.print(" C");

      if (digitalRead(Next_Pin) == 1) {
        Delay(200);

        temp_arr[TEMP_LOW] = temp_arr[TEMP_LOW] + 1;
        if (temp_arr[TEMP_LOW] > 40) {
          temp_arr[TEMP_LOW] = 0;
        }
        if (temp_arr[TEMP_LOW] >= temp_arr[TEMP_HIGH]) {
          lcd.print(F("Too High "));
          Delay(2000);
          temp_arr[TEMP_LOW] = temp_arr[TEMP_HIGH] - 1;
        }
      }

      //Returns to hold state
      if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        loop_count = 1; reset_Count = 0;
        temp_arr[TEMP_LOW] = 0;
      }


      else if (digitalRead(Confirm_pin) == 1) {
        Delay(200);
        loop_count = 1;

      }
      delay(20);
    }
  }
}

void Set_Temp_Upper_limit() {

  bool loop_count = 0;

  while (loop_count == 0) {
    count = 0;
    Print_Date_Time();
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Left(F("Set Upper Tempreture Limits  ")));
    lcd.setCursor(0, 1);
    while (count <= 25) {
      count++;
      Check_CAN();
      Check_Auto_Water();
      lcd.print(F("Upper limit: "));
      lcd.print(temp_arr[TEMP_HIGH]);
      lcd.print(" C");

      if (digitalRead(Next_Pin) == 1) {
        Delay(200);

        temp_arr[TEMP_HIGH] = temp_arr[TEMP_HIGH] + 1;

        if (temp_arr[TEMP_HIGH] <= temp_arr[TEMP_LOW]) {
          lcd.print(F("Too low  "));
          Delay(2000);
          temp_arr[TEMP_HIGH] = temp_arr[TEMP_LOW] + 1;

        }
        if (temp_arr[TEMP_HIGH] > 40) {
          temp_arr[TEMP_HIGH] = 0;
        }
      }

      //Returns to hold state
      if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        loop_count = 1; reset_Count = 0;
        temp_arr[TEMP_HIGH] = 0;
      }


      else if (digitalRead(Confirm_pin) == 1) {

        Delay(200);
        loop_count = 1;

      }
      delay(20);
    }
  }
}

void Set_Soil_Moisture_limit() {


  bool loop_count = 0;

  while (loop_count == 0) {
    count = 0;
    Print_Date_Time();
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Left(F("Set Moisture Limit  ")));
    lcd.setCursor(0, 1);
    while (count <= 25) {
      count++;
      Check_CAN();
      Check_Auto_Water();
      lcd.print(F("Limit: "));
      lcd.print(moisture_arr[MOISTURE_TRIG]);
      lcd.print(F(" %"));

      if (digitalRead(Next_Pin) == 1) {
        Delay(200);

        moisture_arr[MOISTURE_TRIG] = moisture_arr[MOISTURE_TRIG] + 1;
        if (moisture_arr[MOISTURE_TRIG] > 100) {
          moisture_arr[MOISTURE_TRIG] = 0;
        }
      }

      //Returns to hold state
      if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        loop_count = 1; reset_Count = 0;
        moisture_arr[MOISTURE_TRIG] = 0;
      }


      else if (digitalRead(Confirm_pin) == 1) {
        Delay(200);
        loop_count = 1;

      }
      delay(20);
    }
  }
}

void Cal_Soil_Moisture_Sensor() {
  Delay(200);

  bool loop_count = 0;

  while (loop_count == 0) {
    lcd.setCursor(0, 0);
    lcd.print(F("Calibrating "));
    lcd.setCursor(0, 1);
    lcd.print(Scroll_LCD_Left(F("Please Press enter to begin  ")));
    count = 0;
    while (count <= 25) {
      count++;
      Check_CAN();
      Check_Auto_Water();

      //Returns to hold state
      if (digitalRead(Cancel_Pin) == 1) {
        Delay(200);
        loop_count = 1;
      }


      else if (digitalRead(Confirm_pin) == 1) {
        Delay(200);
        loop_count = 1;
        lcd.clear();
        lcd.print(F("Calibrating"));
        Send_CAN(SENSOR_MODULE_ID, CAL_SENSOR);
        Delay(2000);

        if (moisture_Cal == 1) {
          lcd.print(F("Cal Successful"));
          Delay(2000);
        }
        else if (moisture_Cal == 0) {
          lcd.print(F("Cal Failed"));
          Delay(2000);
        }
        else{
          lcd.print(F("Error"));
          Delay(2000);
        }

      }
      delay(20);
    }
  }
}

//-------------------------------------------DTC---------------------------------------------------------------------
void Print_Date_Time() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" ");
  sprintf(dateBuffer, "%02u/%02u ", now.day(), now.month());
  lcd.print(dateBuffer);
  sprintf(dateBuffer, "%02u:%02u ", now.hour(), now.minute());
  lcd.println(dateBuffer);
}

void Set_Hour() {
  Delay(200);
  hour_Set = 12;
  lcd.clear();
  bool loop_count = 0;

  while (loop_count == 0) {
    lcd.setCursor(0, 0);
    lcd.print(F("Set hour: "));
    lcd.setCursor(0, 1);

    sprintf(dateBuffer, "%02i:%02i", hour_Set, min_Set);
    lcd.print(dateBuffer);

    if (digitalRead(Next_Pin) == 1) {
      Delay(200);
      hour_Set = hour_Set + 1;
      if (hour_Set > 24) {
        hour_Set = 1;
      }
    }

    else if (digitalRead(Cancel_Pin) == 1) {
      Delay(200);
      hour_Set = 1;
      reset_Count = 0;
      loop_count = 1;
    }

    else if (digitalRead(Confirm_pin) == 1) {
      loop_count = 1;
    }
  }
}

void Set_Min() {
  Delay(200);
  lcd.clear();
  bool loop_count = 0;

  while (loop_count == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Set Minute: ");
    lcd.setCursor(0, 1);

    sprintf(dateBuffer, "%02i:%02i", hour_Set, min_Set);
    lcd.print(dateBuffer);

    if (digitalRead(Next_Pin) == 1) {
      Delay(200);
      min_Set = min_Set + 1;
      if (min_Set > 60) {
        min_Set = 1;
      }
    }

    else if (digitalRead(Cancel_Pin) == 1) {
      Delay(200);
      min_Set = 0;
      reset_Count = 0;
      loop_count = 1;
    }

    else if (digitalRead(Confirm_pin) == 1) {
      loop_count = 1;
    }
  }
}

void Set_Date() {
  Delay(200);
  lcd.clear();
  bool loop_count = 0;

  while (loop_count == 0) {
    lcd.setCursor(0, 0);
    lcd.print(F("Set Date: "));
    lcd.setCursor(0, 1);

    sprintf(dateBuffer, "%02i/%02i", date_Set, month_Set);
    lcd.print(dateBuffer);

    if (digitalRead(Next_Pin) == 1) {
      Delay(200);
      date_Set = date_Set + 1;
      if (date_Set > 30) {
        date_Set = 1;
      }
    }

    else if (digitalRead(Cancel_Pin) == 1) {
      Delay(200);
      date_Set = 1;
      reset_Count = 0;
      loop_count = 1;
    }

    else if (digitalRead(Confirm_pin) == 1) {
      loop_count = 1;
    }
  }
}

void Set_Month() {
  Delay(200);
  bool loop_count = 0;
  lcd.clear();

  while (loop_count == 0) {

    lcd.setCursor(0, 0);
    lcd.print(F("Set Month: "));
    lcd.setCursor(0, 1);

    sprintf(dateBuffer, "%02i/%02i", date_Set, month_Set);
    lcd.print(dateBuffer);

    if (digitalRead(Next_Pin) == 1) {
      Delay(200);
      month_Set = month_Set + 1;
      if (month_Set > 12) {
        month_Set = 1;
      }
    }

    else if (digitalRead(Cancel_Pin) == 1) {
      Delay(200);
      month_Set = 1;
      loop_count = 1;
      reset_Count = 0;
    }

    else if (digitalRead(Confirm_pin) == 1) {
      loop_count = 1;
    }
  }
}

void Set_Time() {
  Delay(200);
  bool loop_count = 0;
  lcd.clear();

  while (loop_count == 0) {

    lcd.setCursor(0, 0);
    lcd.print(F("New DTG:"));
    lcd.setCursor(0, 1);

    sprintf(dateBuffer, "%02i:%02i %02i/%02i", hour_Set, min_Set, date_Set, month_Set);
    lcd.print(dateBuffer);

    if (digitalRead(Cancel_Pin) == 1) {
      Delay(200);
      reset_Count = 0;
      loop_count = 1;
    }

    else if (digitalRead(Confirm_pin) == 1) {
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      Delay(200);
      rtc.adjust(DateTime(2021, month_Set, date_Set, hour_Set, min_Set, 0));
      loop_count = 1;
    }

  }
}
