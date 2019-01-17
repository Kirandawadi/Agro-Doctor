#include "STEPPER.h"
#include <math.h>


float linear_encoder_in_cm = 120 ;
float radius =3;
float wheel_size = 55.6;
float		travel = 0;
float angl=0;
uint16_t pid_error;
uint32_t PID;
uint8_t p_scalar, i_scalar, d_scalar;

extern volatile uint32_t encoder_reading_wheel;
extern volatile uint32_t encoder_reading_left_right;
extern volatile uint8_t direction_wheel;
extern volatile uint8_t direction_left_right;
extern volatile int forward_speed;
extern volatile float velocity;
extern volatile int throttel_left, throttel_right;
extern volatile uint16_t encoder_reading_pre;
extern volatile int total_distance ;


float distance_travelled(uint32_t encoder_reading_wheel)
{
	
		return ((wheel_size*encoder_reading_wheel)/fullcounter);
	
}

float left_right_angle()
{
	if(encoder_reading_left_right >1000)
	{
		return(asin((3000-encoder_reading_left_right)/(radius* linear_encoder_in_cm)) * 57.29577951);
	}
	else
	{
		return -(asin(encoder_reading_left_right/ (radius * linear_encoder_in_cm)) * 57.2957795);
	}
	
}

void move(uint32_t distance, float velocity,int dir)
{
	float dis;
	
	
	if(dir == Front)
	{
		while(distance > encoder_reading_wheel )
		{
	
			HAL_GPIO_WritePin(sig_port,sig1,GPIO_PIN_SET);
			HAL_GPIO_WritePin(sig_port,sig2, GPIO_PIN_RESET);
			htim2.Instance->CCR1 = (int)((20 - forward_speed) * (2800 / 17));
			HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
		}	
	
	}
	
	if(dir == Back)
	{
		while(distance > encoder_reading_wheel)
		{
		
			HAL_GPIO_WritePin(sig_port,sig1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(sig_port,sig2, GPIO_PIN_SET);
			htim2.Instance->CCR1 = (int)((20 - forward_speed) * (2800 / 17));
			HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
				
		}
	
	
	}	
	TIM4->CNT = 0;
	encoder_reading_wheel = 0;
	encoder_reading_pre = 0;
	HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
	HAL_GPIO_WritePin(sig_port,sig1,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(sig_port,sig2, GPIO_PIN_RESET);
			
}

void set_angle(float ang,uint8_t direction)
{
	if(direction == Right)
	{
		angl = left_right_angle();
		while(ang > angl  )
		{
			angl = left_right_angle();
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
			throttel_left =  -70;
			throttel_right = -70;	
		}
		
	}

	else if(direction == Left)
	{	
		angl = left_right_angle();
		while(ang <  angl)
		{
			angl = left_right_angle();
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
			throttel_left = 70;
			throttel_right = 70;
		}
	}
	throttel_left =  0;
	throttel_right = 0;	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
}

//Sets PID for required distance
/*
mode:0 for DC motor
mode:1 for stepper

Wind_up is the value to limit capacity of motor
*/
int pid(int16_t set_distance,uint16_t wind_up,uint8_t mode)
{
	 pid_error = set_distance - total_distance;
	 if(PID >10 || PID < -10)
		 pid_error += PID * 0.015 ;

	 float proportional = pid_error * p_scalar;//Proportional 
	
	 static float integral = 0;
	 integral += pid_error * i_scalar; // Intregal 
	 if(integral >  1000) integral = 1000; // limit wind-up
	 if(integral < -1000) integral =-1000;

	 static float previous_error = 0;
	
	 float derivative = (pid_error - previous_error) * d_scalar;// Derivative
	 previous_error = pid_error;
	 PID = proportional+derivative+integral; // Required PID
	 if(PID > 1000) PID = 1000;
	 if(PID <-1000)PID= -1000;
	 
	 if(PID <5 && PID>-5) PID =0;//Create a dead-band to stop the motors when the robot is balanced
	
		if(mode == 0)
			return PID;
		
}
