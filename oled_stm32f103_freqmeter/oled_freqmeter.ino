#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128                // OLED display width
#define SCREEN_HEIGHT 64  

#define OLED_RESET     -1      // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // device name is oled

volatile int  mon_flag;
unsigned long freq;
float freqc;
byte x;
 
void setup() {
      oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // select 3C or 3D (set your OLED I2C address)
      oled.clearDisplay();
      pinMode(PA15,INPUT_PULLDOWN); 
 
      RCC->APB1ENR|= (1<<2)|(1<<1)|(1<<0); ////enable clocking tim-2,3,4
      RCC->APB2ENR|= (1<<3)|(1<<11)|(1<<2)|(1<<0)|(1<<4);////enable clocking port-a-b-c,tim1
      AFIO->MAPR=(1<<8)|(1<<6); //tim 1 && tim 2 Partial remap
}
 
void loop() {
   freq_meter();

   oled.clearDisplay();
   oled.setCursor(0,10);
   oled.setTextSize(2);
   oled.setTextColor(WHITE);

   

   if(freq>999999){
      freqc = (float)freq/1000000;
      oled.print("Freq(mhz):");
      oled.setCursor(0,30);
      oled.setTextSize(2);
      oled.print(freqc,3);
   }else if(freq>999){
      freqc = (float)freq/1000;
      oled.print("Freq(khz):");
      oled.setCursor(0,30);
      oled.setTextSize(2);
      oled.print(freqc,3);
   }else {
      oled.print("Freq(hz):");
      oled.setCursor(0,30);
      oled.setTextSize(1);
      oled.print(freq);
   }

   oled.display();
}
 
void freq_meter(){  // http://arduino.ru/forum/proekty/generator-s-reguliruemoei-chastotoi-na-arduino#comment-296530
   __asm volatile( "cpsid i" );
/// TIM2 count of low 16 bits
    TIM2->CR1=0;//stop timer
    TIM2->CCER=0; TIM2->PSC=0;  TIM2->CNT=0; 
    TIM2->CCR1=0; TIM2->CCR2=0; TIM2->CCR3=0; 
    TIM2->CCR4=0;TIM2->PSC=0;TIM2->SR=0;
    TIM2->CCMR2=0;
    TIM2->CR2=1<<5; //MMS:010 управление подчинённым в режиме "Update" 
    TIM2->SMCR= (1<<14);// ECE & TS:000  режим 2 внешнего тактирования & разрешение работы от таймера1
    TIM2->ARR=65535; //count to maximum
    TIM2->EGR=1; //reread registers.
    TIM2->CR1|=(1<<0);//start TIM2
   /// TIM3 counting of the most significant 16 bits
    TIM3->CR1=1<<0;//stop timer
    TIM3->CCER=0; TIM3->PSC=0; TIM3->CNT=0; 
    TIM3->CCR1=0; TIM3->CCR2=0; TIM3->CCR3=0; 
    TIM3->CCR4=0;TIM3->PSC=0;TIM3->SR=0;TIM3->CR2=0;  
    TIM3->CCMR1=0; 
    TIM3->SMCR=(1<<2)|(1<<1)|(1<<0)|(1<<4);//SMS:111 && TS:001  take the clock from the 2nd timer
    TIM3->ARR=65535; 
    TIM3->EGR=1; 
    TIM3->CR1|=(1<<0);//start TIM3
/// Setting the resolution time on timer1 for timer2
    TIM1->CR1=(1<<3)|(1<<2);//one pulse, no interruptions
    TIM1->CNT=0;
    TIM1->CR2=(1<<4);  //MMS:001 signal to allow other timers to operate
    TIM1->CCER=0;// disable timer outputs on physical legs
    TIM1->PSC=F_CPU/36000 -1 ;// 1999; // 72000000/2000= 36000kHz timer clock
    TIM1->ARR=35996;//count to 36000 (1 second)
    TIM1->EGR=1; //reread registers.
    TIM1->CR1|=(1<<0);
    __asm volatile( "cpsie i" );
    while (TIM1->CR1&1) {asm volatile("nop"); if(mon_flag) {return;}  }
    freq = TIM3->CNT<<16  | TIM2->CNT ; 
}