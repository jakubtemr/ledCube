// Code Example for jolliFactory 3D Audio Spectrum Visualizer  example 1.0
// Code adapted from http://tronixstuff.com/2013/01/31/tutorial-arduino-and-the-msgeq7-spectrum-analyzer/


/* Wirings

SPI connections between Arduino Nano/UNO and jolliCube board
- Left audio channel: MOSI (Pin 11), SCK (Pin 13) and SS (Pin 10) at the Arduino side and Din, CLK and Load #1 pins at the jolliCube board respectively.        
- Right audio channel: MOSI (Pin 11), SCK (Pin 13) and SS (Pin 9) at the Arduino side and Din, CLK and Load #2 pins at the jolliCube board respectively.        

Please visit instructable at http://www.instructables.com/id/3D-Stereo-Audio-Spectrum-Visualizer/ for more detail
*/

#include <SPI.h>          
       
int SPI_CS_Right = 9;  // This SPI Chip Select pin controls the MAX7219
int SPI_CS_Left = 10;  // This SPI Chip Select pin controls the MAX7219
int maxInUse = 4;      // No. of MAX72xx IC used for each audio channel
int SetbrightnessValue = 1;   // Valid values 1 to 15

int strobe = 4; // strobe pins on digital 4
int res = 5;    // reset pins on digital 5
int left[7];    // store left band values in these arrays
int right[7];   // store right band values in these arrays
int band;

int cur_leftLevel[7];
int falling_leftLevel[7];
int falling_leftCount[7];

int cur_rightLevel[7];
int falling_rightLevel[7];
int falling_rightCount[7];

char inArray[8];
char outArray[8];
char temp1Array[8];
char temp2Array[8];

byte tempByte;
byte tempByte2;

const int tempFallingLevel[16] = 
{ 
0b0000000000000001, 
0b0000000000000010, 
0b0000000000000100, 
0b0000000000001000, 
0b0000000000010000, 
0b0000000000100000, 
0b0000000001000000, 
0b0000000010000000,

0b0000000100000000,
0b0000001000000000,
0b0000010000000000,
0b0000100000000000,
0b0001000000000000,
0b0010000000000000,
0b0100000000000000,
0b1000000000000000
};

const int tempCurrentLevel[16] = 
{ 
0b0000000000000001, 
0b0000000000000011, 
0b0000000000000111, 
0b0000000000001111, 
0b0000000000011111, 
0b0000000000111111, 
0b0000000001111111, 
0b0000000011111111,

0b0000000111111111,
0b0000001111111111,
0b0000011111111111,
0b0000111111111111,
0b0001111111111111,
0b0011111111111111,
0b0111111111111111,
0b1111111111111111
};



//**********************************************************************************************************************************************************  
void setup()
{
  Serial.begin(115200);
  Serial.println("jolliFactory 3D Stereo Audio Spectrum Visualizer example 1.0");              

  pinMode(SPI_CS_Left, OUTPUT);
  pinMode(SPI_CS_Right, OUTPUT);

  SPI.begin();

  maxTransferAll(0x0F, 0x00);   // 00 - Turn off Test mode
  maxTransferAll(0x09, 0x00);   // Register 09 - BCD Decoding  // 0 = No decoding
  maxTransferAll(0x0B, 0x07);   // Register B - Scan limit 1-7  // 7 = All LEDS
  maxTransferAll(0x0C, 0x01);   // 01 = on 00 = Power saving mode or shutdown

  setBrightness();

  clearDisplays();

  pinMode(res, OUTPUT);      // reset
  pinMode(strobe, OUTPUT);   // strobe
  digitalWrite(res,LOW);     // reset low
  digitalWrite(strobe,HIGH); // strobe high  
}



//**********************************************************************************************************************************************************  
void readMSGEQ7()
// Function to read 7 band equalizers
{
  digitalWrite(res, HIGH);
  digitalWrite(res, LOW);
  for(band=0; band <7; band++)
  {
    digitalWrite(strobe,LOW);    // strobe pin to go to the next band
    delayMicroseconds(30); 
    left[band] = analogRead(0);  // store left band reading
    right[band] = analogRead(1); // store right band reading
    digitalWrite(strobe,HIGH);
  }
}



