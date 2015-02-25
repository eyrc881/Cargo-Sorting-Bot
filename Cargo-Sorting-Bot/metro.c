int forward_move()
{
	init_devices();
	lcd_set_4bit();
	lcd_init();
	
	while(1)
	{

		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor

	
		flag=0;
		int bcount=0;
		char bprint;
		print_sensor(1,1,3);	//Prints value of White Line Sensor1
		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
		print_sensor(1,9,1);	//Prints Value of White Line Sensor3

		//010
		if(Left_white_line<0x64 && Center_white_line>0x64 &&  Right_white_line<0x64)
		{	
			forward();
			velocity(130,130);
		}
		//000
		if(Left_white_line<0x64 && Center_white_line<0x64 && Right_white_line>0x64)
		{	
			velocity(150,50);
			_delay_ms(20);

			velocity(50,150);
			_delay_ms(30);

		}
		//001
		if(Left_white_line<0x64 && Center_white_line<0x64 &&  Right_white_line>0x64)
		{
			
			velocity(130,30);
			_delay_ms(20);
		}
		//011
		if(Left_white_line<0x64 && Center_white_line>0x64 && Right_white_line>0x64)
		{	
			velocity(130,30);
			_delay_ms(10);

			/*back();
			velocity(130,130);
			_delay_ms(100);
			*/
		}
		//100
		if(Left_white_line>0x64 && Center_white_line<0x64 &&  Right_white_line<0x64)
		{
			velocity(30,130);
			_delay_ms(20);
		}
		//110
		if(Left_white_line>0x64 && Center_white_line>0x64 &&  Right_white_line<0x64)
		{
			velocity(30,130);
			_delay_ms(10);

			/*back();
			velocity(130,130);
			_delay_ms(100);
			*/
		}
		//111
		if(Left_white_line>0x64 && Center_white_line>0x64 &&  Right_white_line>0x64)
		{	
			
			velocity(0,0);
			bcount=bcount+1;
			bprint=bcount;
			lcd_string(" ");
			lcd_string(bprint);
		}
	}
}

unsigned char left_turn()
			{
				if(Center_white_line>0x64 && Left_white_line>0x64 && Right_white_line>0x64)
					{
						velocity(30,150);
					if(Center_white_line>0x64 && Left_white_line<0x64 && Right_white_line<0x64)
						{
							stop();
						}
					else
						{
							continue;
						}
						return '0';
					}

unsigned char right_turn()
			{
				if(Center_white_line>0x64 && Left_white_line>0x64 && Right_white_line>0x64)
					{
						velocity(150,30);
					if(Center_white_line>0x64 && Left_white_line<0x64 && Right_white_line<0x64)
						{
							stop();
						}
					else
						{
							continue;
						}
						return '1';

	for (int i = 0; i < 3; i++)
	{
		if (dir[i] == '0'&& (i==0||i==1))
		{
			left_turn();
					}
			}

void nav(int dir[i])
{
			forward_move();
		}
		else
		{
			if (dir[i] == '0'&&(i==2))
			{
				left_turn();
			}
		}

		if(dir[i] == '1'&& (i==0||i==1))
		{
			right_turn();
			forward_move();
		}
		else
		{
			if (dir[i] == '1'&&(i==2))
			{
				right_turn();
			}
		}
	}
}

toner[i]={0,0,0};
tonel[i]={0,0,1};
ttwor[i]={0,1,1};
ttwol[i]={0,1,0};
tthreer[i]={1,1,1};
tthreel[i]={1,1,0};
tfourr[i]={1,0,1};
tfourl[i]={1,0,0};

int int main(int argc, char const *argv[])
{
	
	indicatot

	for (int i = 0; i < 8; ++i)
	{
		nav(arr[i]);
	}
	return 0;
}




