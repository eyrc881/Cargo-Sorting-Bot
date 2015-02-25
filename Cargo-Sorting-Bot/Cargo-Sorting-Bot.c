/*
 * Cargo_Sorting_Bot.c
 *
 * Created: 14-02-2015 16:16:11
 *  Author: Devesh Khandelwal
 */ 

#define F_CPU 14745600
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h> //included to support power function
#include "lcd.h"

#define NO_IND 4

typedef enum 
{
	Red,
	Green,
	Blue,
	Key
} Color;

typedef int bool;
#define true 1
#define false 0

Color iBlocks[NO_IND];


void port_init();
void timer5_init();
void velocity(unsigned char, unsigned char);
void motors_delay();

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
volatile unsigned long int pulse = 0; //to keep the track of the number of pulses generated by the color sensor
volatile unsigned long int red;       // variable to store the pulse count when read_red function is called
volatile unsigned long int blue;      // variable to store the pulse count when read_blue function is called
volatile unsigned long int green;     // variable to store the pulse count when read_green function is called

unsigned long int ShaftCountLeft = 0; //to keep track of left position encoder 
unsigned long int ShaftCountRight = 0; //to keep track of right position encoder
unsigned int Degrees; //to accept angle in degrees for turning
unsigned int count=0;

unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char Left_white_line = 0;
unsigned char Center_white_line = 0;
unsigned char Right_white_line = 0;

//Function to configure ports to enable robot's motion
void motion_pin_config (void) 
{
 DDRA = DDRA | 0x0F;
 PORTA = PORTA & 0xF0;
 DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
 PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

//Function to configure INT4 (PORTE 4) pin as input for the left position encoder
void left_encoder_pin_config (void)
{
 DDRE  = DDRE & 0xEF;  //Set the direction of the PORTE 4 pin as input
 PORTE = PORTE | 0x10; //Enable internal pull-up for PORTE 4 pin
}

//Function to configure INT5 (PORTE 5) pin as input for the right position encoder
void right_encoder_pin_config (void)
{
 DDRE  = DDRE & 0xDF;  //Set the direction of the PORTE 4 pin as input
 PORTE = PORTE | 0x20; //Enable internal pull-up for PORTE 4 pin
}


//Configure PORTB 5 pin for servo motor 1 operation
void servo1_pin_config (void)
{
 DDRB  = DDRB | 0x20;  //making PORTB 5 pin output
 PORTB = PORTB | 0x20; //setting PORTB 5 pin to logic 1
}

//Configure PORTB 6 pin for servo motor 2 operation
void servo2_pin_config (void)
{
 DDRB  = DDRB | 0x40;  //making PORTB 6 pin output
 PORTB = PORTB | 0x40; //setting PORTB 6 pin to logic 1
}

//Configure PORTB 7 pin for servo motor 3 operation
void servo3_pin_config (void)
{
 DDRB  = DDRB | 0x80;  //making PORTB 7 pin output
 PORTB = PORTB | 0x80; //setting PORTB 7 pin to logic 1
}
void buzzer_pin_config (void)
{
 DDRC = DDRC | 0x08;    //Setting PORTC 3 as output
 PORTC = PORTC & 0xF7;    //Setting PORTC 3 logic low to turnoff buzzer
}

void lcd_port_config (void)
{
  DDRC = DDRC | 0xF7; //setting all the LCD pin's direction set as output
  PORTC = PORTC & 0x80; //setting all the LCD pins are set to logic 0 except PORTC 7
}

void color_sensor_pin_config(void)
{
  DDRD  = DDRD | 0xFE; //set PD0 as input for color sensor output
  PORTD = PORTD | 0x01;//Enable internal pull-up for PORTD 0 pin
}

void port_init(void)
{
  buzzer_pin_config();
  lcd_port_config();      //lcd pin configuration
  color_sensor_pin_config();  //color sensor pin configuration
  servo1_pin_config();    //Configure PORTB 5 pin for servo motor 1 operation
  servo2_pin_config();    //Configure PORTB 6 pin for servo motor 2 operation 
  servo3_pin_config();    //Configure PORTB 7 pin for servo motor 3 operation  
  motion_pin_config(); //robot motion pins config
  left_encoder_pin_config(); //left encoder pin config
  right_encoder_pin_config(); //right encoder pin config 
}
void buzzer_on (void)
{
 unsigned char port_restore = 0;
 port_restore = PINC;
 port_restore = port_restore | 0x08;
 PORTC = port_restore;
}

void buzzer_off (void)
{
 unsigned char port_restore = 0;
 port_restore = PINC;
 port_restore = port_restore & 0xF7;
 PORTC = port_restore;
}

void color_sensor_pin_interrupt_init(void) //Interrupt 0 enable
{
  cli(); //Clears the global interrupt
  EICRA = EICRA | 0x02; // INT0 is set to trigger with falling edge
  EIMSK = EIMSK | 0x01; // Enable Interrupt INT0 for color sensor
  sei(); // Enables the global interrupt
}
void left_position_encoder_interrupt_init (void) //Interrupt 4 enable
{
 cli(); //Clears the global interrupt
 EICRB = EICRB | 0x02; // INT4 is set to trigger with falling edge
 EIMSK = EIMSK | 0x10; // Enable Interrupt INT4 for left position encoder
 sei();   // Enables the global interrupt 
}

void right_position_encoder_interrupt_init (void) //Interrupt 5 enable
{
 cli(); //Clears the global interrupt
 EICRB = EICRB | 0x08; // INT5 is set to trigger with falling edge
 EIMSK = EIMSK | 0x20; // Enable Interrupt INT5 for right position encoder
 sei();   // Enables the global interrupt 
}


//Function For ADC Conversion
unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}

