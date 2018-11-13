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
void initDisplay()
{
	/* LCD Module init */
	LCD_init();
	LCD_clear(White);
	LCD_setTextColor(Blue);
}

void setupHW(void)
{
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

void potentioMeterTask(void *params);

xSemaphoreHandle interruptSignal;

void EXTI15_10_IRQHandler(void)
{
	portBASE_TYPE higherPrio;
	printf("Interrupt!\n");
	xSemaphoreGiveFromISR(interruptSignal, &higherPrio);
	EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
	portEND_SWITCHING_ISR(higherPrio);
}

uint16_t max = 0, min = 0, mean = 0;

/**
 * Scheduled interrupt handler
 */
void scheduledInterruptTask(void *params)
{
	printf("Init function started\n");

	printf("Press key to set max\n")
		xSemaphoreTake(interruptSignal, portMAX_DELAY);
	ADC_SoftwareStartConv(ADC3);
	while (!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC))
	{
	}
	max = ADC_GetConversionValue(ADC3);
	printf("Set max as: %d\n", max);

	printf("Press key to set min\n")
		xSemaphoreTake(interruptSignal, portMAX_DELAY);
	ADC_SoftwareStartConv(ADC3);
	while (!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC))
	{
	}
	min = ADC_GetConversionValue(ADC3);
	printf("Set min as: %d\n", min);

	mean = (max + min) / 2;
	xTaskCreate(potentioMeterTask, "Pottask", 100, NULL, 1, NULL);

	for (;;)
	{
		xSemaphoreTake(interruptSignal, portMAX_DELAY);
		printf("External interrupt (PA0, PB0, etc.)!\n");
	}
}

uint16_t potValue;
void potentioMeterTask(void *params)
{
	for (;;)
	{
		ADC_SoftwareStartConv(ADC3);
		while (!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC))
		{
		}
		potValue = ADC_GetConversionValue(ADC3);
		printf("Potvalue: %d\n", potValue);

		if (potValue > mean)
		{
			STM_EVAL_LEDOn(LED1);
			STM_EVAL_LEDOn(LED2);
			STM_EVAL_LEDOn(LED3);
			STM_EVAL_LEDOn(LED4);
		}
		else
		{
			STM_EVAL_LEDOff(LED1);
			STM_EVAL_LEDOff(LED2);
			STM_EVAL_LEDOff(LED3);
			STM_EVAL_LEDOff(LED4);
		}

		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

GPIO_InitTypeDef GPIO_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure1;

GPIO_InitTypeDef GPIO_InitStructure;
ADC_InitTypeDef ADC_InitStructure;

/*-----------------------------------------------------------*/
int main(void)
{
	setupHW();

	/* TODO: initialize your task(s) here */

	vSemaphoreCreateBinary(interruptSignal);
	// empty the semaphore (initially, no interrupt has occurred)
	xSemaphoreTake(interruptSignal, portMAX_DELAY);

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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	ADC_StructInit(&ADC_InitStructure);
	ADC_Init(ADC3, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 1,
							 ADC_SampleTime_480Cycles);

	ADC_Cmd(ADC3, ENABLE);

	printf("\n 1DT106\n\n   Programming Embedded Systems\n");
	/* this is redirected to the display in LCD.c */
	/* See LCD.h if you do not like the font size or to see 
	   other LCD functions */

	vTaskStartScheduler();

	assert(0);
	return 0; // not reachable
}
