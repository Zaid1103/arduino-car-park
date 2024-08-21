// libraries to be used
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <TimeLib.h>
#include <MemoryFree.h>


Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield(); // creating an instance of the Adafruit_RGBLCDShield class to interact with and control the LCD shield


#define A 0
#define B 1

// vehicle struct containing all necessary information required for each vehicle
struct vehicle {
  String reg;
  String type;
  String location;
  String entry;
  String exit;
  bool paid;
};

int current_state = A; // creating variable current state and setting it to A, which is defined above. This variable will be used to switch between the two main states of the program

const int max_vehicles = 12; // max vehicle number is set to 10
vehicle vehicles[max_vehicles]; // creating an array of vehicle objects named 'vehicles'
int vehicle_count = 0; // setting vehicle count to 0
int current_plate_index = 0; // setting current plate index to 0
bool select_released = false; // setting boolean value to false

// creation custom downward arrow
byte customChar1[8] = {
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b11111,
  0b01110,
  0b00100
};

// creation of custom upward arrow
byte customChar2[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b01110
};

// function created to handle button actions, contains further functions to handle the actions of the up, down and select buttons as well as the function to show the free SRAM available
void handle_button_actions(uint8_t buttons) {
  if (buttons) {
    if (buttons & BUTTON_UP) {
      move_current_plate_index(-1);
    }

    if (buttons & BUTTON_DOWN) {
      move_current_plate_index(1);
    }

    if (buttons & BUTTON_SELECT) {
      delay(1000);
      if (buttons & BUTTON_SELECT) {
        show_static_license_plate();
        display_free_sram();
      }
    }
  }

  handle_select_button_release(buttons);
}

// function for extension to display the free SRAM available
void display_free_sram() {
  lcd.setCursor(0, 1);
  lcd.print("Free SRAM: ");
  lcd.print(freeMemory());
  lcd.print(" bytes");
  delay(5000);
  display();
}

// function to move the current plate index
void move_current_plate_index(int direction) {
  if (current_plate_index + direction >= 0 && current_plate_index + direction < vehicle_count) {
    current_plate_index += direction;
    display();
    delay(100);
  }
}

// function to show the student ID when select button is held
void show_static_license_plate() {
  lcd.clear();
  lcd.setBacklight(5);
  lcd.print("F311024");
  select_released = true;
}

// function to handle the release of the select button
void handle_select_button_release(uint8_t buttons) {
  if (!(buttons & BUTTON_SELECT) && select_released) {
    display();
    select_released = false;
  }
}

// error checking function that checks whether the status entered in serial monitor is valid or not as per specification
bool valid_status(String input) {
  if (input.equals("PD") || input.equals("NPD")) {
    return true;
  }
  else
  {
    return false;
  }

}


// error checking function that checks whether reg plate entered is valid or not as per specification
bool valid_reg(String input) {
  //check length of string
  if (input.length() != 7) {
    return false;
  }
  for (int i = 0; i < 2; i++) {
    if (!(input[i] >= 'A' && input[i] <= 'Z')) {
      return false;
    }
  }
  for (int i = 2; i < 4; i++) {
    if (!(input[i] >= '0' && input[i] <= '9' )) {
      return false;
    }
  }
  for (int i = 4; i < 7; i++ ) {
    if (!(input[i] >= 'A' && input[i] <= 'Z')) {
      return false;
    }
  }
  return true;
}

// error checking function that checks whether location entered is valid or not as per specification
bool valid_location(String input) {
  //check if input is empty
  if (input.length() == 0) {
    return false;
  }
  //check if length is between 1 and 11 characters
  if (input.length() > 11 || input.length() < 1) {
    return false;
  }
  //check all character in correct range
  for (int i = 0; i < input.length(); i++) {
    if (!((input.charAt(i) >= 'A' && input.charAt(i) <= 'Z') || (input.charAt(i) >= 'a' && input.charAt(i) <= 'z') || (input.charAt(i) >= 0 && input.charAt(i) <= 9) || (input.charAt(i) == '.' ))) {
      return false;
    }
  }
  //check if there are any spaces
  if (input.indexOf(' ') != -1) {
    return false;
  }
  return true;
}

// error checking function that checks whether vehicle type entered is valid or not as per specification
bool valid_type(char x) {

  if (!((x == 'C') || (x == 'M') || (x == 'B') || (x == 'V') || (x == 'L'))) {
    return false;
  }
  return true;
}

// error checking function that checks whether the correct number of dashes are present in the input as per specification
bool correct_dashes(String input, int exp_count) {
  int separator_count = 0;
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == '-') {
      separator_count++;
    }
  }
  //check if count matches expected count
  if (separator_count == exp_count) {
    return true;
  }
  else {
    return false;
  }
}

