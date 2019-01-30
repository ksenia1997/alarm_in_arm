
#include "MK60D10.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 50

#define LED_D9  0x20 // Port B, bit 5
#define LED_D10 0x10 // Port B, bit 4
#define LED_D11 0x8  // Port B, bit 3
#define LED_D12 0x4  // Port B, bit 2

char buffer[BUF_SIZE];
unsigned int initSec = 0;
unsigned int secTmp = 0;
unsigned int alarmSec = 0;
int chosen_song = 0;
int chosen_light = 0;
int chosen_repeat = 0;
int chosen_delay = 0;


// Delay
void Delay(unsigned long long int bound) {
    for(unsigned long long int i=0; i<bound; i++);
}

// Print character
void SendCh(char c) {
    while(!(UART5->S1 & UART_S1_TDRE_MASK) && !(UART5->S1 & UART_S1_TC_MASK));
    UART5->D = c;
}

// Print string
void PrintStr(char* s) {
    unsigned int i = 0;
    while (s[i] != '\0') {
        SendCh(s[i++]);
    }
}

// Receive character
unsigned char ReceiveCh() {
    while(!(UART5->S1 & UART_S1_RDRF_MASK));
    return UART5->D;
}

//Receive string
void ReceiveStr() {
	unsigned char c;
	unsigned int idx = 0;

	//empty buffer
	for (unsigned i = 0; i < BUF_SIZE; i++) {
		buffer[i] = '\0';
	}
	//filling buffer
	while(idx < (BUF_SIZE-1)) {
		c = ReceiveCh();
		SendCh(c);
		if (c == '\r') {
			break;
		}
		buffer[idx] = c;
		idx++;
	}
	buffer[idx] = '\0';

	PrintStr("\r\n"); //new line
}

bool ConvertStrToTime(char* str, unsigned int* time_dest){
	struct tm time;
	//checking length of string
	int strLen = strlen(str);
	if ((strLen < 14) ||(strLen > 19)) {
		PrintStr("Please repeat again!\r\n");
		return false;
	}

	//Parse string to time
	int parseTime = sscanf(str, "%d-%d-%d %d:%d:%d", &time.tm_mday, &time.tm_mon, &time.tm_year, &time.tm_hour, &time.tm_min , &time.tm_sec);
	if (parseTime != 6) {
		PrintStr("Wrong format of date or time!\r\n");
		return false;
	}
	//Date
	//check year
	if ((time.tm_year < 2018) || (time.tm_year > 2038)) {
		PrintStr("Wrong year! Must be in interval [2018-2038].\r\n");
		return false;
	}
	//check month
	if ((time.tm_mon < 1) || (time.tm_mon > 12)) {
		PrintStr("Wrong month!\r\n");
		return false;
	}
	//jan mar may jule august oct dec
	if ((time.tm_mon == 1) || (time.tm_mon == 3) || (time.tm_mon == 5) || (time.tm_mon == 7) ||
		(time.tm_mon == 8) || (time.tm_mon == 10) || (time.tm_mon == 12)) {
		if ((time.tm_mday < 1) || (time.tm_mday > 31)){
			PrintStr("Wrong day!\r\n");
			return false;
		}
	}
	//feb
	else if (time.tm_mon == 2) {
		if (time.tm_year % 4) {
			if ((time.tm_mday < 1) || (time.tm_mday > 29)) {
				PrintStr("Wrong day!\r\n");
				return false;
			}
		}
		else {
			if ((time.tm_mday < 1) || (time.tm_mday > 28)) {
				PrintStr("Wrong day!\r\n");
				return false;
			}
		}
	}
	//apr june sep nov
	else {
		if ((time.tm_mday < 1) || (time.tm_mday > 30)) {
			PrintStr("Wrong day!\r\n");
			return false;
		}
	}

	//Time
	//Hours
	if ((time.tm_hour < 0) || (time.tm_hour > 23)) {
		PrintStr("Wrong hour!\r\n");
		return false;
	}
	//Minutes
	if ((time.tm_min < 0) || (time.tm_min > 59)) {
		PrintStr("Wrong minutes!\r\n");
		return false;
	}
	//Seconds
	if ((time.tm_sec < 0) || (time.tm_sec > 59)) {
		PrintStr("Wrong seconds!\r\n");
		return false;
	}

    time.tm_year  = time.tm_year - 1900;
    time.tm_mon   = time.tm_mon  - 1;
    time.tm_isdst = -1;

    time_t tmp;
    tmp = mktime(&time);
    *time_dest = (unsigned int)tmp;

	return true;
}