//**********************************************************************************************************************************************************  
void loop()
{
  readMSGEQ7();

  for (band = 0; band < 7; band++)
  {
    // display values of left channel on serial monitor
    //Serial.print(left[band]);
    //Serial.print(" ");

    falling_leftCount[band]++;

    if (left[band]>=720)       //Level 8
      cur_leftLevel[band] = 8;
    else if (left[band]>=600)  //Level 7
      cur_leftLevel[band] = 7;
    else if (left[band]>=520)  //Level 6
      cur_leftLevel[band] = 6;
    else if (left[band]>=440)  //Level 5
      cur_leftLevel[band] = 5;
    else if (left[band]>=360)  //Level 4
      cur_leftLevel[band] = 4;
    else if (left[band]>=280)  //Level 3
      cur_leftLevel[band] = 3;
    else if (left[band]>=200)  //Level 2
      cur_leftLevel[band] = 2;
    else if (left[band]>=140)  //Level 1
      cur_leftLevel[band] = 1;  
    else  //To consider as noise
    {    
      cur_leftLevel[band] = 0;
    }
  }

  output_Left();


  for (band = 0; band < 7; band++)
  {
    // display values of right channel on serial monitor
    //Serial.print(right[band]);
    //Serial.print(" ");

    falling_rightCount[band]++;

    if (right[band]>=720)       //Level 8
      cur_rightLevel[band] = 8;
    else if (right[band]>=600)  //Level 7
      cur_rightLevel[band] = 7;
    else if (right[band]>=520)  //Level 6
      cur_rightLevel[band] = 6;
    else if (right[band]>=440)  //Level 5
      cur_rightLevel[band] = 5;
    else if (right[band]>=360)  //Level 4
      cur_rightLevel[band] = 4;
    else if (right[band]>=280)  //Level 3
      cur_rightLevel[band] = 3;
    else if (right[band]>=200)  //Level 2
      cur_rightLevel[band] = 2;
    else if (right[band]>=140)  //Level 1
      cur_rightLevel[band] = 1;    
    else  //To consider as noise
    {    
      cur_rightLevel[band] = 0;
    }
  }

  output_Right();
}



//**********************************************************************************************************************************************************  
// Left channel - low frequency audio displayed at extreme left of LED Matrix
void output_Left()
{   
  for (band = 0; band < 7; band++)
  {
  
    if (falling_leftLevel[band] > cur_leftLevel[band])
    {
      if (falling_leftCount[band] > 20)
      {
        falling_leftLevel[band]--;
        falling_leftCount[band] = 0;
      }
    }
    else
      falling_leftLevel[band] = cur_leftLevel[band];

    temp1Array[band] = lowByte(tempFallingLevel[falling_leftLevel[band]-1]) | lowByte(tempCurrentLevel[cur_leftLevel[band]-1]);
    temp2Array[band] = lowByte(tempFallingLevel[falling_leftLevel[band]-1]);
  }
  
  temp1Array[7] = 0;
  temp2Array[7] = 0;
    
  memcpy( inArray, temp1Array, sizeof(inArray) );
  rotateArray();
  memcpy( temp1Array, outArray, sizeof(temp1Array) );
    
  memcpy( inArray, temp2Array, sizeof(inArray) );
  rotateArray();
  memcpy( temp2Array, outArray, sizeof(temp2Array) );
  
  for (int row = 0; row < 8; row++)
  {    
    tempByte = temp1Array[row];
    tempByte2 = temp2Array[row];

    maxTransferSingle_Left(1, row, tempByte);  
    maxTransferSingle_Left(2, row, tempByte2);  
    maxTransferSingle_Left(3, row, tempByte2);  
    maxTransferSingle_Left(4, row, tempByte);  
  }  
}



