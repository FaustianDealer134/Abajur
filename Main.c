//*****************************************************************************
//         PROJETO:
//   
//   Controle de Versão: V1.0
//   Autor: Rafael e Matheus
//  Num |       Evento     |   Data    | Observações
//  1°  | Criação do código| 14/09/18 |
//                               |
//                               |
//                               |
//*****************************************************************************

//********************* ÁREA DE INCLUSÃO DE BIBLIOTECAS ***********************

#include <p18f4550.h> // biblioteca do microcontrolador 
#include <stdio.h> // Blibioteca de I/O 
#include <delays.h> // Biblioteca de delay
#include <timers.h> // Biblioteca de timers
#include <pwm.h>  // Biblioteca de PWM

//********************* ÁREA DE DEFINES - APELIDOS ****************************
// Estados de led
#define LED_ON 1 
#define LED_OFF 0
//Port dos LEDs
#define RED_LED PORTDbits.RD0 
#define GREEN_LED PORTDbits.RD1 
#define BLUE_LED PORTDbits.RD2
//Port dos botoes
#define BOTAO_MENOS PORTBbits.RB1
#define BOTAO_MAIS PORTBbits.RB2
//********************* ÁREA DE AJUSTE DOS BITS DE CONFIGURAÇÃO ***************

#pragma config FOSC = INTOSC_EC // Habilita o oscilador interno
#pragma config WDT = OFF    //Desabilita o Watchdog Timer (WDT).
#pragma config PWRT = ON   //Habilita o Power-up Timer (PWRT).
#pragma config BOR = OFF   //Brown-out Reset (BOR) desabilitado.
#pragma config PBADEN = OFF   //RB0,1,2,3 e 4 configurado como I/O digital.
#pragma config LVP = OFF   //Desabilita o Low Voltage Program.

//********************* ÁREA DE VARIAVEIS GLOBAIS  *****************************
unsigned char period = 255; // Periodo de PWM 

unsigned int duty_cycle = 0; // Duty cycle PWM

unsigned int luminosidade = 0; // Estado da luz

unsigned int contimer0 = 0; // Contador de estouros timer 0

unsigned int contimer1 = 0; // Contador de estouros timer 1

unsigned int menutemp = 0; //Flag de menu

unsigned char tipotemp = 0; //Tipo de temporização
//*****************************************************************************
// Área de Protótipos de Funções
//*****************************************************************************

void ISR_tratamento(void); // Tratamento de interrupcao de botao
void configuraInt(void); //Configuracao de interrupcao 
void ISR_TIMER(void); //Tratamento de interrupcao de timer
void configuraTimer0(void); //Configuracao timer 0
void setTimer0(int i); //Ativacao timer 0
void configuraTimer1(void); //Configuracao timer 1
void setTimer1(int i); //Ativacao timer 1
void controlaIntensidade(char intensidade); //Muda intensidade da luz
void menu(void); //Entra no menu

//*****************************************************************************
// Definição da função que será chamada quando ocorrer a interrupção
//*****************************************************************************

#pragma code interrupcao_alta_prior = 0x08 // Vetor de interrupção de alta prioridade

void interrupcao_alta_prior(void)
{
	_asm 
			goto ISR_TIMER 
	_endasm
}
#pragma code

#pragma code low_prioridad = 0X018 // Vetor de interrupção de baixa prioridade

 void low_interrupt  (void)
 {
	_asm
		goto  ISR_tratamento
	_endasm
 }
#pragma code

//*****************************************************************************
// Tratamemto da interrupção
//*****************************************************************************