void ConvertTimeToStr(unsigned int* time, char* str) {
	 time_t tmp = *time;
	 struct tm t = *localtime(&tmp);
	 for (int i = 0; i < BUF_SIZE; i++) {
		 str[i] = '\0';
	 }
	 strftime(str, BUF_SIZE, "%Y-%m-%d %H:%M:%S", &t);
}
// Beep
void Beep() {
    for (unsigned int i=0; i<600; i++) {
        PTA->PDOR = GPIO_PDOR_PDO(0x0010);
        Delay(600);
        PTA->PDOR = GPIO_PDOR_PDO(0x0000);
        Delay(600);
    }
}

void Music(int song) {
	if (song == 1) {
		for(unsigned int i=0; i<10; i++) {
			Beep();
			Delay(90000);
		}
	}

	else if (song == 2) {
		for(unsigned int i=0; i<10; i++) {
			Beep();
			Delay(10000);
			Beep();
			Beep();
			Delay(1000000);
		}
	}

	else if (song == 3) {
		for(unsigned int i=0; i<10; i++) {
			Beep();
			Beep();
			Delay(9000);
			Beep();
			Delay(9000);
			Beep();
			Delay(9000);
			Beep();
			Delay(1000000);

		}
	}
}

void Light(int light){
	if (light == 1) {
		 for(unsigned int i=0; i<10; i++) {

			GPIOB_PDOR ^= LED_D12;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(500000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D11;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(500000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D10;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(500000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D9;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(500000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

		 }
	}

	else if (light == 2) {
		 for(unsigned int i=0; i<10; i++) {

			GPIOB_PDOR ^= LED_D12;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(900000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D10;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(900000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

		 }

		 for(unsigned int i=0; i < 10; i++) {
			GPIOB_PDOR ^= LED_D11;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(900000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D9;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(900000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);
		 }
	}

	else if (light == 3) {
		for(unsigned int i=0; i < 10; i++) {
			GPIOB_PDOR ^= LED_D12;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(300000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D9;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(300000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D11;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(900000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);

			GPIOB_PDOR ^= LED_D10;
			PTB->PDOR &= ~GPIO_PDOR_PDO(0x1);
			Delay(900000);
			PTB->PDOR |= GPIO_PDOR_PDO(0x3C);
		}
	}
}

int Initialization() {
	RTC_SR &= ~RTC_SR_TCE_MASK; // turn OFF RTC
	RTC_TSR = initSec;
	RTC_SR |= RTC_SR_TCE_MASK; // turn ON RTC
	return 1;
}

int MusicChoose() {
	ReceiveStr();
	if(strcmp(buffer,"M1")==0) {
		Music(1);
		return 1;
	}
	else if (strcmp(buffer, "M2") == 0) {
		Music(2);
		return 1;
	}
	else if (strcmp(buffer, "M3") == 0) {
		Music(3);
		return 1;
	}
	else if (strcmp(buffer, "1") == 0) {
		chosen_song = 1;
		return 2;
	}
	else if (strcmp(buffer, "2") == 0) {
		chosen_song = 2;
		return 2;
	}
	else if (strcmp(buffer, "3") == 0) {
		chosen_song = 3;
		return 2;
	}

	return 1;
}

int LightChoose(){
	ReceiveStr();
	if(strcmp(buffer,"L1")==0) {
		Light(1);
		return 2;
	}

	else if (strcmp(buffer, "L2") == 0) {
		Light(2);
		return 2;
	}

	else if (strcmp(buffer, "L3") == 0) {
		Light(3);
		return 2;
	}

	else if (strcmp(buffer, "1") == 0) {
		chosen_light = 1;
		return 3;
	}
	else if (strcmp(buffer, "2") == 0) {
		chosen_light = 2;
		return 3;
	}
	else if (strcmp(buffer, "3") == 0) {
		chosen_light = 3;
		return 3;
	}
	return 2;
}

int RepChoose() {
	ReceiveStr();
	bool isRepetition = sscanf(buffer,"%d", &chosen_repeat);
	if (isRepetition) {
		if ((chosen_repeat < 0) || (chosen_repeat > 3)) {
			PrintStr("Please choose repetition from 0 to 3.\r\n");
		}
		else {
			if (chosen_repeat == 0) {
				return 5;
			}
			else {
				return 4;
			}
		}
	}

	else {
		PrintStr("Wrong count of repetition.\r\n");
	}

	return 3;
}

int DelayChoose() {
	ReceiveStr();
	bool isDelay = sscanf(buffer,"%d", &chosen_delay);
	if (isDelay) {
		if ((chosen_delay < 10) || (chosen_delay > 100)) {
			PrintStr("Please enter delay in seconds from 10 to 100.\r\n");
		}
		else {
			return 5;
		}
	}
	else {
		PrintStr("Wrong delay.\r\n");
	}

	return 4;
}

int AlarmInit() {
	secTmp = RTC_TSR;
	PrintStr("Current date and time:");
	ConvertTimeToStr(&secTmp, buffer);
	PrintStr(buffer);
	PrintStr("\r\n");
	ReceiveStr();
	bool isTime  = ConvertStrToTime(buffer, &alarmSec);
	if ((isTime) && (RTC_TSR < alarmSec)) {
		RTC_TAR = alarmSec;
		return 6;
	}

	else {
		PrintStr("Please try again alarm initialization.\r\n");
	}
	return 5;
}

int Activate() {
	PrintStr("Current date and time: ");
	secTmp = RTC_TSR;
	ConvertTimeToStr(&secTmp, buffer);
	PrintStr(buffer);
	PrintStr("\r\n");

	PrintStr("Alarm date and time: ");
	secTmp = RTC_TAR;
	if (secTmp == 0) {
		PrintStr("alarm does not exist\r\n");
	}
	else {
		ConvertTimeToStr(&secTmp, buffer);
		PrintStr(buffer);
		PrintStr("\r\n");
	}

	PrintStr("Please choose command: \r\n");
	PrintStr("new - to create new alarm.\r\n");
	PrintStr("off - to disable alarm.\r\n");
	PrintStr("reboot - to restart initialization\r\n");
	PrintStr("poweroff - to power off system\r\n");

	ReceiveStr();

	if (strcmp(buffer, "new") == 0) {
		RTC_TAR = 0; //turn off alarm
		return 1;
	}

	else if (strcmp(buffer, "off") == 0) {
		RTC_TAR = 0;
		PrintStr("You turned off the alarm.\r\n");
	}

	else if (strcmp(buffer, "reboot") == 0) {
		RTC_TAR = 0;
		return 0;
	}

	else if (strcmp(buffer, "poweroff") == 0) {
		RTC_TAR = 0;
		return 7;
	}

	return 6;
}
bool isConverted;

// RTC interrupt handler
void RTC_IRQHandler() {
    if(RTC_SR & RTC_SR_TAF_MASK) {

        // Time Alarm Flag
        Music(chosen_song);
        Light(chosen_light);

        if (chosen_repeat > 0) {
            chosen_repeat--;
            RTC_TAR += chosen_delay;
        }
        else {
            RTC_TAR = 0;
        }

    }

}
int main() {

	//MCU INIT
	MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
	SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK; // turn off watchdog

	//PORT INIT
	// Enable CLOCKs for PORT-A, PORT-B, PORT-E, UART5, RTC
	SIM->SCGC1 = SIM_SCGC1_UART5_MASK;
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;
	SIM->SCGC6 = SIM_SCGC6_RTC_MASK;

	// PORT A
	PORTA->PCR[4] = PORT_PCR_MUX(0x01);

	// PORT B
	PORTB->PCR[5] = PORT_PCR_MUX(0x01); // D9  LED
	PORTB->PCR[4] = PORT_PCR_MUX(0x01); // D10 LED
	PORTB->PCR[3] = PORT_PCR_MUX(0x01); // D11 LED
	PORTB->PCR[2] = PORT_PCR_MUX(0x01); // D12 LED

	// PORT E
	PORTE->PCR[8]  = PORT_PCR_MUX(0x03); // UART0_TX
	PORTE->PCR[9]  = PORT_PCR_MUX(0x03); // UART0_RX
	PORTE->PCR[10] = PORT_PCR_MUX(0x01); // SW2
	PORTE->PCR[12] = PORT_PCR_MUX(0x01); // SW3
	PORTE->PCR[27] = PORT_PCR_MUX(0x01); // SW4
	PORTE->PCR[26] = PORT_PCR_MUX(0x01); // SW5
	PORTE->PCR[11] = PORT_PCR_MUX(0x01); // SW6

	// set ports as output
	PTA->PDDR =  GPIO_PDDR_PDD(0x0010);
	PTB->PDDR =  GPIO_PDDR_PDD(0x3C);
	PTB->PDOR |= GPIO_PDOR_PDO(0x3C); // turn all LEDs OFF

	//UART5 INIT
	UART5->C2  &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
	UART5->BDH =  0x00;
	UART5->BDL =  0x1A; // baud rate is 115 200 Bd, 1 stop bit
	UART5->C4  =  0x0F; // oversampling ratio is 16, match address mode disabled
	UART5->C1  =  0x00; // 8 data bits, without parity bit
	UART5->C3  =  0x00;
	UART5->MA1 =  0x00; // no match address (mode disabled in C4)
	UART5->MA2 =  0x00; // no match address (mode disabled in C4)
	UART5->S2  |= 0xC0;
	UART5->C2  |= ( UART_C2_TE_MASK | UART_C2_RE_MASK ); // Switch on transmitter and receiver

	//RTC INIT
    RTC_CR |= RTC_CR_SWR_MASK;  // SWR = 1, reset all RTC's registers
    RTC_CR &= ~RTC_CR_SWR_MASK; // SWR = 0

    RTC_TCR = 0x0000; // reset CIR and TCR

    RTC_CR |= RTC_CR_OSCE_MASK; // enable 32.768 kHz oscillator

    Delay(0x600000);

    RTC_SR &= ~RTC_SR_TCE_MASK; // turn OFF RTC

    RTC_TSR = 0x00000000; // MIN value in 32bit register
    RTC_TAR = 0xFFFFFFFF; // MAX value in 32bit register

    RTC_IER |= RTC_IER_TAIE_MASK;

    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);

    RTC_SR |= RTC_SR_TCE_MASK; // turn ON RTC

	Delay(550);
	int state = 0;
	while(1) {
		switch(state) {
			//Init
			case 0:
				PrintStr("Please enter date and time: DD-MM-YYYY HH:MM:SS \n\r");
				ReceiveStr();

				isConverted = ConvertStrToTime(buffer, &initSec);
				if (!isConverted) {
					PrintStr("Please, enter date and time again!\r\n");
				}

				else {
					state = Initialization();
				}
				break;
			//Music
			case 1:
				PrintStr("Please, choose music for alarm\r\n");
				PrintStr("To listen all possible songs please type M[1-3].\r\n");
				PrintStr("To choose song please type [1-3]\r\n");
				state = MusicChoose();
				break;
			//Light
			case 2:
				PrintStr("Please choose light for alarm\r\n");
				PrintStr("To see all possible lights please type L[1-3].\r\n");
				PrintStr("To choose light please type [1-3]\r\n");
				state = LightChoose();
				break;
			//Repetition
			case 3:
				PrintStr("Please enter count of alarm's repetition [0-3]!\r\n");
				PrintStr("If you do not want repetition, please type 0\r\n");
				state = RepChoose();
				break;
			//Delay
			case 4:
				PrintStr("Please enter delay in seconds [10-100]!\r\n");
				state = DelayChoose();
				break;
			//Alarm
			case 5:
				PrintStr("Please enter date and time for alarm in format DD-MM-YYYY HH:MM:SS\r\n");
				state = AlarmInit();
				break;
			//Activation, choosing commands for alarm
			case 6:
				state = Activate();
				break;
			case 7:
				PrintStr("System is PowerOff\r\n");
				while(1);
				break;
			default:
				break;

		}


	}
    return 0;
}