//Function To Print Sesor Values At Desired Row And Coloumn Location on LCD
void print_sensor(char row, char coloumn,unsigned char channel)
{
	
	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, coloumn, ADC_Value, 3);
}


//ISR for color sensor
ISR(INT0_vect)
{
  pulse++; //increment on receiving pulse from the color sensor
}

void timer1_init(void)
{
 TCCR1B = 0x00; //stop
 TCNT1H = 0xFC; //Counter high value to which OCR1xH value is to be compared with
 TCNT1L = 0x01; //Counter low value to which OCR1xH value is to be compared with
 OCR1AH = 0x03; //Output compare Register high value for servo 1
 OCR1AL = 0xFF; //Output Compare Register low Value For servo 1
 OCR1BH = 0x03; //Output compare Register high value for servo 2
 OCR1BL = 0xFF; //Output Compare Register low Value For servo 2
 OCR1CH = 0x03; //Output compare Register high value for servo 3
 OCR1CL = 0xFF; //Output Compare Register low Value For servo 3
 ICR1H  = 0x03; 
 ICR1L  = 0xFF;
 TCCR1A = 0xAB; /*{COM1A1=1, COM1A0=0; COM1B1=1, COM1B0=0; COM1C1=1 COM1C0=0}
          For Overriding normal port functionality to OCRnA outputs.
          {WGM11=1, WGM10=1} Along With WGM12 in TCCR1B for Selecting FAST PWM Mode*/
 TCCR1C = 0x00;
 TCCR1B = 0x0C; //WGM12=1; CS12=1, CS11=0, CS10=0 (Prescaler=256)
}

//ISR for right position encoder
ISR(INT5_vect)  
{
 ShaftCountRight++;  //increment right shaft position count
}


//ISR for left position encoder
ISR(INT4_vect)
{
 ShaftCountLeft++;  //increment left shaft position count
}


//Function used for setting motor's direction
void motion_set (unsigned char Direction)
{
 unsigned char PortARestore = 0;

 Direction &= 0x0F;     // removing upper nibbel for the protection
 PortARestore = PORTA;    // reading the PORTA original status
 PortARestore &= 0xF0;    // making lower direction nibbel to 0
 PortARestore |= Direction; // adding lower nibbel for forward command and restoring the PORTA status
 PORTA = PortARestore;    // executing the command
}

void forward (void) //both wheels forward
{
  motion_set(0x06);
}

void back (void) //both wheels backward
{
  motion_set(0x09);
}

void left (void) //Left wheel backward, Right wheel forward
{
  motion_set(0x05);
}

void right (void) //Left wheel forward, Right wheel backward
{
  motion_set(0x0A);
}

void soft_left (void) //Left wheel stationary, Right wheel forward
{
 motion_set(0x04);
}

void soft_right (void) //Left wheel forward, Right wheel is stationary
{
 motion_set(0x02);
}

void soft_left_2 (void) //Left wheel backward, right wheel stationary
{
 motion_set(0x01);
}

