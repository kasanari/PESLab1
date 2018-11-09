/**
 * Program skeleton for the course "Programming embedded systems"
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "semphr.h" /*needed for Exercise 1.2 (i.e. deferred int. handling)*/
#include "LCD.h" 
#include "assert.h"

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void initDisplay () {
  /* LCD Module init */
  LCD_init();
  LCD_clear(White);
  LCD_setTextColor(Blue);
}

void setupHW (void){
	  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */     
	
	/*Initialize the LEDs*/
	/* Check out what is happening in these functions by right clicking and
	choosing "Go to definiton of ...".
		Alternatively, you can open file stm322xg_eval.c and find the function
	there.
	*/	
	STM_EVAL_LEDInit(LED1);
	STM_EVAL_LEDInit(LED2);
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
	
  /* SysTick end of count event each 1ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency);
	
	initDisplay(); /* Required if you want to use the LCD */
}

/*-----------------------------------------------------------*/

uint16_t period1 = 100;
uint16_t period2 = 200;
uint16_t period3 = 400;
uint16_t period4 = 500;

uint16_t counter = 0;

static void ledTask(void *params) {
	/* 
		stm322xg_eval.h and stm322xg_eval.c provide wrappers
	for using the evaluation board. See LED related function
	declaration and definitions there to get an idea about 
	what is going on when you call these functions.
	Potentially useful functions for this task:
		void STM_EVAL_LEDOn(Led_TypeDef Led);
		void STM_EVAL_LEDOff(Led_TypeDef Led);
		void STM_EVAL_LEDToggle(Led_TypeDef Led);
		
		You will also need to add delay to the task.
	See task.h for functions such as:
		void vTaskDelay( portTickType xTicksToDelay );
		void vTaskDelayUntil( portTickType * const pxPreviousWakeTime, portTickType xTimeIncrement );
	*/
	

	for (;;) {
		// TODO: write your task code here
		
		/* an example delay of 100ms: 
		 vTaskDelay(100 / portTICK_RATE_MS);
		 (remember that absolute delay will cause drift)
		*/
		
		if (!(counter % 1)) {
			STM_EVAL_LEDToggle(LED1);
		}
		if (!(counter % 2)) {
			STM_EVAL_LEDToggle(LED2);
		}
		if (!(counter % 4)) {
			STM_EVAL_LEDToggle(LED3);
		}
		if (!(counter % 5)) {
			STM_EVAL_LEDToggle(LED4);
		}
		
		vTaskDelay(100/portTICK_RATE_MS);
		counter++;
		if (counter == 20) {
			counter = 0;
			printf("HYPERPERIOD!!!\n"); 
		}
	}
}

/*-----------------------------------------------------------*/
int main (void){
	setupHW();

	/* TODO: initialize your task(s) here */
	/* e.g. xTaskCreate(ledTask, ... ); */
	
	xTaskCreate(ledTask, "ledtask", 100, NULL, 1, NULL);

	printf("\n 1DT106\n\n   Programming Embedded Systems\n"); 
	/* this is redirected to the display in LCD.c */
	/* See LCD.h if you do not like the font size or to see 
	   other LCD functions */
	
	vTaskStartScheduler();
	
	assert(0);
	return 0;                 // not reachable
}
