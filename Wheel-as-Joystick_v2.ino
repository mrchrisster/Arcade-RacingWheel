// Version with dead zone and increased sensitivity 

#include <Joystick.h>

// Port Bit/Pin layout   
//      Bit - 76543210 - Silk screen ## - Micro-Controller
#define xPD3 0b00001000 //Digital Pin 0  - Micro/PRO Micro - RX,  INT2
#define xPD2 0b00000100 //Digital Pin 1  - Micro/PRO Micro - TX,  INT3
#define xPD1 0b00000010 //Digital Pin 2  - Micro/PRO Micro - SDA, INT0
#define xPD0 0b00000001 //Digital Pin 3  - Micro/PRO Micro - SCL, INT1
#define xPD_10 (xPD1 | xPD0)
#define xPD_32 (xPD3 | xPD2)
#define xPD4 0b00010000 //Digital Pin 4  - Micro/PRO Micro
#define xPC6 0b01000000 //Digital Pin 5  - Micro/PRO Micro
#define xPD7 0b10000000 //Digital Pin 6  - Micro/PRO Micro
#define xPE6 0b01000000 //Digital Pin 7  - Micro/PRO Micro
#define xPB4 0b00010000 //Digital Pin 8  - Micro/PRO Micro
#define xPB5 0b00100000 //Digital Pin 9  - Micro/PRO Micro
#define xPB6 0b01000000 //Digital Pin 10 - Micro/PRO Micro
#define xPB7 0b10000000 //Digital Pin 11 - Micro
#define xPD6 0b01000000 //Digital Pin 12 - Micro
#define xPC7 0b10000000 //Digital Pin 13 - Micro
#define xPB3 0b00001000 //Digital Pin 14 - PRO Micro
#define xPB1 0b00000010 //Digital Pin 15 - PRO Micro
#define xPB2 0b00000100 //Digital Pin 16 - PRO Micro

#define pinA 2    //The pins that the rotary encoder's A and B terminals are connected to.
#define pinB 3

#define maxBut 10   //Update lastButtonState array below when changing number of elements.
#define axisFlip 6  //Special flip button (button offset: 0 thru 9) - comment out if 'x/y-axis' feature not required by you.

#define DEAD_ZONE 10

//Create a Joystick object.
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
  maxBut, 0,             // Button Count, Hat Switch Count
  true, true, false,     // X and Y Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

//The previous state of the AB pins
volatile int prevQuadratureX = 0;

//Keeps track of how much the encoder has been moved
volatile int rotPositionX = 0;

// Last state of 10 buttons (update array for your maxBut buttons)
int lastButtonState[maxBut] = {1,1,1,1,1,1,1,1,1,1};

void setup() {
  //Use internal input resistors for all the pins we're using  
  PORTD = 0b10010011; //Digital pins D2(1), D3(0), D4(4), and D6(7).
  PORTB = 0b01110111; //Digital pins D8(4), D9(5), D10(6), D15(1).
  PORTC = 0b01000000; //Digital pin D5(6)  
  PORTE = 0b01000000; //Digital pin D7(6)

  //Start the joystick
  Joystick.begin();
  Joystick.setYAxis(511);
  
  //Set up the interrupt handler for the encoder's A and B terminals on digital pins 2 and 3 respectively. Both interrupts use the same handler.
  attachInterrupt(digitalPinToInterrupt(pinA), pinChange, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(pinB), pinChange, CHANGE);
}

//Interrupt handler
void pinChange() {
  int currQuadratureX = PIND & xPD_10;
  int comboQuadratureX = (prevQuadratureX << 2) | currQuadratureX; 

  if(comboQuadratureX == 0b0010 || comboQuadratureX == 0b1011 || comboQuadratureX == 0b1101 || comboQuadratureX == 0b0100) 
    rotPositionX++;                   

  if(comboQuadratureX == 0b0001 || comboQuadratureX == 0b0111 || comboQuadratureX == 0b1110 || comboQuadratureX == 0b1000) 
    rotPositionX--;                   

  prevQuadratureX = currQuadratureX;
  rotPositionX = constrain(rotPositionX, -512, 512);
}

void loop() {
  // Apply dead zone adjustment
  int adjustedPositionX = rotPositionX;
  
  if(abs(rotPositionX) < DEAD_ZONE) {
    adjustedPositionX = 0;
  } else {
    // Increase sensitivity by scaling the adjustedPositionX before mapping
    // This can be adjusted depending on your desired sensitivity.
    // Example: reducing the range of rotPositionX to make it more sensitive.
    adjustedPositionX *= 2; // Increase sensitivity by scaling the position
  }
  
  // Ensure the adjustedPositionX does not exceed the original expected range
  adjustedPositionX = constrain(adjustedPositionX, -512, 512);
  
  // Map adjustedPositionX to joystick X-axis value range (e.g., 0-1023 for a 10-bit ADC resolution)
  int joystickValue = map(adjustedPositionX, -512, 512, 0, 1023);


  
  // Apply dead zone: do not update joystick position if within dead zone
  if(abs(rotPositionX) >= DEAD_ZONE) {
    Joystick.setXAxis(joystickValue);
  } else {
    // Optionally, center the joystick when within the dead zone
    Joystick.setXAxis(512); // Center position assuming a 0-1023 range
  }
  
  // Iterate through the buttons and update their states
  for (int button = 0; button < maxBut; ++button) {
    bool currentButtonState = false; // Assume button is not pressed, adjust based on your setup

    // Using direct port manipulation to read button state, adjust as per actual connection
    switch (button) {
      case 0: // Button 1 on digital pin 4
        currentButtonState = !(PIND & xPD4);
        break;
      case 1: // Button 2 on digital pin 5
        currentButtonState = !(PINC & xPC6);
        break;
      case 2: // Button 3 on digital pin 6
        currentButtonState = !(PIND & xPD7);
        break;
      case 3: // Button 4 on digital pin 7
        currentButtonState = !(PINE & xPE6);
        break;
      case 4: // Button 5 on digital pin 8
        currentButtonState = !(PINB & xPB4);
        break;
      case 5: // Button 6 on digital pin 9
        currentButtonState = !(PINB & xPB5);
        break;
      case 6: // Axis flip button (if used) on digital pin 16
        currentButtonState = !(PINB & xPB2);
        break;
      case 8: // COIN/Select Button on digital pin 10
        currentButtonState = !(PINB & xPB6);
        break;
      case 9: // PLAYER/Start Button on digital pin 15
        currentButtonState = !(PINB & xPB1);
        break;
    }

    // Update the joystick button state
    // Invert the logic if your button wiring is active high
    Joystick.setButton(button, currentButtonState);
  }


}
