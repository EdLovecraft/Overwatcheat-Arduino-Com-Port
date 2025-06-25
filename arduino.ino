#include <Mouse.h>
#include <Wire.h>
#include <SPI.h>

#include <usbhub.h>
USB Usb;
USBHub Hub(&Usb);

#include <hidboot.h>
HIDBoot<USB_HID_PROTOCOL_MOUSE> HidMouse(&Usb);

int lmb = 0;
int rmb = 0;
int mmb = 0;
int scroll = 0;
int xb1 = 0;
int xb2 = 0;

int dx;
int dy;
int dz;

// ----- Mouse Report Parser
class MouseRptParser : public MouseReportParser {
protected:
  void OnMouseMove(MOUSEINFO *mi);
  void OnMouseScroll(MOUSEINFO *mi); 
  void OnLeftButtonUp(MOUSEINFO *mi);
  void OnLeftButtonDown(MOUSEINFO *mi);
  void OnRightButtonUp(MOUSEINFO *mi);
  void OnRightButtonDown(MOUSEINFO *mi);
  void OnMiddleButtonUp(MOUSEINFO *mi);
  void OnMiddleButtonDown(MOUSEINFO *mi);
  void OnXButton1Up (MOUSEINFO *mi); 
  void OnXButton1Down (MOUSEINFO *mi);
  void OnXButton2Up (MOUSEINFO *mi); 
  void OnXButton2Down (MOUSEINFO *mi);
};

void MouseRptParser::OnMouseMove(MOUSEINFO *mi) {
  dx = mi->dX;
  dy = mi->dY;
};

void MouseRptParser::OnMouseScroll(MOUSEINFO *mi) {
  dz = mi->dZ;
};

void MouseRptParser::OnLeftButtonUp(MOUSEINFO *mi) {
  lmb = 0;
};

void MouseRptParser::OnLeftButtonDown(MOUSEINFO *mi) {
  lmb = 1;
};

void MouseRptParser::OnRightButtonUp(MOUSEINFO *mi) {
  rmb = 0;
};

void MouseRptParser::OnRightButtonDown(MOUSEINFO *mi) {
  rmb = 1;
};

void MouseRptParser::OnMiddleButtonUp(MOUSEINFO *mi) {
  mmb = 0;
};

void MouseRptParser::OnMiddleButtonDown(MOUSEINFO *mi) {
  mmb = 1;
};

void MouseRptParser::OnXButton1Up (MOUSEINFO *mi) {
  xb1 = 0; 
}; 
 
 
void MouseRptParser::OnXButton1Down (MOUSEINFO *mi) { 
  xb1 = 1; 
};
 
 
void MouseRptParser::OnXButton2Up (MOUSEINFO *mi) {
  xb2 = 0; 
}; 
 
 
void MouseRptParser::OnXButton2Down (MOUSEINFO *mi) { 
  xb2 = 1; 
};

// Global variables to hold the command and movement values
String command = "";  // The command received from the serial buffer
int deltaX = 0, deltaY = 0;  // Movement values for the X and Y axes

// Click state management
bool isClicking = false;  // Tracks whether a mouse click is currently happening
unsigned long clickStartTime = 0;  // Marks the time when the click begins
unsigned long clickDuration;  // Specifies how long the click will last in milliseconds

MouseRptParser Prs;

void setup() {
    delay(5000);
    // Initialize serial communication at a baud rate of 115200
    Serial.begin(115200);
    Serial.setTimeout(1);  // Set a short timeout for serial reads
    Mouse.begin();  // Initialize mouse control
    
    // Seed the random number generator for varying click durations
    randomSeed(analogRead(0));  // Use an unconnected analog pin for better randomness

    Usb.Init();
    HidMouse.SetReportParser(0, &Prs);
}

void loop() {
    dx = 0;
    dy = 0;
    dz = 0;

    Usb.Task();

    // Clicking
    if (lmb == 0) {
      Mouse.release(MOUSE_LEFT);
    } else if (lmb == 1) {
      Mouse.press(MOUSE_LEFT);
    }

    if (rmb == 0) {
      Mouse.release(MOUSE_RIGHT);
    } else if (rmb == 1) {
      Mouse.press(MOUSE_RIGHT);
    }

    if (mmb == 0) {
      Mouse.release(MOUSE_MIDDLE);
    } else if (mmb == 1) {
      Mouse.press(MOUSE_MIDDLE);
    }

    if (xb1 == 0) {
      Mouse.release(MOUSE_XB1);
    } else if (xb1 == 1) {
      Mouse.press(MOUSE_XB1);
    }
 
    if (xb2 == 0) {
      Mouse.release(MOUSE_XB2);
    } else if (xb2 == 1) {
      Mouse.press(MOUSE_XB2);
    }

    Mouse.move(dx, dy, dz); // Moving the mouse without pcinput

    // Check if there's any command waiting in the serial buffer
    if (Serial.available() > 0) {
        // Read the incoming command until a newline character
        command = Serial.readStringUntil('\n');
        command.trim();  // Clean up any leading or trailing spaces

        // If the command starts with 'M', it's a mouse movement command
        if (command.startsWith("M")) {
            int commaIndex = command.indexOf(',');  // Find the position of the comma
            // Make sure the command is formatted correctly
            if (commaIndex != -1) {
                // Extract the movement values for X and Y axes
                deltaX = command.substring(1, commaIndex).toInt();  // Get X-axis movement
                deltaY = command.substring(commaIndex + 1).toInt();  // Get Y-axis movement

                // Move the mouse incrementally to prevent sudden jumps
                while (deltaX != 0 || deltaY != 0) {
                    int moveX = constrain(deltaX, -128, 127);  // Limit X movement to avoid overflow
                    int moveY = constrain(deltaY, -128, 127);  // Limit Y movement similarly
                    Mouse.move(moveX, moveY);  // Perform the mouse movement
                    deltaX -= moveX;  // Decrease remaining movement for X-axis
                    deltaY -= moveY;  // Decrease remaining movement for Y-axis
                }
            }
        }
        // If the command starts with 'C', it's a mouse click command
        else if (command.startsWith("C")) {
            // Start the click process if we're not already clicking
            if (!isClicking) {
                Mouse.press(MOUSE_LEFT);  // Press the left mouse button down
                clickStartTime = millis();  // Record the current time as the start of the click
                clickDuration = random(40, 80);  // Choose a random click duration between 40ms and 80ms
                isClicking = true;  // Mark that we're in a clicking state
            }
        }
    }

    // If a click is ongoing, check if it's time to release the button
    if (isClicking) {
        // If the specified click duration has passed, release the button
        if (millis() - clickStartTime >= clickDuration) {
            Mouse.release(MOUSE_LEFT);  // Release the left mouse button
            isClicking = false;  // Reset the clicking state
        }
    }
}