//**********************************************************************************************************************************************************  
// Right channel - low frequency audio displayed at extreme right
void output_Right()
{   
  for (band = 0; band < 7; band++)
  {
  
    if (falling_rightLevel[band] > cur_rightLevel[band])
    {
      if (falling_rightCount[band] > 20)
      {
        falling_rightLevel[band]--;
        falling_rightCount[band] = 0;
      }
    }
    else
      falling_rightLevel[band] = cur_rightLevel[band];

    temp1Array[band] = lowByte(tempFallingLevel[falling_rightLevel[band]-1]) | lowByte(tempCurrentLevel[cur_rightLevel[band]-1]);
    temp2Array[band] = lowByte(tempFallingLevel[falling_rightLevel[band]-1]);
  }

  temp1Array[7] = 0;
  temp2Array[7] = 0;
    
  memcpy( inArray, temp1Array, sizeof(inArray) );
  rotateArray();
  memcpy( temp1Array, outArray, sizeof(temp1Array) );
    
  memcpy( inArray, temp2Array, sizeof(inArray) );
  rotateArray();
  memcpy( temp2Array, outArray, sizeof(temp2Array) );

  for (int row = 0; row < 8; row++)
  {
    tempByte = temp1Array[row];
    tempByte2 = temp2Array[row];

    // Reverse bits so that the 7 right audio bands are displayed starting from extreme right of LED Matrix
    tempByte = Bit_Reverse(tempByte);
    tempByte2 = Bit_Reverse(tempByte2);

    maxTransferSingle_Right(1, row, tempByte);  
    maxTransferSingle_Right(2, row, tempByte2);  
    maxTransferSingle_Right(3, row, tempByte2);  
    maxTransferSingle_Right(4, row, tempByte);    
  }   
}



//**********************************************************************************************************************************************************  
// Change Max72xx brightness
void setBrightness()
{      
  maxTransferAll(0x0A, SetbrightnessValue);  //Set Brightness
  maxTransferAll(0x00, 0x00);  //No-op commands
}



//**********************************************************************************************************************************************************  
// Clear Display
void clearDisplays()
{     
  for (int y=0; y<8; y++) 
    maxTransferAll(y+1, 0);      
}



//**********************************************************************************************************************************************************  
/**
 * Transfers data to a MAX7219/MAX7221 register of a particular Bi-color LED Matrix module.
 *
 * @param whichMax The Max72xx to load data and value into
 * @param address The register to load data into
 * @param value Value to store in the register
 */
//**********************************************************************************************************************************************************  
void maxTransferAll(uint8_t address, uint8_t value) 
{
  digitalWrite(SPI_CS_Left, LOW); 
  digitalWrite(SPI_CS_Right, LOW); 

  for ( int c=1; c<= maxInUse;c++) 
  {
    SPI.transfer(address);  // specify register
    SPI.transfer(value);    // put data
  }

  digitalWrite(SPI_CS_Left, HIGH); 
  digitalWrite(SPI_CS_Right, HIGH); 
}



//**********************************************************************************************************************************************************  
void maxTransferSingle_Left(uint8_t whichMax, uint8_t address, uint8_t value) 
{
  byte noop_reg = 0x00;    //max7219 No op register
  byte noop_value = 0x00;  //value

  digitalWrite(SPI_CS_Left, LOW); 

  for (int i=maxInUse; i>0; i--) // Loop through our number of Bi-color LED Matrices 
  {
    if (i==whichMax)
    {
      SPI.transfer(address+1);   // Send the register address
      SPI.transfer(value);       // Send the value
    }
    else
    {
      SPI.transfer(noop_reg);    // Send the register address
      SPI.transfer(noop_value);  // Send the value
    }
  }

  digitalWrite(SPI_CS_Left, HIGH);
}



//**********************************************************************************************************************************************************  
void maxTransferSingle_Right(uint8_t whichMax, uint8_t address, uint8_t value) 
{
  byte noop_reg = 0x00;    //max7219 No op register
  byte noop_value = 0x00;  //value

  digitalWrite(SPI_CS_Right, LOW); 

  for (int i=maxInUse; i>0; i--)   // Loop through our number of Bi-color LED Matrices 
  {
    if (i==whichMax)
    {
      SPI.transfer(address+1);   // Send the register address
      SPI.transfer(value);       // Send the value
    }
    else
    {
      SPI.transfer(noop_reg);    // Send the register address
      SPI.transfer(noop_value);  // Send the value
    }
  }

  digitalWrite(SPI_CS_Right, HIGH);
}



//**********************************************************************************************************************************************************  
// Reverse the order of bits in a byte.
// I.e. MSB is swapped with LSB, etc.
unsigned char Bit_Reverse( unsigned char x )
{
   x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
   x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
   x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
   return x;    
} 



//**********************************************************************************************************************************************************  
void rotateArray (void)
{
  int i, j, val;

  for (i = 0; i < 8; i++) {
    outArray[i] = 0;
  }

  //rotate 90* clockwise
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      val = ((inArray[i] >> j) & 1); //extract the j-th bit of the i-th element
      outArray[7-j] |= (val << i); //set the newJ-th bit of the newI-th element
    }
  }
}