void soft_right_2 (void) //Left wheel stationary, Right wheel backward
{
 motion_set(0x08);
}

void stop (void)
{
  motion_set(0x00);
}


//Function used for turning robot by specified degrees
void angle_rotate(unsigned int Degrees)
{
 float ReqdShaftCount = 0;
 unsigned long int ReqdShaftCountInt = 0;

 ReqdShaftCount = (float) Degrees/ 4.090; // division by resolution to get shaft count
 ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
 ShaftCountRight = 0; 
 ShaftCountLeft = 0; 

 while (1)
 {
  if((ShaftCountRight >= ReqdShaftCountInt) | (ShaftCountLeft >= ReqdShaftCountInt))
  break;
 }
 stop(); //Stop robot
}

//Function used for moving robot forward by specified distance

void linear_distance_mm(unsigned int DistanceInMM)
{
 float ReqdShaftCount = 0;
 unsigned long int ReqdShaftCountInt = 0;

 ReqdShaftCount = DistanceInMM / 5.338; // division by resolution to get shaft count
 ReqdShaftCountInt = (unsigned long int) ReqdShaftCount;
  
 ShaftCountRight = 0;
 while(1)
 {
  if(ShaftCountRight > ReqdShaftCountInt)
  {
    break;
  }
 } 
 stop(); //Stop robot
}

void forward_mm(unsigned int DistanceInMM)
{
 forward();
 linear_distance_mm(DistanceInMM);
}

void back_mm(unsigned int DistanceInMM)
{
 back();
 linear_distance_mm(DistanceInMM);
}

void left_degrees(unsigned int Degrees) 
{
// 88 pulses for 360 degrees rotation 4.090 degrees per count
 left(); //Turn left
 angle_rotate(Degrees);
}



void right_degrees(unsigned int Degrees)
{
// 88 pulses for 360 degrees rotation 4.090 degrees per count
 right(); //Turn right
 angle_rotate(Degrees);
}


void soft_left_degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 soft_left(); //Turn soft left
 Degrees=Degrees*2;
 angle_rotate(Degrees);
}

void soft_right_degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 soft_right();  //Turn soft right
 Degrees=Degrees*2;
 angle_rotate(Degrees);
}

void soft_left_2_degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 soft_left_2(); //Turn reverse soft left
 Degrees=Degrees*2;
 angle_rotate(Degrees);
}

void soft_right_2_degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 soft_right_2();  //Turn reverse soft right
 Degrees=Degrees*2;
 angle_rotate(Degrees);
}

void init_devices(void)
{
  cli(); //Clears the global interrupt
  port_init();  //Initializes all the ports
  color_sensor_pin_interrupt_init();
  timer1_init();
  left_position_encoder_interrupt_init();
  right_position_encoder_interrupt_init();
  sei();   // Enables the global interrupt
}

//Function for velocity control
void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}


//Filter Selection
void filter_red(void)    //Used to select red filter
{
  //Filter Select - red filter
  PORTD = PORTD & 0xBF; //set S2 low
  PORTD = PORTD & 0x7F; //set S3 low
}

void filter_green(void) //Used to select green filter
{
  //Filter Select - green filter
  PORTD = PORTD | 0x40; //set S2 High
  PORTD = PORTD | 0x80; //set S3 High
}

void filter_blue(void)  //Used to select blue filter
{
  //Filter Select - blue filter
  PORTD = PORTD & 0xBF; //set S2 low
  PORTD = PORTD | 0x80; //set S3 High
}

void filter_clear(void) //select no filter
{
  //Filter Select - no filter
  PORTD = PORTD | 0x40; //set S2 High
  PORTD = PORTD & 0x7F; //set S3 Low
}

//Color Sensing Scaling
void color_sensor_scaling()   //This function is used to select the scaled down version of the original frequency of the output generated by the color sensor, generally 20% scaling is preferable, though you can change the values as per your application by referring datasheet
{
  //Output Scaling 20% from datasheet
  //PORTD = PORTD & 0xEF;
  PORTD = PORTD | 0x10; //set S0 high
  //PORTD = PORTD & 0xDF; //set S1 low
  PORTD = PORTD | 0x20; //set S1 high
}