void display() {
  lcd.clear();

  if (current_plate_index == 0 && vehicle_count > 1) {
    lcd.setCursor(0, 1);
    lcd.write((byte)0);
  } else if (current_plate_index > 0 && current_plate_index < vehicle_count - 1) {
    lcd.setCursor(0, 0);
    lcd.write((byte)1);
    lcd.setCursor(0, 1);
    lcd.write((byte)0);
  } else if (current_plate_index > 0 && current_plate_index == vehicle_count - 1) {
    lcd.setCursor(0, 0);
    lcd.write((byte)1);
  }

  displayVehicleInfo();

  if (vehicle_count == 0) {
    lcd.clear();
    lcd.setBacklight(7);
  }
}

int find_vehicle_index(String reg) {
  for (int i = 0; i < vehicle_count; i++) {
    if (vehicles[i].reg == reg) {
      return i; // return the index of the vehicle if found
    }
  }
  return -1; // return -1 if the vehicle is not found
}

//function that displays vehicle info on the screen
void displayVehicleInfo() {
  lcd.setCursor(1, 0);
  lcd.print(vehicles[current_plate_index].reg.substring(0, 7));

  lcd.setCursor(9, 0);
  lcd.print(vehicles[current_plate_index].location.substring(0, 7));

  lcd.setCursor(1, 1);
  lcd.print(vehicles[current_plate_index].type);

  if (vehicles[current_plate_index].paid) {
    lcd.setBacklight(2);
    lcd.setCursor(4, 1);
    lcd.print("PD");
  } else {
    lcd.setBacklight(3);
    lcd.setCursor(3, 1);
    lcd.print("NPD");
  }

  lcd.setCursor(7, 1);
  lcd.print(vehicles[current_plate_index].entry);

  if (vehicles[current_plate_index].exit != "9999") {
    lcd.setCursor(12, 1);
    lcd.print(vehicles[current_plate_index].exit);
  }
}


void setup() {
  Serial.begin(9600);
  setTime(6, 0, 0, 1, 1, 2023); // setting time to 6:00
  lcd.begin(16, 2);
  lcd.setBacklight(5); // setting backlight to purple
  lcd.createChar(0, customChar1); // custom arrow 1
  lcd.createChar(1, customChar2); // custom arrow 2

}

