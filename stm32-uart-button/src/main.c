/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include <stdbool.h>
#include "stm32f0xx.h"
#include "stm32f0xx_nucleo.h"

char global_hello[] = "Hello World!\r\n";
int current;	// The character in the message we are sending

#define BUFFER_SIZE 8

struct circularBuffer {
	uint32_t putIndex;		// Where we will put the next character
	uint32_t getIndex;		// Where to get the next character
	uint32_t nCharacters;	// Number of characters in the buffer
	char data[BUFFER_SIZE];	// The data in the buffer
};

UART_HandleTypeDef global_uartHandle; // UART initialization


// A simple, classic circular buffer for USART2
volatile struct circularBuffer global_buffer = {0, 0, 0, {'\0'}};	// The string we are sending
volatile struct circularBuffer global_bufferRecv = {0, 0, 0, {'\0'}}; // The string we are receiving

// debug code
struct logEvent {
	uint32_t line;
	uint32_t putIndex;
	uint32_t getIndex;
	uint32_t nCharacters;
};

#define N_EVENTS 100							// Store 100 events.
uint32_t global_nextEvent = 0;					// Where to put the next event
struct logEvent global_debugEvents[N_EVENTS];   // The log data

int global_click = 0;	// for button interrupt to change
char global_bye[] = "Good bye!\r\n";
#define MAX_MSG 6 //NOTE(Mark): must equal number of string below
char *global_msg[] = {
		"Hello World!\r\n",
		"How are you?\r\n",
		"What are you doing?\r\n",
		"What is the weather like to day?\r\n",
		"It is nice to talk to you.\r\n",
		"Good bye!\r\n"
};

void myPuts(const char *str);

void debugEvent(const uint32_t line)
{
	global_debugEvents[global_nextEvent].line = line;
	global_debugEvents[global_nextEvent].putIndex = global_buffer.putIndex;
	global_debugEvents[global_nextEvent].getIndex = global_buffer.getIndex;
	global_debugEvents[global_nextEvent].nCharacters = global_buffer.nCharacters;
	++global_nextEvent;
	if (global_nextEvent == N_EVENTS)
	{
		global_nextEvent = 0;
	}
}

void Error_Handler(void)
{
	/* Turn LED2 on */
	HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);

	while(true)
	{
		// Toggle the state of LED2.
		HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
		HAL_Delay(1000);		// Wait one second.
	}
}

/*
 *  Handle the USART2 interrupt.
 *
 *  Magically called by the chip's interrupt system.
 *  Name is fixed because of the startup code that
 *  populates the interrupt vector.
 */
void USART2_IRQHandler(void)
{
	if ((global_uartHandle.Instance->ISR & USART_ISR_RXNE) != 0)
	{
			if (global_bufferRecv.nCharacters == BUFFER_SIZE) //buffer is full. should we write over the oldest data for stop and wait with a chance of losing the newest data?
			{
				global_uartHandle.Instance->CR1 &= ~(USART_CR1_RXNEIE);
				return;
			}
			global_bufferRecv.data[global_bufferRecv.putIndex] = (int)global_uartHandle.Instance->RDR;
			global_bufferRecv.putIndex++;
			if (global_bufferRecv.putIndex == BUFFER_SIZE)
			{
				global_bufferRecv.putIndex = 0;
			}

			global_bufferRecv.nCharacters++;

			if (global_bufferRecv.nCharacters == BUFFER_SIZE)
			{
				global_uartHandle.Instance->CR1 &= ~(USART_CR1_RXNEIE);
			}
			return;
	}
	if ((global_uartHandle.Instance->ISR & USART_ISR_TXE) != 0) { // ?
		// This should never happen, but we don't want to crash if it does.
		if (global_buffer.nCharacters == 0) { //NOTE(Mark): this happen now because interrupt is enable when look to receive and of course the send buffer will be empty
			// Turn of interrupt.
			global_uartHandle.Instance->CR1 &= ~(USART_CR1_TXEIE);
			return;
		}
		// Send to UART.
		global_uartHandle.Instance->TDR = global_buffer.data[global_buffer.getIndex];
		global_buffer.getIndex++;
		if (global_buffer.getIndex == BUFFER_SIZE)
		{
			global_buffer.getIndex = 0;
		}

		debugEvent(__LINE__);
		--global_buffer.nCharacters;
		debugEvent(__LINE__);

		if (global_buffer.nCharacters == 0)
		{
			// Turn of interrupt.
			global_uartHandle.Instance->CR1 &= ~(USART_CR1_TXEIE);
		}
		return;
	}

	// Since the only interrupt we enabled was TXE, we should never
	// get here. When we do enable other interrupts, we need to put
	// code to handle them here.
}


