/*
Guitar Looper

Records the guitar signal and stores it in memory.
Recorded sound is stored in PCM format at 22.05kHz 8bit

*/

#include "Arduino.h"

#include <adc.h>

#define SIZEOFBUFFER 1000

uint16_t buffer[SIZEOFBUFFER];
uint16_t toggle = 1;

void startTimer(){
  pmc_set_writeprotect(0); //enable the write protect register of the pmc_set_writeprotect
  pmc_enable_periph_clk(ID_TC0); //enable timer counter 0, channel 0
  //Configure TC0 channel 1, Waveform Mode, Reset tc_val on RC compare, chose mclk/2, TIOA toggle on RC compare
  TC_Configure(TC0, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_ACPA_NONE |  TC_CMR_ACPC_TOGGLE);
  TC_SetRC(TC0, 0, 952); // sets interrupt rate; curretly set for 22.05KHz
  TC_Start(TC0, 0);       // Start the timer
}

void pinInit(){
  //We want to output the counter timer onto pin 2 of the Arduino
  //tc0 outputs to TIOA0, which is on peripheral B; we need to set this up
  PIOB->PIO_PDR = PIO_PB25B_TIOA0 ;  // disable PIO control
  PIOB->PIO_IDR = PIO_PB25B_TIOA0 ;   // disable PIO interrupts
  PIOB->PIO_ABSR |= PIO_PB25B_TIOA0 ;  // switch to B peripheral
}

void adcSetUp(){
  pmc_enable_periph_clk(ID_ADC);
  adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST);

  ADC->ADC_CHER = ADC_CHER_CH7; //enable channel 7, which is A0 in the due board

  ADC->ADC_MR |= ADC_MR_TRGEN_EN | ADC_MR_TRGSEL_ADC_TRIG1; // enable TIOA trigger
  ADC->ADC_MR |= ADC_MR_FREERUN_OFF;  //turn on freerun mode
  // setting up the timings for the ADC (I HAVE NO CLUE HERE, JUST CHUCKING IN RANDOM NUMBERS!!!)
  ADC->ADC_MR |= ADC_MR_STARTUP_SUT64 | ADC_MR_SETTLING_AST5;
  ADC->ADC_MR |= (0x0F << ADC_MR_TRACKTIM_Pos);
}

void pdcSetUp(){
  //set up pointer to buffer and counter for buffer.
  PDC_ADC->PERIPH_RPR = (uint32_t)buffer;
  PDC_ADC->PERIPH_RCR = SIZEOFBUFFER;
  PDC_ADC->PERIPH_PTCR = PERIPH_PTCR_RXTEN;
}

void setup() {
  Serial1.begin(9600); //used for debugging only
  startTimer();
  pinInit();
  pdcSetUp();
  adcSetUp();
  pinMode(10, OUTPUT);
}

void loop() {

  if((ADC->ADC_ISR & ADC_ISR_ENDRX) != 0)
  {
    toggle ^= 1;
    pdcSetUp();
  }

  if(toggle)
  digitalWrite(10, HIGH);
  else
  digitalWrite(10, LOW);

}