#pragma interrupt ISR_tratamento
void ISR_tratamento(void)
{
	if(BOTAO_MENOS & BOTAO_MAIS){ //Caso os dois botoes sejam pressionados
	
		T0CONbits.TMR0ON = 0; //Desliga timer
		menutemp = 1; //Liga o flag do menu
		RED_LED = LED_OFF; //Desliga o LED vermelho e verde
		GREEN_LED= LED_OFF;
		INTCON3bits.INT1IF = 0; // limpa o flag de interrupção externa  1
		INTCON3bits.INT2IF = 0; // limpa o flag de interrupção externa  2
		
	}else if(INTCON3bits.INT1IF){ //Caso o botao de diminuicao for pressionado 
		if(menutemp){ //Caso esteja no menu
		
			menutemp = 0; //Desliga o flag do menu
			
		}else{ //No funcionamento normal
		
			controlaIntensidade('-'); //Diminui a intensidade
			GREEN_LED = LED_OFF; //Desliga o LED verde e liga o vermelho
			RED_LED = LED_ON;
			setTimer0(20); //Liga o timer para desligar o LED
			
		}
		INTCON3bits.INT1IF = 0; // Limpa o flag de interrupção externa  1

	}else if(INTCON3bits.INT2IF){ //Caso o botao de diminuicao for pressionado 
	
		if(menutemp){ //Caso esteja no menu
			//Muda o tipo de temporizacao
			if (tipotemp < 5){
				
				tipotemp++;
				
			} else {
				
				tipotemp = 1;
				
			}
			
			BLUE_LED = LED_OFF; // Desliga o LED azul
			setTimer0((tipotemp*2)-1); //Liga o timer para piscar o LED azul
			
		} else{ //No funcionamento normal
		
			controlaIntensidade('+');//Aumenta a intensidade
			RED_LED = LED_OFF; //Desliga o LED vermelho e liga o verde
			GREEN_LED = LED_ON;
			setTimer0(20);//Liga o timer para desligar o LED
			
		}
		
		INTCON3bits.INT2IF = 0; // limpa o flag de interrupção extrena  2
	}
	
}
#pragma interrupt ISR_TIMER
void ISR_TIMER(void)
{
	if(INTCONbits.TMR0IF){ //Verifica se a interrupcao foi do timer 0
		WriteTimer0(0x0F3CB); //Recarrega o registrador do timer 0
		switch(menutemp) 
		{
			case 0: //Caso nao esteja no menu
			{
				if(contimer0 == 0){
			
					RED_LED= LED_OFF; // Desliga o LED vermelho
					GREEN_LED= LED_OFF; // Desliga o LED verde
					T0CONbits.TMR0ON = 0; //Desliga o timer 0
					INTCONbits.TMR0IF = 0;// limpa flag de interrupção do timer 0
					
				} 
				if(contimer0 == 10){
					if(BOTAO_MAIS){
						controlaIntensidade('+');//Aumenta a intensidade
						contimer0 = 20; //Reset no timer 0
						INTCONbits.TMR0IF = 0;// limpa flag de interrupção do timer 0
					}
					else{
						contimer0--; //Diminui o contador de estouros
						INTCONbits.TMR0IF = 0;// limpa flag de interrupção do timer 2
					}
				}
				else{
					
					contimer0--; //Diminui o contador de estouros
					INTCONbits.TMR0IF = 0;// limpa flag de interrupção do timer 2
					
				}
				break;
			}
			case 1: //Caso esteja no menu
			{
				BLUE_LED = !BLUE_LED; //Toggle no LED azul
				
				if(contimer0 == 0){ //Verifica se o numero de estouros foi suficiente
					
					T0CONbits.TMR0ON = 0; // Desliga timer 0
					INTCONbits.TMR0IF = 0;// limpa flag de interrupção do timer 2
					
				}else{
					
					contimer0--; //Diminui o contador de estouros
					INTCONbits.TMR0IF = 0; // limpa flag de interrupção do timer 2
					
				}
				
			}
		
		}
	}else{ //Caso seja o timer 1
		WriteTimer1(0x09E58); //Recarrega o registrador do timer 1
		
		if(contimer1 == 0){ //Verifica se o numero de estouros foi suficiente
			
			luminosidade = 0; //Desliga lampada
			duty_cycle = 0; // Desliga lampada
			T1CONbits.TMR1ON = 0;//Desliga timer 1
			
		}else{
			
			contimer1--;//Diminui o contador de estouros
			
		}
		
		PIR1bits.TMR1IF = 0;// limpa flag de interrupção do timer 2
	}
	
}
//*****************************************************************************
// Função principal do projeto
//*****************************************************************************

void main (void)
{
	TRISC = 0b00000000; //RC0 a RC7 - como saida
	TRISB = 0b00001111; //RB0 a RB3 – como entrada; RB4 a RB7 como saída 
	TRISD = 0b00000000; //RD0 a RD7 - como saida

	ADCON1 = 0X0F; // Todas as entradas como pinos digitais
	PORTD = 0; //Zera o Port D
	OSCCON = 0xF2; // configura oscilador interno para 8 MHz
	 
	configuraInt(); // Configura interrupcao de botao
	configuraTimer0(); //Configura timer 0
	configuraTimer1(); //Configura timer 1
	
	OpenTimer2(TIMER_INT_OFF & T2_PS_1_1 & T2_POST_1_1); // configuração do TIMER 2
	OpenPWM1 (period); // configuração do Módulo CCP 1 como PWM1


	while (1) //Looping
	{
		if(menutemp){ // Verifica se esta no menu
			
			menu(); //Entra no menu
		}else{
			
			SetDCPWM1 (duty_cycle); //Muda o dutycycle do sinal PWM
		}
	}
}

