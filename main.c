#include <stdint.h>
#include <stm32f10x.h>
#include "StaticLib/delay.h"
#include "StaticLib/buttons.h"
#include <stdbool.h>


bool ToggleLED; // Adding global variable, which control LED light toggling

int __attribute((noreturn)) main(void)
{
	// ENABLING CLOCKS
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // Enabling clock for AFIO
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // Enabling clock for GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // Enabling clock for GPIOC
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enabling clock for TIM2

	// CONFIGURING IO PORTS
	GPIOC->CRH &= ~GPIO_CRH_CNF13;	 // Setting up port C13 in push-pull output mode
	GPIOC->CRH |= GPIO_CRH_MODE13_0; //

	GPIOC->CRH &= ~GPIO_CRH_CNF14;	// Setting up port C14 in input pull-up mode
	GPIOC->CRH |= GPIO_CRH_CNF14_1; //
	GPIOC->CRH &= ~GPIO_CRH_MODE14; //
	GPIOC->ODR |= GPIO_ODR_ODR14;	//

	GPIOA->CRL &= ~GPIO_CRL_CNF1;  // Setting up port A1 in input pull-up mode
	GPIOA->CRL |= GPIO_CRL_CNF1_1; //
	GPIOA->CRL &= ~GPIO_CRL_MODE1; //
	GPIOA->ODR |= GPIO_ODR_ODR1;   //

	GPIOA->CRL &= ~GPIO_CRL_CNF2;  // Setting up port A2 in input pull-up mode
	GPIOA->CRL |= GPIO_CRL_CNF2_1; //
	GPIOA->CRL &= ~GPIO_CRL_MODE2; //
	GPIOA->ODR |= GPIO_ODR_ODR2;   //

	// CONFIGURING TIMER TIM2
	RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;	// Reseting TIM2
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST; //
	TIM2->PSC = 999U;							// Editing prescaler value (1-65536) to configure timer frequency
	TIM2->ARR = 1000;						// Editing TIM2 reload value (count to X number)
	TIM2->CCR1 = 500U;
	TIM2->DIER |= TIM_DIER_UIE | TIM_DIER_CC1IE; // Enabling "Update" interrupt
	NVIC_ClearPendingIRQ(TIM2_IRQn);			 // Clearing pending interuptions for TIM2 ???
	NVIC_EnableIRQ(TIM2_IRQn);					 // Enabling interuptions from TIM2
	TIM2->CR1 |= TIM_CR1_CEN;					 // Enabling counter

	// BODY

	ToggleLED = false;
	GPIOC->ODR |= 1 << 13; // Disabling LED by default
	bool Mid_state, Up_state, Down_state;
	uint16_t Current_PSC_value;

	while (true)
	{
		// All "button press" events organized in hierarchial order because of their mutual exclusiveness
		// MID BUTTON - first priority - toggling LED light

		// Function BtnCLick is triggering Click event, according to its input data:
		// Port, Pin, ClickMode(OnPress -"D", OnRelease - "U", While holding -"H"), 
		// Defalut delay (for "D" and "U" mode) and Hold delay (for "H" mode).
		// Integer, returned by function, provides information about internal errors.
		// ErrCodes 7,8,9 are indicating successfull event generatin after corresponding button was pressed,
		// 0 - corresponding button was not pressed.

		if (BtnClick('C',14,'D',333,10000)==7)
		{

		}
		// UP BUTTON - second priority - increasing LED brightness
		else if (BtnClick('A',1,'D',333,10000)==7)
			{

			}
			// DOWN BUTTON - third priority - decreacing LED brightness
			else if (BtnClick('A',2,'D',333,10000)==7)
				{

				}
				// NOTHING HAPPEND
				else
				{
					delay_us(333);
				}
			
		
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF) // True if Update Interrupt Flag is set
	{
		if (ToggleLED)
		{
			uint32_t _gpios = GPIOC->ODR;
			GPIOC->BSRR = (~_gpios & (1 << 13)); // Toggling LED light
		}
		TIM2->SR &= ~TIM_SR_UIF; // Clearing UIF flag
	}
	if (TIM2->SR & TIM_SR_CC1IF)
	{
		if (ToggleLED) 
		{
			uint32_t _gpios = GPIOC->ODR;
			GPIOC->BSRR = ((_gpios & (1 << 13)) << 16); // Toggling LED light
		}
		TIM2->SR &= ~TIM_SR_CC1IF; //Clearing Count compare interrupt flag
	}
}

