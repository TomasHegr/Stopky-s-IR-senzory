#include "main.h"
#include "milis.h"
#include <stdbool.h>
#include <stm8s.h>
// #include "delay.h"
#include "uart1.h"
#include <stdio.h>

//makra pro startovací a stopovací senzor
#define START_PIN GPIO_PIN_3
#define START_PORT GPIOG

#define STOP_PIN GPIO_PIN_2
#define STOP_PORT GPIOG

//makra pro diody

#define ZELENA_PIN GPIO_PIN_7
#define ZELENA_PORT GPIOB

#define CERVENA_PIN GPIO_PIN_6
#define CERVENA_PORT GPIOB

//globální proměnné pro rutinu přerušení TIM3
extern uint32_t cas;
extern uint8_t stav;

void init(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovani MCU na 16MHz
    init_milis();
    init_uart1();

    //inicializace pinů
    GPIO_Init(CERVENA_PORT, CERVENA_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(ZELENA_PORT, ZELENA_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

    GPIO_Init(START_PORT, START_PIN, GPIO_MODE_IN_PU_IT);
    GPIO_Init(STOP_PORT, STOP_PIN, GPIO_MODE_IN_PU_IT);
    
    

    //inicializace timeru
    TIM3_TimeBaseInit(
        TIM3_PRESCALER_16,1000 - 1); //nastavení timeru pro 1ms
        TIM3_Cmd(ENABLE);            
    TIM3_ITConfig(TIM3_IT_UPDATE,ENABLE); 
    enableInterrupts(); //povolení přerušení
}


int main(void)
{

uint32_t time = 0;
uint8_t start_active=0;
uint8_t stop_active=0;
uint32_t prevod_cas_s ;
uint32_t prevod_cas_ms;



    init();
    printf("Pro spuštění stopek projeďte prvním senzorem\n"); //Úvodní zpráva
    HIGH(ZELENA);


    while (1) {
       
        if (milis()-time> 1){ // Vzorkování pro každou 1 ms
            time=milis(); 

            if (!PUSH(START) && start_active) { //podmínka pro spuštění stopek (reakce na sestupnou hranu)
            
            cas = 0; // ošetření aby měření bylo spuštěno od nuly
            stav = 1; // globální proměnná, která v rutině přerušení povoluje inkrementaci proměnné čas
            printf("Váš čas byl právě odstartován!\n");
            LOW(ZELENA);
            HIGH(CERVENA);
           
            
        }
        start_active = PUSH(START);
        
        if(stav==1){                //podmínka pro vypisování aktuálního času stopek
            prevod_cas_s =cas/1000;     //výpočty pro převedení času na sekundy a milisekundy
            prevod_cas_ms=cas%1000;

            printf("Čas: %ld,%03ld s\r",prevod_cas_s,prevod_cas_ms); //vypisování aktuálního času stopek
            
        
        }
        
        if (PUSH(STOP) && !stop_active && stav==1) { // podmínka pro zastavení stopek (reakce na nástupnou hranu)
            stav = 0;
            printf("\nVáš čas je %ld,%03ld sekund, pro nový čas projeďte prvním senzorem\n", prevod_cas_s,prevod_cas_ms); //výpis o změřeném čase
            LOW(CERVENA);
            HIGH(ZELENA);
            
        }
        stop_active = PUSH(STOP);
        
    }
        }

        
    

       
    // delay_ms(333);
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