void loop() {
  static unsigned long lastTime = 0; //unsigned long is a data type used to store large non negative integers, thus used for time keeping
  unsigned long currentTime = millis(); // setting current time using millis function, used later to ensure Q is sent once per second in initialisation

  // switch statement, using current state variable defined above
  switch (current_state) {
    // first case, the initialisation phase
    case A:
      {
        //Send character 'Q' once per second
        if (currentTime - lastTime >= 1000) {
          Serial.print('Q');
          lastTime = currentTime;
          if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            if (input.charAt(0) == 'X') {
              Serial.println(F(" "));
              Serial.println(F("UDCHARS FREERAM"));
              Serial.println(F("NL"));
              lcd.setBacklight(7);
              current_state = B; // swtiches current state to B once initialisation phase is complete
              delay(100);
            }
          }

        } break;
      }
    // second case, contains the main protocols
    case B:
      {
        delay(1000);
        if (Serial.available() > 0) {
          String user_input = Serial.readStringUntil('\n');
          char choice = user_input.charAt(0); // variable created to house the protocol entered by the user(the first character in string), variable is used in following switch statement

          // nested switch statement within case B, used to switch between protocols as entered by the user
          switch (choice) {
            // the append case, used to add vehicles into the system
            case 'A':
              {
                // Correct dashes check using if statement
                if (correct_dashes(user_input, 3)) {
                  user_input = user_input.substring(2); // Removes the first two characters in userinput

                  int dash1 = user_input.indexOf('-'); // Assigns the index value of the first dash to variable dash1
                  int dash2 = user_input.indexOf('-', dash1 + 1); // Assigns the index value of the second dash to variable dash2

                  String reg = user_input.substring(0, dash1); // Assigns the reg plate to variable reg, gained between the index 0 and the value assigned to dash1 above
                  char type = user_input.charAt(dash1 + 1); // Assigns the vehicle type to variable type
                  String loc = user_input.substring(dash2 + 1); // Assigns the location to the variable loc

                  //error checks for registration plate, vehicle type, vehicle location
                  if (valid_reg(reg)) {
                    if (valid_type(type)) {
                      if (valid_location(loc)) {
                        int existing_index = find_vehicle_index(reg);

                        // Check if the vehicle plate already exists, if does, allows for changing of location and/or type after a check if paid
                        if (existing_index != -1) {
                          // Check if the payment status is Paid (PD)
                          if (vehicles[existing_index].paid) {
                            // Update vehicle type and location
                            vehicles[existing_index].type = type;
                            vehicles[existing_index].location = loc;
                            Serial.println(F("DEBUG: Vehicle type and/or location updated for a paid vehicle"));
                            display();
                          } else {
                            Serial.println(F("ERROR: Cannot modify due to non-payment status")); 
                          }
                        } else {
                          // Vehicle doesn't exist, add a new vehicle
                          time_t t = now(); // Returns the current time
                          int timeHHMM = (hour(t) * 100) + minute(t); // Formatting time into HHMM format
                          String formatted_time = "";

                          // Adds a leading 0 if time is less than 1000 to maintain the format of HHMM
                          if (timeHHMM < 1000) {
                            formatted_time = "0" + String(timeHHMM);
                          } else {
                            formatted_time = String(timeHHMM);
                          }
                          // appending all info into Vehiclex
                          vehicle Vehiclex;
                          Vehiclex.reg = reg;
                          Vehiclex.type = type;
                          Vehiclex.location = loc;
                          Vehiclex.entry = formatted_time;
                          Vehiclex.exit = "9999";
                          Vehiclex.paid = false;

                          // check to make sure max vehicle count is not reached 
                          if (vehicle_count < max_vehicles) {
                            vehicles[vehicle_count] = Vehiclex; // adding into vehicles array 
                            vehicle_count++;
                            Serial.println(F("DEBUG: Vehicle added"));
                            current_plate_index = 0;
                            display(); // calling function defined above to display on lcd screen
                            //error messages if any of checks fail
                          } else {
                            Serial.println(F("ERROR: Max vehicle number reached")); 
                          }
                        }
                      } else {
                        Serial.println(F("ERROR: Invalid vehicle location entered"));
                      }
                    } else {
                      Serial.println(F("ERROR: Invalid vehicle type entered"));
                    }
                  } else {
                    Serial.println(F("ERROR: Invalid registration plate number entered"));
                  }
                } else {
                  Serial.println(F("ERROR: Incorrect number of dashes"));
                }
                break;
              }
            // the status case, used to change payment status of vehicles
            case 'S':
              {
                // correct dashes check 
                if (correct_dashes(user_input, 2)) {
                  user_input = user_input.substring(2); // altering userinput so it removes first two characters

                  int dash1 = user_input.indexOf('-'); 

                  String reg = user_input.substring(0, dash1); // stroing reg plate 
                  String ispaid = user_input.substring(dash1 + 1); // storing payment status entered

                  //error checks 
                  if (valid_status(ispaid)) {
                    bool isthere = false; //if user inputs
                    for (int i = 0; i < vehicle_count; i++) {
                      // check if vehicle exists in system
                      if (vehicles[i].reg == reg) {
                        isthere = true;
                        if (ispaid == "PD") {
                          if (vehicles[i].paid == true) {
                            Serial.println(F("ERROR: vehicle status already set to paid"));
                            display();
                          }
                          else {
                            vehicles[i].paid = true; // update status 

                            //time setting 
                            time_t t = now();


                            int timeHHMM = (hour(t) * 100) + minute(t);

                            String formatted_time = "";
                            if (timeHHMM < 1000) {
                              formatted_time = "0" + String(timeHHMM);
                            }
                            else {
                              formatted_time = "0" + String(timeHHMM);
                            }
                            vehicles[i].exit = formatted_time; // exit time 
                            Serial.println(F("DEBUG: Payment status changed: Paid"));
                            current_plate_index = 0;
                            display(); // display on lcd
                          }
                        }
                        else if (ispaid == "NPD") {
                          if (vehicles[i].paid == false) {
                            Serial.println(F("ERROR: vehicle status already set to not paid"));
                          }
                          else {
                            vehicles[i].paid = false;
                            // time setting if change from PD to NPD
                            time_t t = now();


                            int timeHHMM = (hour(t) * 100) + minute(t);

                            String formatted_time = "";
                            if (timeHHMM < 1000) {
                              formatted_time = "0" + String(timeHHMM);
                            }
                            else {
                              formatted_time = "0" + String(timeHHMM);
                            }
                            vehicles[i].entry = formatted_time;
                            vehicles[i].exit = 9999;
                            Serial.println(F("DEBUG: Payment status changed: Not Paid"));
                            current_plate_index = 0;
                            display(); // display on lcd 
                          }
                        }
                      }
                    }
                    // error messages 
                  } else {
                    Serial.println(F("ERROR: invalid payment status entered"));
                  }
                } else {
                  Serial.println(F("ERROR: incorrect number of dashes"));
                }
                break;
              }
            // case type, used to change type of vehicle 
            case 'T':
              {
                // correct daashes check 
                if (correct_dashes(user_input, 2)) {
                  user_input = user_input.substring(2);

                  int dash1 = user_input.indexOf('-'); 

                  String reg = user_input.substring(0, dash1); // storing reg 
                  char type = user_input.charAt(dash1 + 1); //storing type 

                  // error checks 
                  if (valid_reg(reg)) {
                    if (valid_type(type)) {

                      for (int i = 0; i < vehicle_count; i++) {
                        // check if vehicle exists 
                        if (vehicles[i].reg == reg) {
                          //check if paid or not
                          if (vehicles[i].paid == false) {
                            Serial.println(F("ERROR: type cannot be modified as not paid"));
                          }
                          else {
                            if (vehicles[i].type == type) {
                              Serial.println(F("Error: cannot be modified as same vehicle type"));
                            }
                            else {
                              vehicles[i].type = type; // updating type 
                              Serial.println(F("DEBUG: Vehicle type changed successfully"));
                              current_plate_index = 0;
                              display(); // displaying on lcd 
                            }
                          }
                          // error messages
                        } else {
                          Serial.println(F("ERROR: reg plate not found"));
                        }
                      }
                    } else {
                      Serial.println(F("ERROR: invalid vehicle type entered"));
                    }
                  } else {
                    Serial.println(F("ERROR: invalid reg plate entered"));
                  }
                } else {
                  Serial.println(F("ERROR: incorrect number of dashes"));
                }
                break;
              }
              // case location, used to change location of vehicle 
            case 'L':
              {
                // correct dashes check 
                if (correct_dashes(user_input, 2)) {
                  user_input = user_input.substring(2);

                  int dash1 = user_input.indexOf('-');

                  String reg = user_input.substring(0, dash1); // storing reg 
                  String loc = user_input.substring(dash1 + 1); // storing location 
                  //error checks 
                  if (valid_reg(reg)) {
                    if (valid_location(loc)) {

                      for (int i = 0; i < vehicle_count; i++) {
                        // check if reg exists 
                        if (vehicles[i].reg == reg) {
                          // check if location is the same 
                          if (vehicles[i].location == loc) {
                            Serial.println(F("ERROR: cannot be modified as location is the same"));
                          }
                          // check if status is paid or not 
                          if (vehicles[i].paid = false) {
                            Serial.println(F("ERROR: cannot be modified as not paid"));
                          }
                          else {
                            vehicles[i].location = loc;
                            Serial.println(F("DEBUG: Vehicle location successfully changed")); // updating location 
                            current_plate_index = 0;
                            display(); // displaying on lcd 
                          }
                        }
                      }
                      // error messages 
                    } else {
                      Serial.println(F("ERROR: invalid location entered"));
                    }
                  } else {
                    Serial.println(F("ERROR: invalid reg plate entered"));
                  }
                } else {
                  Serial.println(F("ERROR: incorrect number of dashes entered"));
                }
                break;
              }
              // case remove, used to remove a vehicle from the system 
            case 'R':
              {
                // correct dashes check 
                if (correct_dashes(user_input, 1)) {
                  user_input = user_input.substring(2);

                  // error checks 
                  if (valid_reg(user_input)) {
                    bool is_there = false;
                    for (int i = 0; i < vehicle_count; i++) {
                      // check if vehicle exists in system 
                      if (vehicles[i].reg = user_input) {
                        is_there = true;
                        //check if paid or not 
                        if (vehicles[i].paid == false) {
                          Serial.println(F("ERROR: cannot remove as not paid"));
                        }
                        else {
                          for (int x = i; x < vehicle_count - 1; x++) {
                            vehicles[x] = vehicles[x + 1]; // removing vehicle 
                          }
                          vehicle_count--; // reducing vehicle count 
                          Serial.println(F("DEBUG: Vehicle successfully removed"));
                        }
                      }
                      // error messages 
                      if (is_there = false) {
                        Serial.println(F("ERROR: vehicle not found"));
                      }
                    }
                  } else {
                    Serial.println(F("ERROR: invalid reg plate entered"));
                  }
                } else {
                  Serial.println(F("ERROR: incorrect number of dashes"));
                }
                break;
              }
              // case for incorrect usage
            default:
              {
                Serial.println("Error: incorrect usage, please enter statement starting with A-, S-, T-, L- or R-");
                break;
              }
          }
          break;
        }

        // variable buttons will contain information about which buttons on the LCD shield are currently pressed or held.
        uint8_t buttons = lcd.readButtons(); 
        // calling function to handle all button actions 
        handle_button_actions(buttons);
      }
  }
}