void EXTI4_15_IRQHandler(void)
{
	//TODO(Mark): not sure how to tell which exti cause the interrupt
	if (EXTI->PR | EXTI_PR_PR13_Msk)
	{
		global_click++;
		if (global_click == MAX_MSG) global_click = 0;
	}
	EXTI->PR |= EXTI_PR_PR13_Msk; // clear the PR register
}
//void EXTI4_15_IRQHandler(void)
//{
// EXTI4_15_IRQn
//	typedef struct
//	{
//	  __IO uint32_t IMR;          /*!<EXTI Interrupt mask register,                 Address offset: 0x00 */
//	  __IO uint32_t EMR;          /*!<EXTI Event mask register,                     Address offset: 0x04 */
//	  __IO uint32_t RTSR;         /*!<EXTI Rising trigger selection register ,      Address offset: 0x08 */
//	  __IO uint32_t FTSR;         /*!<EXTI Falling trigger selection register,      Address offset: 0x0C */
//	  __IO uint32_t SWIER;        /*!<EXTI Software interrupt event register,       Address offset: 0x10 */
//	  __IO uint32_t PR;           /*!<EXTI Pending register,                        Address offset: 0x14 */
//	} EXTI_TypeDef;
//
//#define EXTI_IMR_MR13             EXTI_IMR_MR13_Msk                            /*!< Interrupt Mask on line 13 */
//
//}

/*
 *  Put a character in the serial buffer
 *
 *  @param ch The character to send
 */
void myPutchar(const char ch)
{
	if ((global_buffer.nCharacters == 0) && (global_buffer.getIndex != global_buffer.putIndex))
	{
		Error_Handler();
	}
	// Wait until there is room.
	while(global_buffer.nCharacters == BUFFER_SIZE)
	{
		continue;
	}
	global_buffer.data[global_buffer.putIndex] = ch;
	++global_buffer.putIndex;
	if (global_buffer.putIndex == BUFFER_SIZE)
	{
		global_buffer.putIndex = 0;
	}

	debugEvent(__LINE__);
	__disable_irq();
	++global_buffer.nCharacters;
	__enable_irq();
	debugEvent(__LINE__);
	// Enable the interrupt (or reenable it. this cause no harm).
	global_uartHandle.Instance->CR1 |= USART_CR1_TXEIE;
}

void myGetChar(char *recvBuffer, int *loc)
{
	while(global_bufferRecv.nCharacters == 0)
	{
		continue;
	}
	recvBuffer[*loc] = global_bufferRecv.data[global_bufferRecv.getIndex];
	global_bufferRecv.getIndex++;
	*loc += 1;
	if (global_bufferRecv.getIndex == BUFFER_SIZE)
	{
		global_bufferRecv.getIndex = 0;
	}

	__disable_irq();
	--global_bufferRecv.nCharacters;
	__enable_irq();


}

#define RECV_SIZE 1000
#define ENTER 0x0D
void myResponse()
{
	char recvBuffer[RECV_SIZE] = {"received: "};
	int loc = 10;
	int locPrev = loc -1;

	//global_uartHandle.Instance->CR1 &= ~(USART_CR1_TXEIE);
	global_uartHandle.Instance->CR1 |= USART_CR1_RXNEIE;

	// recv data
	while(recvBuffer[locPrev] != ENTER) //TODO(Mark): check for enter
	{
		myGetChar(recvBuffer, &loc);
		locPrev = loc - 1; // need this so when the buffer loops you do not use a negative number for array access
		if (loc == RECV_SIZE)
		{
			recvBuffer[loc -1] = '\0'; // NULL terminate the need of the buffer because myPuts can't handle circular buffers
			loc = 0; // wrap buffer. just going to lose data and data will be out of order
		}
	}

	recvBuffer[loc] = '\n';
	if (loc+1 < RECV_SIZE)
	{
		recvBuffer[loc] = '\n';
		recvBuffer[loc+1] = '\0';
	}
	else
	{
		//TODO(Mark): bug figure out how i want to handle this
		recvBuffer[loc+1] = '\0';
	}
	// send data
	myPuts(recvBuffer);
}

/**
 * Our version of puts
 *
 * Outputs the exact string given to the output
 *
 * @param str String to send
 *
 * @note Assumes that str is not null
 */
