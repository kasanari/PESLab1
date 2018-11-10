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

uint8_t toggle = 0;

xSemaphoreHandle interruptSignal;

void EXTI15_10_IRQHandler(void){
portBASE_TYPE higherPrio;
printf("Interrupt!\n");
xSemaphoreGiveFromISR(interruptSignal, &higherPrio);
EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
portEND_SWITCHING_ISR(higherPrio);
}


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
		
		if (toggle) {
				STM_EVAL_LEDToggle(LED1);
				STM_EVAL_LEDToggle(LED4);
		} else {
				STM_EVAL_LEDToggle(LED2);
				STM_EVAL_LEDToggle(LED3);
		}
	
		vTaskDelay(100/portTICK_RATE_MS);
		counter++;
		if (counter == 20) {
			counter = 0;
			printf("HYPERPERIOD!!!\n"); 
		}
	}
}

/**
 * Scheduled interrupt handler
 */
void scheduledInterruptTask (void* params) {
  for (;;) {
    xSemaphoreTake(interruptSignal, portMAX_DELAY);

		if (!toggle) {
			STM_EVAL_LEDOff(LED1);
			STM_EVAL_LEDOff(LED4);
		} else {
			STM_EVAL_LEDOff(LED2);
			STM_EVAL_LEDOff(LED3);
		}
	toggle = !toggle;
	printf("External interrupt (PA0, PB0, etc.)!\n");
  }
}

GPIO_InitTypeDef GPIO_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure1;

/*-----------------------------------------------------------*/
int main (void){
	setupHW();

	/* TODO: initialize your task(s) here */
	/* e.g. xTaskCreate(ledTask, ... ); */
	
	vSemaphoreCreateBinary(interruptSignal);
  // empty the semaphore (initially, no interrupt has occurred)
  xSemaphoreTake(interruptSignal, portMAX_DELAY);
	
	xTaskCreate(ledTask, "ledtask", 100, NULL, 1, NULL);
	xTaskCreate(scheduledInterruptTask, "scheduled interrupt task", 100, NULL, 1, NULL);
	
	/* Enable the BUTTON Clock */
	RCC_AHB1PeriphClockCmd(KEY_BUTTON_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	/* Configure Button pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = KEY_BUTTON_PIN;
	GPIO_Init(KEY_BUTTON_GPIO_PORT, &GPIO_InitStructure);
	
	/* Connect Button EXTI Line to Button GPIO Pin */
	SYSCFG_EXTILineConfig(KEY_BUTTON_EXTI_PORT_SOURCE,
	KEY_BUTTON_EXTI_PIN_SOURCE);
	
	/* Configure Button EXTI line */
	EXTI_InitStructure.EXTI_Line = KEY_BUTTON_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	
	//detect rising edge (release)
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set Button EXTI Interrupt
	to the lowest priority */
	NVIC_InitStructure1.NVIC_IRQChannel = KEY_BUTTON_EXTI_IRQn;
	NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure1.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure1);

	printf("\n 1DT106\n\n   Programming Embedded Systems\n"); 
	/* this is redirected to the display in LCD.c */
	/* See LCD.h if you do not like the font size or to see 
	   other LCD functions */
	
	vTaskStartScheduler();
	
	assert(0);
	return 0;                 // not reachable
}