void ButtonClick_C_14_Down()
{
	// ON/OFF LED light on MID button click
	ToggleLED = !ToggleLED;
	if (!ToggleLED) 
	{
		GPIOC-> BSRR =~GPIOC->ODR & (1<<13);
	} else
	{
		GPIOC-> BSRR = GPIOC->ODR & (1<<13)<<16;
	}

}

void ButtonClick_A_1_Down()
{	
	if (TIM2 -> CCR1 < 1000) 
	{
		TIM2->CCR1 +=100;
	}

}

void ButtonClick_A_2_Down()
{	
	if (TIM2 -> CCR1 > 100) 
	{
		TIM2->CCR1 -=100;
	}

}

void SPI1_Init(void)
{
//Включаем тактирование SPI1 и GPIOA
RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPAEN;
/**********************************************************/
/*** Настройка выводов GPIOA на работу совместно с SPI1 ***/
/**********************************************************/
//PA7 - MOSI (=SI Slave In)
//PA6 - MISO (Free)
//PA5 - SCK (=SCL Clock)

// Also used with the display
//PA4 - CS (=CS Chip Select)
//PA3 - RS (also called A0, data=1 or command=0)
//PA2 - RSE (Reset=0, Standby=1)
//Для начала сбрасываем все конфигурационные биты в нули
GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2
| GPIO_CRL_CNF3 | GPIO_CRL_MODE3
| GPIO_CRL_CNF4 | GPIO_CRL_MODE4
| GPIO_CRL_CNF5 | GPIO_CRL_MODE5
| GPIO_CRL_CNF6 | GPIO_CRL_MODE6
| GPIO_CRL_CNF7 | GPIO_CRL_MODE7);

//Настраиваем
//SCK: MODE5 = 0x03 (11b); CNF5 = 0x02 (10b)
GPIOA->CRL |= GPIO_CRL_MODE5 | GPIO_CRL_CNF5_1;
//MISO: MODE6 = 0x00 (00b); CNF6 = 0x01 (01b)
GPIOA->CRL |= GPIO_CRL_CNF6_0;
//MOSI: MODE7 = 0x03 (11b); CNF7 = 0x02 (10b)
GPIOA->CRL |= GPIO_CRL_MODE7 | GPIO_CRL_CNF7_1;

//CS: MODE4 = 0x03 (11b); CNF4 = 0x00 (00b)
GPIOA->CRL |= GPIO_CRL_MODE4;

//RS: MODE3 = 0x03 (11b); CNF3 = 0x00 (00b)
GPIOA->CRL |= GPIO_CRL_MODE3;

//RESET: MODE2 = 0x03 (11b); CNF2 = 0x01 (00b)
GPIOA->CRL |= GPIO_CRL_MODE2;
GPIOA->ODR |= GPIO_ODR_ODR2; // Set 'RESET' high (Standby mode)

/**********************/
/*** Настройка SPI1 ***/
/**********************/
SPI1->CR1 &= ~SPI_CR1_DFF; // DFF=0 Размер кадра 8 бит
SPI1->CR1 &= ~SPI_CR1_LSBFIRST; // LSBFIRST=0 MSB First
SPI1->CR1 &= ~SPI_CR1_CRCEN; // Disable CRC
SPI1->CR1 |= SPI_CR1_SSM; // Программное управление SS
SPI1->CR1 |= SPI_CR1_SSI; // SS в высоком состоянии
SPI1->CR1 &= ~SPI_CR1_BR; // Clear BR[2:0] bits
SPI1->CR1 |= SPI_CR1_BR_2; // BR[2:0]=100, Скорость передачи: F_PCLK/32
SPI1->CR1 |= SPI_CR1_MSTR; // Режим Master (ведущий)
SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA); //Режим работы SPI: CPOL=0 CPHA=0
SPI1->CR1 |= SPI_CR1_SPE; //Включаем SPI
}

void SPI1_Write(uint8_t data)
{
//Ждем, пока не освободится буфер передатчика
while(!(SPI1->SR & SPI_SR_TXE));
//заполняем буфер передатчика
SPI1->DR = data;
}

uint8_t SPI1_Read(void)
{
SPI1->DR = 0; //запускаем обмен
//Ждем, пока не появится новое значение
//в буфере приемника
while(!(SPI1->SR & SPI_SR_RXNE));
//возвращаем значение буфера приемника
return SPI1->DR;
}