void red_read(void) // function to select red filter and display the count generated by the sensor on LCD. The count will be more if the color is red. The count will be very less if its blue or green.
{
  //Red
  filter_red(); //select red filter
  pulse=0; //reset the count to 0
  _delay_ms(100); //capture the pulses for 100 ms or 0.1 second
  red = pulse;  //store the count in variable called red
  
//  lcd_cursor(1,1);  //set the cursor on row 1, column 1
//  lcd_string("Red Pulses"); // Display "Red Pulses" on LCD
//  lcd_print(2,1,red,5);  //Print the count on second row
//  _delay_ms(1000);  // Display for 1000ms or 1 second
//  lcd_wr_command(0x01); //Clear the LCD
}

void green_read(void) // function to select green filter and display the count generated by the sensor on LCD. The count will be more if the color is green. The count will be very less if its blue or red.
{
  //Green
  filter_green(); //select green filter
  pulse=0; //reset the count to 0
  _delay_ms(100); //capture the pulses for 100 ms or 0.1 second
  green = pulse;  //store the count in variable called green
  
//  lcd_cursor(1,1);  //set the cursor on row 1, column 1
//  lcd_string("Green Pulses"); // Display "Green Pulses" on LCD
//  lcd_print(2,1,green,5);  //Print the count on second row
//  _delay_ms(1000);  // Display for 1000ms or 1 second
//  lcd_wr_command(0x01); //Clear the LCD
}

void blue_read(void) // function to select blue filter and display the count generated by the sensor on LCD. The count will be more if the color is blue. The count will be very less if its red or green.
{
  //Blue
  filter_blue(); //select blue filter
  pulse=0; //reset the count to 0
  _delay_ms(100); //capture the pulses for 100 ms or 0.1 second
  blue = pulse;  //store the count in variable called blue
  
//  lcd_cursor(1,1);  //set the cursor on row 1, column 1
//  lcd_string("Blue Pulses"); // Display "Blue Pulses" on LCD
//  lcd_print(2,1,blue,5);  //Print the count on second row
//  _delay_ms(1000);  // Display for 1000ms or 1 second
//  lcd_wr_command(0x01); //Clear the LCD
}


//Function to rotate Servo 1 by a specified angle in the multiples of 1.86 degrees
void servo_1(unsigned char degrees)  
{
 float PositionPanServo = 0;
  PositionPanServo = ((float)degrees / 1.86) + 35.0;
 OCR1AH = 0x00;
 OCR1AL = (unsigned char) PositionPanServo;
}


//Function to rotate Servo 2 by a specified angle in the multiples of 1.86 degrees
void servo_2(unsigned char degrees)
{
 float PositionTiltServo = 0;
 PositionTiltServo = ((float)degrees / 1.86) + 35.0;
 OCR1BH = 0x00;
 OCR1BL = (unsigned char) PositionTiltServo;
}

//Function to rotate Servo 3 by a specified angle in the multiples of 1.86 degrees
void servo_3(unsigned char degrees)
{
 float PositionServo = 0;
 PositionServo = ((float)degrees / 1.86) + 35.0;
 OCR1CH = 0x00;
 OCR1CL = (unsigned char) PositionServo;
}

//servo_free functions unlocks the servo motors from the any angle 
//and make them free by giving 100% duty cycle at the PWM. This function can be used to 
//reduce the power consumption of the motor if it is holding load against the gravity.

void servo_1_free (void) //makes servo 1 free rotating
{
 OCR1AH = 0x03; 
 OCR1AL = 0xFF; //Servo 1 off
}

void servo_2_free (void) //makes servo 2 free rotating
{
 OCR1BH = 0x03;
 OCR1BL = 0xFF; //Servo 2 off
}

void servo_3_free (void) //makes servo 3 free rotating
{
 OCR1CH = 0x03;
 OCR1CL = 0xFF; //Servo 3 off
} 



