// eyrc881


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <math.h> //included to support power function
#include "lcd.c"
//#include "col.c"

void port_init();
void timer5_init();
void velocity(unsigned char, unsigned char);
void motors_delay();

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char Left_white_line = 0;
unsigned char Center_white_line = 0;
unsigned char Right_white_line = 0;


//Function to configure LCD port
void lcd_port_config (void)
{
 DDRC = DDRC | 0xF7; //all the LCD pin's direction set as output
 PORTC = PORTC & 0x80; // all the LCD pins are set to logic 0 except PORTC 7
}

//ADC pin configuration
void adc_pin_config (void)
{
 DDRF = 0x00; 
 PORTF = 0x00;
 DDRK = 0x00;
 PORTK = 0x00;
}

//Function to configure ports to enable robot's motion
void motion_pin_config (void) 
{
 DDRA = DDRA | 0x0F;
 PORTA = PORTA & 0xF0;
 DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
 PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

//Function to initialize Buzzer 
void buzzer_pin_config (void)
{
 DDRC = DDRC | 0x08;		//Setting PORTC 3 as output
 PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
}


//Function to Initialize PORTS
void port_init()
{
	lcd_port_config();
	adc_pin_config();
	motion_pin_config();
	buzzer_pin_config();	
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

// Timer 5 initialized in PWM mode for velocity control
// Prescale:256
// PWM 8bit fast, TOP=0x00FF
// Timer Frequency:225.000Hz
void timer5_init()
{
	TCCR5B = 0x00;	//Stop
	TCNT5H = 0xFF;	//Counter higher 8-bit value to which OCR5xH value is compared with
	TCNT5L = 0x01;	//Counter lower 8-bit value to which OCR5xH value is compared with
	OCR5AH = 0x00;	//Output compare register high value for Left Motor
	OCR5AL = 0xFF;	//Output compare register low value for Left Motor
	OCR5BH = 0x00;	//Output compare register high value for Right Motor
	OCR5BL = 0xFF;	//Output compare register low value for Right Motor
	OCR5CH = 0x00;	//Output compare register high value for Motor C1
	OCR5CL = 0xFF;	//Output compare register low value for Motor C1
	TCCR5A = 0xA9;	/*{COM5A1=1, COM5A0=0; COM5B1=1, COM5B0=0; COM5C1=1 COM5C0=0}
 					  For Overriding normal port functionality to OCRnA outputs.
				  	  {WGM51=0, WGM50=1} Along With WGM52 in TCCR5B for Selecting FAST PWM 8-bit Mode*/
	
	TCCR5B = 0x0B;	//WGM12=1; CS12=0, CS11=1, CS10=1 (Prescaler=64)
}

void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;		//MUX5 = 0
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
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

//Function for velocity control
void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}

//Function used for setting motor's direction
void motion_set (unsigned char Direction)
{
 unsigned char PortARestore = 0;

 Direction &= 0x0F; 		// removing upper nibbel for the protection
 PortARestore = PORTA; 		// reading the PORTA original status
 PortARestore &= 0xF0; 		// making lower direction nibbel to 0
 PortARestore |= Direction; // adding lower nibbel for forward command and restoring the PORTA status
 PORTA = PortARestore; 		// executing the command
}

void forward (void) 
{
  motion_set (0x06);
}

void stop (void)
{
  motion_set (0x00);
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


void init_devices (void)
{
 	cli(); //Clears the global interrupts
	port_init();
	adc_init();
	timer5_init();
	sei();   //Enables the global interrupts
}

//Main Function
int blackline()
{	

	int foo;
	int temp;
	
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		flag=0;

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		
		
		//010
		if(Left_white_line<0x0c && Center_white_line>0x0c &&  Right_white_line<0x0c)
		{	
			forward();
			velocity(130,130);
		}
		//000
		if(Left_white_line<0x0c && Center_white_line<0x0c && Right_white_line>0x0c)
		{	
			velocity(130,50);
			_delay_ms(20);

			velocity(50,130);
			_delay_ms(30);

		}
		//001
		if(Left_white_line<0x0c && Center_white_line<0x0c &&  Right_white_line>0x0c)
		{
			
			velocity(130,30);
			_delay_ms(10);
		}
		//011
		if(Left_white_line<0x0c && Center_white_line>0x0c && Right_white_line>0x0c)
		{	
			velocity(130,30);
			_delay_ms(10);

			/*back();
			velocity(130,130);
			_delay_ms(100);
			*/
		}
		//100
		if(Left_white_line>0x0c && Center_white_line<0x0c &&  Right_white_line<0x0c)
		{
			velocity(30,130);
			_delay_ms(20);
		}
		//110
		if(Left_white_line>0x0c && Center_white_line>0x0c &&  Right_white_line<0x0c)
		{
			velocity(30,130);
			_delay_ms(10);


			/*back();
			velocity(130,130);
			_delay_ms(100);
			*/
		}
		//111
		if(Left_white_line>0x0c && Center_white_line>0x0c &&  Right_white_line>0x0c)
		{	
			foo=temp; 
			temp=foo+1;
			lcd_print(2, 8, temp, 3);
			lcd_print(2, 12, foo, 3);			 
			velocity(130,130);
			_delay_ms(450);
			velocity(0,0);
			return;
		}		//if(temp<foo)
			

	}
}

void turn_left()
{
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		flag=0;

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		
		velocity(0,130);
		_delay_ms(10);
		if(Left_white_line>0x0c)
		{
			velocity(0,0);

			return;
		}
		
	}
}