void myPuts(const char *str)
{
	for (/* str set */; *str != '\0'; ++str)
	{
		myPutchar(*str);
	}
}

void led2_Init(void)
{
	// LED clock initialization
	LED2_GPIO_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_LedInit;	// Initialization for the LED
	// Initialize LED.
	GPIO_LedInit.Pin = LED2_PIN;
	GPIO_LedInit.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_LedInit.Pull = GPIO_PULLUP;
	GPIO_LedInit.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_LedInit);
}

void uart2_Init(void)
{
	// UART initialization
	// UART2 -- one connected to ST-LINK USB

	global_uartHandle.Instance = USART2;
	global_uartHandle.Init.BaudRate = 9600;					// Speed 9600
	global_uartHandle.Init.WordLength = UART_WORDLENGTH_8B;	// 8 bits/character
	global_uartHandle.Init.StopBits = UART_STOPBITS_1;			// 1 stop bit
	global_uartHandle.Init.Parity = UART_PARITY_NONE;			// no parity
	//global_uartHandle.Init.Parity = UART_PARITY_ODD;
	global_uartHandle.Init.Mode = UART_MODE_TX_RX;				// Transmit & receive
	global_uartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;	// No hw control

	// Oversample the incoming stream.
	global_uartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	// Do not use one-bit sampling.
	global_uartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

	// Noting advanced
	global_uartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	/*
	 * For those of you connecting a terminal emulator, the above parameters
	 * translate to 9600, 8, N,1.
	 */

	if(HAL_UART_Init(&global_uartHandle) != HAL_OK)
	{
		Error_Handler();
	}
}

//TODO(MARK): figure out if i should remove the code below up to the main function
void print_msg(const char * const data)
{
	for (int index = 0; data[index] != '\0'; index++)
	{
		myPutchar(data[index]);
	}
}

char reply[] = "would you like me to repat a new message\r\n";
char clear[] = "read buffer overflow clearing msg max msg is 49 chars \r\n";

#define BUFFERSIZE 50
#define EXTRASIZE = 53

void processInput(char *data, int size)
{
	if (size < BUFFERSIZE - 3)
	{
		data[size - 1] = '\r';
		data[size] = '\n';
		data[size + 1] = '\0';
	}
	else
	{
		data[size -1] = '\0';
	}
	print_msg(data);
	print_msg(reply);
}

void button_Init(void)
{
	// Push button clock initialization
	USER_BUTTON_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_ButtonInit;
	GPIO_ButtonInit.Pin = USER_BUTTON_PIN;
	GPIO_ButtonInit.Mode = GPIO_MODE_IT_RISING; // or GPIO_MODE_IT_FALLING
	GPIO_ButtonInit.Pull = GPIO_PULLDOWN;
	GPIO_ButtonInit.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(USER_BUTTON_GPIO_PORT, &GPIO_ButtonInit);

}

int main(void)
{
	HAL_Init();	// Initialize hardware.
	led2_Init();
	uart2_Init();
	button_Init();
	// Tell the chip that we want the interrupt vector
	// for USART2 to be enabled.
	//HAL_NVIC_SetPriority //TODO(Mark): i don't think i need this default priority is fine if there is that.
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_EnableIRQ(EXTI4_15_IRQn);


	// Keep sending the message for a long time.
	for (;;) {
		myPuts(global_msg[global_click]);
		HAL_Delay(500);
	}
}

/**
 * Magic function that's called by the HAL layer to actually
 * initialize the UART. In this case we need to
 * put the UART pins in alternate mode so they act as
 * UART pins and not like GPIO pins.
 *
 * @note: Only works for UART2, the one connected to the USB serial
 * converter
 *
 * @param uart the UART information
 */
void HAL_UART_MspInit(UART_HandleTypeDef *uart)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if (uart->Instance == USART2)
	{
		// Peripheral clock enable
		__HAL_RCC_USART2_CLK_ENABLE();

		/*
		 *  USART2 GPIO Configuration
		 *  PA2		------> USART2_TX
		 *  PA3		------> USART2_RX
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		// Alternate function -- that of UART
		GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

}


/**
 * Magic function called by Hal layer to de-initialize the
 * UART hardware. It's something we never do, but we put it
 * in here for the sake of completeness.
 *
 * @note; Only works for UART2, the one connected to the USB serial
 * converter
 *
 * @param uart the UART information
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *uart)
{
	if(uart->Instance == USART2)
	{
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();

		/*
		 * USART2 GPIO Configuration
		 * PA2    --------> USART2_TX
		 * PA3    --------> USART2_RX
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
	}
}