Color check_color(void)
{
  
    _delay_ms(1000);  

    red_read();   //display the pulse count when red filter is selected
    
    green_read();   //display the pulse count when green filter is selected
    
    blue_read();  //display the pulse count when blue filter is selected
     
     
     
     if((red>green) && (red>blue) && (red>2500))
     {
        
      lcd_cursor(1,1);      //set the cursor on row 1, column 1
      lcd_string("Red"); 
      _delay_ms(1000);    // Display for 1000ms or 1 second
      lcd_wr_command(0x01);   //Clear the LCD
      return Red;
     
     }
     
     if((blue>green) && (blue>red) && (blue>3000))
     {
        
      lcd_cursor(1,1);      //set the cursor on row 1, column 1
      lcd_string("Blue"); 
      _delay_ms(1000);    // Display for 1000ms or 1 second
      lcd_wr_command(0x01);   //Clear the LCD
        return Blue;
     
     }
     if((green>red) && (green>blue) && (green>2800))
     {
        lcd_cursor(1,1);      //set the cursor on row 1, column 1
      lcd_string("Green"); 
      _delay_ms(1000);    // Display for 1000ms or 1 second
      lcd_wr_command(0x01);   //Clear the LCD
        return Green;

     }
    if((green<2000 && green>900) && (blue<2000 && blue>1000) && (red<2000 && red>1000))
     {
        lcd_cursor(1,1);      //set the cursor on row 1, column 1
      lcd_string("Black"); 
      _delay_ms(1000);    // Display for 1000ms or 1 second
      lcd_wr_command(0x01);   //Clear the LCD
        return Key;
      
    }
     
      
}

void pick_box()
{
  servo_2(5);         //open the gripper
  _delay_ms(1000);
  servo_1(60);        //lower the arm
  _delay_ms(1000);
 
  servo_2(50);        //grip the box 
  _delay_ms(1000);
  servo_1(0);         //raise the arm
  _delay_ms(1000);

}
void keep_box()
{
    //as the arm would be in air 
    //bring it down (set the box a little up from the surface)
    //release the gripper only at a lesser angle first and then increase the angle later)
    //bring the gripper-arm back at original position
    //free the servos
}


void _forward()
{
	while(true)
	{
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		flag=0;

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		
		

		if(Center_white_line>0x0c)
		{
			flag=1;
			forward();
			velocity(150,150);
		}

		if((Left_white_line<0x0c) && (flag==0))
		{
			flag=1;
			forward();
			velocity(130,50);
		}

		if((Right_white_line<0x0c) && (flag==0))
		{
			flag=1;
			forward();
			velocity(50,130);
		}
	
		if(Center_white_line<0x0c && Left_white_line<0x0c && Right_white_line<0x0c)
		{
			
			velocity(130,50);
			_delay_ms(20);

			velocity(50,130);
			_delay_ms(100);

		}
		if(Center_white_line<0x0c && Left_white_line<0x0c && Right_white_line>0x0c){
		
			velocity(130,50);

		}
		if(Center_white_line>0x0c && Left_white_line>0x0c && Right_white_line>0x0c)
		{	forward();
			velocity(0,0);
			count=count+1;
			lcd_string(" ");
			lcd_string(count);
			
		}
	}
}

void turn_left()
{
	while(true)
	{
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		

		soft_left();
		velocity(0, 30);
		if (Left_white_line>0x0c)
			return;
		else
			continue;
	}
}

void turn_right()
{
	while(true)
	{
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		

		soft_right_2();
		velocity(0, 30);
		if (Center_white_line>0x0c)
			break;
		else
			continue;


	}

	stop();
	_delay_ms(20);

	while(true)
	{
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2		
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		

		soft_right();
		velocity(30, 0);
		if (Right_white_line>0x0c)
			return;
		else
			continue;


	}
}
 /*
 *
 * Function Name: indicator_blocks
 * Input: None
 * Output: void
 * Logic: 
 * 
 *		Function for implementing detection of indicator blocks and their respective colors.
 *		
 * Example Call: 
 * 
 *		// Preceding code.
 *		indicator_blocks();
 *		// Following code.
 * 
 */
 
void indicator_blocks() 
{
	// TODO

	for (int i=0; i<NO_IND ;i++)
	{
		_forward();
		stop();
		turn_left();
		stop();
		
		iBlocks[i] = check_color();
		i++;
		// Turn till middle white line sensor gets black.
		turn_right();
		stop();
		// Move Forward.
		// If proximity<20
		// stop
		iBlocks[i] = check_color();
		
		// Turn till middle white line sensor gets black.
		// Move Forward.
		soft_left_2();
		velocity(30, 0);
		while(true)
		{
			if (ADC_Conversion(2)>0x0c)
			{
				stop();
				break;
			}
		}
	}
}

int main(void)
{
    init_devices();
    lcd_set_4bit();
    lcd_init();
    color_sensor_scaling();
	velocity(100,100);
	_delay_ms(10000);
	indicator_blocks();
}