void center_left()
{
	soft_right_2();
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		flag=0;

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		
		velocity(0,130);
		_delay_ms(10);
		if(Center_white_line>0x0c)
		{
			velocity(0,0);

			return;
		}
		
	}
}	

void turn_right()
{
	soft_right();
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		flag=0;

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		
		velocity(130,0);
		_delay_ms(10);
		if(Right_white_line>0x0c)
		{
			velocity(0,0);

			return;
		}
		
	}
}

void center_right()
{
	soft_left_2();
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

		flag=0;

		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
		
		velocity(130,0);
		_delay_ms(10);
		if(Center_white_line>0x0c)
		{
			velocity(0,0);

			return;
		}
		
	}
}	
int main()
{

	
	init_devices();
	lcd_set_4bit();
	lcd_init();

	// Indicator Blocks.
	blackline();
	turn_left();
	center_left();
	turn_right();
	center_right();
	blackline();
	turn_left();
	center_left();
	turn_right();
	center_right();

	// Traversing

	// Center node

	forward();
	velocity(130,130);
	blackline();

	// Part-1

	turn_left();
	blackline();

	// T1

	turn_right();
	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	right();
	velocity(130,130);
	_delay_ms(4800);
	forward();
	turn_right();
	blackline();

	// T3

	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	right();
	velocity(130,130);
	_delay_ms(4800);
	forward();
	turn_right();
	blackline();
	turn_right();
	blackline();

	// Center node

	blackline();

	// Part 2

	// T2

	turn_left();
	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	right();
	velocity(130,130);
	_delay_ms(4800);
	forward();
	turn_right();
	blackline();
	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	right();
	velocity(130,130);
	_delay_ms(4800);
	forward();
	velocity(0,0);

	// till here traversing of terminal obne finshed
	// write three more similar to finish  whole traversing, finish it at terminal 4 left


	// T4-R to T4-L

	right();
	velocity(130,130);
	_delay_ms(4800);
	//forward()
	//pick

	// T4-R to T1-R

	turn_right();
	blackline();
	turn_left();
	blackline();
	blackline();
	turn_right();
	blackline();
	right();
	velocity(130,130);
	_delay_ms(2300);
	stop();
	velocity(0,0);

	//keep

	// T1-R to T3-L
	forward();
	velocity(130,130);
	turn_right();
	


	blackline();
	blackline();
	right();
	velocity(130,130);
	_delay_ms(2300);
	//pick

	// T3-L to T4-L
	turn_right();

	blackline();
	turn_right();
	blackline();
	blackline();
	turn_right();
	blackline();
	right();
	velocity(130,130);
	_delay_ms(2300);
	// drop
	velocity(0,0);



	// T4 - L to T4-R

	right();
	velocity(130,130);
	_delay_ms(4800);
	forward();
	// pick

	// T4-R to T3-L
	turn_left();
	blackline();
	turn_left();
	blackline();
	blackline();
	turn_left();
	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	velocity(0,0);
	// drop

	// T3-L to T3-R

	right();
	velocity(130,130);
	_delay_ms(4800);
	// pick

	// T3-R to T4-R

	forward();
	turn_left();
	blackline();
	turn_right();
	blackline();
	blackline();
	turn_right();
	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	velocity(0,0);
	// drop


	// T4-R to T2-R

	forward();
	turn_left();
	blackline();
	blackline();
	right();
	velocity(130,130);
	_delay_ms(2300);
	velocity(0,0);
	// pick

	// T2-R to T3-R

	forward();
	turn_right();
	blackline();
	turn_right();
	blackline();
	blackline();
	turn_left();
	blackline();
	left();
	velocity(130,130);
	_delay_ms(2300);
	forward();
	velocity(0,0);
	

	// Tada

	buzzer_on();
	_delay_ms(5000);
	buzzer_off();

}
