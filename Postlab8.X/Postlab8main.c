/*
  Archivo:  Postlab8main.c
  Autor:    Alejandro Ramirez Morales
  Creado:   07/oct/21
  Potenciometro individual, alterando PORTA y valor en display de tres digitos.   
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 4000000

/*
 +----------------------------------------------------------------------------+
 |                                VARIABLES                                   |
 +----------------------------------------------------------------------------+
 */
uint8_t cuenta;
int cuenta2;
uint8_t turno;

uint8_t unidades;
uint8_t decenas;
uint8_t centenas;

uint8_t display0;
uint8_t display1;
uint8_t display2;
/*
 +----------------------------------------------------------------------------+
 |                          PROTOTIPOS DE FUNCIONES                           |
 +----------------------------------------------------------------------------+
 */
void setup(void);
uint8_t ArregloNumero (uint8_t numero);
void Displayturno (void);
void ValoresDisplay (void);
/*
 +----------------------------------------------------------------------------+
 |                               INTERRUPCIONES                               |
 +----------------------------------------------------------------------------+
 */
void __interrupt() isr (void)
{
     if(PIR1bits.ADIF)            // Bandera del ADC
    {            
         if(ADCON0bits.CHS == 6)  // Canal 6, pasar ADRESH a PORTA  
             PORTA = ADRESH;
         else
             cuenta = ADRESH;     // Canal 5, pasar ADRESH a cuenta
         
         PIR1bits.ADIF = 0;       // Apagar la bandera
    }  
     else if(INTCONbits.T0IF)
     {
         INTCONbits.T0IF = 0;     // Limpiar bandera
         PORTD = 0;               // Limpiar puerto D
         TMR0 = 131;              // Timer0 a 0.002 segundos
         
         // Turno para cada cada digito del display
         switch(turno){
            case 0:
                PORTC = display0;
                PORTD = 0b001;
                break;
            case 1:
                PORTC = display1;
                PORTD = 0b010;
                break;
            case 2: 
                PORTC = display2 + 0b10000000;
                PORTD = 0b100;
                break; 
            default:
                turno = 0;
                break;     
         }
     }
}

/*
 +----------------------------------------------------------------------------+
 |                                   LOOP                                     |
 +----------------------------------------------------------------------------+
 */
void main(void) 
{
    setup(); // Se ejecuta funcion setup
    ADCON0bits.GO = 1; // El ciclo A/D esta en progreso
    while(1)
    {   
        ValoresDisplay(); // Genera los valores de cada digito del diplay
        Displayturno();   // Dicta el turno del display
    
        if (ADCON0bits.GO == 0){
            if(ADCON0bits.CHS == 6) // Cambio de canal 
                ADCON0bits.CHS = 5;
            else
                ADCON0bits.CHS = 6;
            
            __delay_us(50);         // pausa para cambio de canal
            ADCON0bits.GO = 1;
        }
    }
}

/*
 +----------------------------------------------------------------------------+
 |                                  SETUP                                     |
 +----------------------------------------------------------------------------+
 */
void setup(void)
{
    // Ports 
    ANSEL   =   0b01100000;              // Digital Ports
    ANSELH  =   0;
    
    TRISA   =   0;              // PORTA - salida
    TRISC   =   0;              // PORTC - salida
    TRISD   =   0;              // PORTD - salida
    TRISE   =   0b011;          // PORTE - entrada
    
    PORTA   =   0;              // PORTA en 0
    PORTC   =   0;              // PORTC en 0
    PORTD   =   0;              // PORTD en 0
    PORTE   =   0;              // PORTE en 0
    
    // Reloj
    OSCCONbits.IRCF = 0b0110;    // 4MHz
    OSCCONbits.SCS = 1;         // Activar reloj interno
    
    // Configuración de las interrupciones
    PIR1bits.ADIF = 0;      // A/D conversion no ha empezado o completado
    PIE1bits.ADIE = 1;      // Activa la interrupción de ADC
    INTCONbits.PEIE = 1;    // Interrupciones perifericas
    INTCONbits.GIE = 1;     // Interrupciones globales
    INTCONbits.T0IE = 1;    // Interrupcion de TMR0
    INTCONbits.T0IF = 0;    // Bandera TMR0
    
    // TMR0
    OPTION_REGbits.PS0  = 1;
    OPTION_REGbits.PS1  = 1;
    OPTION_REGbits.PS2  = 0;    // 1:2 Prescaler
    OPTION_REGbits.PSA  = 0;
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.T0SE = 0;
    TMR0 = 131;
    
    // Configuración del ADC
    ADCON1bits.ADFM  = 0;   // Justificado a la izquierda
    ADCON1bits.VCFG1 = 0;   // Referencia como tierra
    ADCON1bits.VCFG0 = 0;   // Referencia poder
    
    ADCON0bits.ADCS = 0b01; // Fosc/8
    ADCON0bits.CHS  = 5;    // Ansel 5
    ADCON0bits.ADON = 1;    // ADC activo
    __delay_us(50);
    return;   
}

uint8_t ArregloNumero (uint8_t numero)
{
    switch(numero){
        case 0:
            return 0b00111111;
            break;    
        case 1:
            return 0b00000110;
            break;    
        case 2:
            return 0b01011011;
            break;
        case 3:
            return 0b01001111;
            break; 
        case 4:
            return 0b01100110;
            break;
        case 5:
            return 0b01101101;
            break;
        case 6:
            return 0b01111101;
            break;
        case 7:
            return 0b00000111;
            break;
        case 8:
            return 0b01111111;
            break;
        case 9:
            return 0b01101111;
            break;
        default:
            return 0b00111111;
            break;   
    }   
}

void Displayturno (void)
{
    switch(turno){
        case 0:
            turno = 1 ;
            break;  
        case 1:
            turno = 2 ;
            break; 
        case 2:
            turno = 0 ;
            break;
        default:
            turno = 0 ;
            break;   
    }  
    return;
}

void ValoresDisplay (void)
{
    // cuenta cambia en función del valor de la entrada en la interrupcion AD
        cuenta2 = (int)cuenta*100/51;
    // unidades es el modulo entres cuenta2 y 10   
        unidades = cuenta2%10;
    // cociente0 son los primeros dos digitos de la division entre cuenta2 y 10
        int cociente0 = (int)(cuenta2/10);
    // decenas es el residuo de cociente0 y 10
        decenas = cociente0%10;
    // centanas es el primer digito de la division entre cuenta2 y 10
        centenas = (int)(cociente0/10);
        
    // Pasar números al display
        display0 = ArregloNumero(unidades);
        display1 = ArregloNumero(decenas);
        display2 = ArregloNumero(centenas);
        return;
}