void configuraInt()
{
//-----------------------------------------------------------------------------
//   configuração da interrupção
//-----------------------------------------------------------------------------
	INTCON2bits.INTEDG1 = 1; // seleção de borda de descida em RB2: INT ext 2
	INTCON3bits.INT1IF = 0; // limpa o flag de interrupção externa  2
	INTCON3bits.INT1IP = 0; // seleção de alta prioridade
	INTCON3bits.INT1IE = 1; // ativação da interrupção externa INT 2 (RB2)
	
	INTCON2bits.INTEDG2 = 1; // seleção de borda de descida em RB2: INT ext 2
	INTCON3bits.INT2IF = 0; // limpa o flag de interrupção externa  2
	INTCON3bits.INT2IP = 0; // seleção de alta prioridade
	INTCON3bits.INT2IE = 1; // ativação da interrupção externa INT 2 (RB2)

	RCONbits.IPEN = 1; //Habilita a interrupção com nível prioridade alta.
								// Endereço do vetor: 0x08
	INTCONbits.GIEH = 1; // Habilita todas as interrupções de alta prioridade
	INTCONbits.GIEL = 1; // Desabilita todas as interrupções de baixa prioridade
//-----------------------------------------------------------------------------
 
}

void configuraTimer0()
{
	OpenTimer0 (TIMER_INT_ON
 						& T0_16BIT
 						&T0_SOURCE_INT
 						& T0_PS_1_64); 
 // configura o Timer 0 com interrupção ligada
							 // prescaler 1x1, e postscaler 1x1
	T0CONbits.TMR0ON = 0; //desliga timer
	INTCONbits.TMR0IF = 0;// limpa flag de interrupção do timer 2
	
	RCONbits.IPEN = 1;// habilita niveis de prioridade para interrupções 
	INTCON2bits.TMR0IP = 1;// prioridade alta para o timer 0
	INTCONbits.GIEL = 1;// habilita a chave geral de interrupções
	INTCONbits.GIE = 1;// habilita a chave geral de interrupções de periféricos

}

void setTimer0(int i)
{
	contimer0 = i; //Configura o numero de estouros
	
	WriteTimer0(0x0F3CB); //Manda valor para o registrador do timer 0
	
	T0CONbits.TMR0ON = 1; //liga timer	
}

void configuraTimer1()
{
	OpenTimer1 (TIMER_INT_ON
						& T1_16BIT_RW
						&T1_SOURCE_INT
						& T1_PS_1_8); 
// configura o Timer 0 com interrupção ligada
							 // prescaler 1x1, e postscaler 1x1
	T1CONbits.TMR1ON = 0; //desliga timer
	PIR1bits.TMR1IF = 0;// limpa flag de interrupção do timer 2
	
	RCONbits.IPEN = 1;// habilita niveis de prioridade para interrupções 
	IPR1bits.TMR1IP = 1;// prioridade alta para o timer 0
	INTCONbits.GIEL = 1;// habilita a chave geral de interrupções
	INTCONbits.GIE = 1;// habilita a chave geral de interrupções de periféricos

}

void setTimer1(int i)
{
	contimer1 = i; //Configura o numero de estouros
	
	WriteTimer1(0x09E58); //Manda valor para o registrador do timer 1
	
	T1CONbits.TMR1ON = 1; //liga timer	
}

//Muda o nivel de luminosidade
void controlaIntensidade(char intensidade){
	
	if(intensidade == '+'){ //Verifica se aumenta ou diminui
	
		if(luminosidade < 10)
			luminosidade++;
		
	}else{
		
		if(luminosidade > 0)
			luminosidade--;
	}
	duty_cycle = 102*luminosidade; //Configura o dutycycle
}

void menu(){
	T1CONbits.TMR1ON = 0; //desliga timer	
	tipotemp = 1; //reinicia o estado do temporizador para 1
	
	setTimer0(1); //Pisca o led uma vez para mostrar que entrou no menu
	
	while(menutemp){ //Espera ocupada
	}
	//Seta timer de temporizacao
	if(tipotemp < 5){
		setTimer1(tipotemp*50);
	}else{
		setTimer1(300);
	}
	//PORTDbits.RD3 = 0;
}