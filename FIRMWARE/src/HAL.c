#include "HAL.h"
#include <string.h>
#include "comm.h"


volatile uint8_t touch_tick = 0;
volatile int sensor0_time = 0;
volatile int sensor1_time = 0;
volatile uint8_t main_tick = 0;
volatile uint8_t tick = 0;
volatile uint8_t read_tick = 0;
volatile uint32_t main_tick_count = 0;
volatile uint8_t comms_tick = 0;

volatile uint16_t active_input_pins[NUM_INPUTS] = {[0 ... 3] = 0};

volatile uint8_t active_input_tick[NUM_INPUTS] = {[0 ... 3] = 0};

volatile uint16_t active_output_pins[NUM_INPUTS] = {[0 ... 3] = 0};

volatile uint32_t dendrite_pulses[NUM_DENDS] = {0};
volatile uint8_t dendrite_pule_count = 0;

uint32_t active_input_ports[NUM_INPUTS] = {
	PORT_AXON1_IN,
	PORT_AXON2_IN,
	PORT_DEND1_EX,
	PORT_DEND1_IN
};

uint32_t active_output_ports[NUM_INPUTS] = {
	PORT_AXON1_EX,
	PORT_AXON2_EX,
	PORT_DEND1_EX,
	PORT_DEND1_IN
};

const uint16_t complimentary_pins[NUM_INPUTS] = {
	PIN_AXON1_EX,
	PIN_AXON2_EX,
	PIN_DEND1_IN,
	PIN_DEND1_EX
};

const uint32_t complimentary_ports[NUM_INPUTS] = {
	PORT_AXON1_EX,
	PORT_AXON2_EX,
	PORT_DEND1_IN,
	PORT_DEND1_EX
};

volatile uint8_t dendrite_pulse_flag[NUM_INPUTS] = {[0 ... NUM_INPUTS-1] = 0};
volatile uint8_t dendrite_ping_flag[NUM_INPUTS] = {[0 ... NUM_INPUTS-1] = 0};

static uint32_t fingerprint[3] __attribute__((section (".fingerprint"))) __attribute__ ((__used__)) = {
	4, // device id
	2, // firmware version
	0  // unique id
};

bool checkVersion(uint32_t device_id, uint32_t version) {return (device_id == fingerprint[0]) && (version = fingerprint[1]) ? true : false;}

void clock_setup(void)
{
	// STM32F0 command:	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_set_sysclk_source(RCC_HSI16);
	rcc_osc_on(RCC_HSI16);
}

void systick_setup()
{
    systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
    STK_CVR = 0;
    systick_set_reload(60);
    systick_counter_enable();
    systick_interrupt_enable();
}

void sys_tick_handler(void)
{    
	switch (touch_tick){
        
		case 0:
			start_touch(SENSOR0);
			touch_tick++;
			break;
		case 1:
			sensor0_time = get_touch(SENSOR0);
			touch_tick++;
			break;
		case 2:
			start_touch(SENSOR1);
			touch_tick++;
			break;
		case 3:
			sensor1_time = get_touch(SENSOR1);
			touch_tick = 0;
			break;
		default:
			touch_tick = 0;
			break;
	}

	if (++tick >= 150){
		main_tick = 1;
		tick = 0;
	}
/*
	if (tick % 3 == 0){
		if (read_tick++ >= 2){
			writeBit();
			read_tick = 0;
		}

		readBit(read_tick);
	}
	*/
}

void gpio_setup(void)
{
	/*	Enable GPIO clocks */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SYSCFGEN);

	/*	Set up LED pins
		Alternative Function Mode with no pullup/pulldown
		Output options: push-pull, high speed
		PIN_R_LED (PA1): AF2, TIM2_CH2
		PIN_G_LED (PA0): AF2, TIM2_CH1
		PIN_B_LED (PA2): AF2, TIM2_CH3
	*/

	gpio_mode_setup(PORT_R_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_R_LED);
	gpio_mode_setup(PORT_G_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_G_LED);
	gpio_mode_setup(PORT_B_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_B_LED);
	gpio_set_output_options(PORT_R_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_R_LED);
	gpio_set_output_options(PORT_G_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_G_LED);
	gpio_set_output_options(PORT_B_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_B_LED);
	gpio_set_af(PORT_R_LED, GPIO_AF2, PIN_R_LED);
	gpio_set_af(PORT_G_LED, GPIO_AF2, PIN_G_LED);
	gpio_set_af(PORT_B_LED, GPIO_AF2, PIN_B_LED);

	/*	Set up touch sensor pins as floating AF pins */
	gpio_mode_setup(PORT_TOUCH0_SENSE, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_TOUCH0_SENSE);
	gpio_mode_setup(PORT_TOUCH1_SENSE, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_TOUCH1_SENSE);
	gpio_set_af(PORT_TOUCH0_SENSE, GPIO_AF0, PIN_TOUCH0_SENSE);
	gpio_set_af(PORT_TOUCH1_SENSE, GPIO_AF5, PIN_TOUCH1_SENSE);

	/*	Set up touch sensor resistor pins and set them low to start */
    gpio_mode_setup(PORT_TOUCH0_RES, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, PIN_TOUCH0_RES);
    gpio_mode_setup(PORT_TOUCH1_RES, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, PIN_TOUCH1_RES);

	gpio_clear(PORT_TOUCH0_RES, PIN_TOUCH0_RES);
	gpio_clear(PORT_TOUCH1_RES, PIN_TOUCH1_RES);

    // setAsInput(PORT_AXON1_IN, PIN_AXON1_IN);
    // setAsInput(PORT_AXON2_IN, PIN_AXON2_IN);
	setAsInput(PORT_DEND1_IN, PIN_DEND1_IN);
    setAsInput(PORT_DEND1_EX, PIN_DEND1_EX);
    
	setAsOutput(PORT_AXON1_EX, PIN_AXON1_EX);
	setAsOutput(PORT_AXON2_EX, PIN_AXON2_EX);

    // enable external interrupts;
	nvic_enable_irq(NVIC_EXTI4_15_IRQ);
	nvic_enable_irq(NVIC_EXTI2_3_IRQ);

	nvic_set_priority(NVIC_EXTI4_15_IRQ, 0);
	nvic_set_priority(NVIC_EXTI2_3_IRQ, 0);
}

void lpuart_setup(void)
{
	// seutp lpuart interface (for communicating with NID)
	// NOTE: this ocnverts the swd interface to lpuart so debugging with swd will be disables
	rcc_periph_clock_enable(RCC_LPUART1);

	gpio_mode_setup(PORT_LPUART1_RX, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_LPUART1_RX);
	gpio_mode_setup(PORT_LPUART1_TX, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_LPUART1_TX);

	gpio_set_af(PORT_LPUART1_RX, GPIO_AF6, PIN_LPUART1_RX);
	gpio_set_af(PORT_LPUART1_TX, GPIO_AF6, PIN_LPUART1_TX);

	USART_BRR(LPUART1) = 0x1A0AA; // 38400 baud
	usart_set_databits(LPUART1, 8);  // USART_CR1_M
	usart_set_stopbits(LPUART1, USART_STOPBITS_1); //USART_CR2_STOP
	usart_set_mode(LPUART1, USART_MODE_TX_RX); //USART_CR1_RE USART_CR1_TE
	usart_set_parity(LPUART1, USART_PARITY_NONE);// USART_CR1_PS USART_CR1_PCE
	usart_set_flow_control(LPUART1, USART_FLOWCONTROL_NONE); // USART_CR3_RTSE USART_CR3_CTSE

	usart_enable(LPUART1); // USART_CR1_UE

	// enable interrupts
	nvic_enable_irq(NVIC_LPUART1_IRQ);
	usart_enable_rx_interrupt(LPUART1); // USART_CR1_RXNEIE
    USART_CR1(LPUART1) |= USART_CR1_RE;
    USART_CR1(LPUART1) |= USART_CR1_TE;
}

void lpuart1_isr(void)
{
    if ((USART_ISR(LPUART1) & USART_ISR_RXNE) != 0){
        readNID();
        USART_RQR(LPUART1) = 0b1000;
        USART_CR1(LPUART1) |= USART_CR1_RE;
    } else if ((USART_ISR(LPUART1) & USART_ISR_TXE) != 0){
        /* USART_CR1(LPUART1) |= USART_CR1_TE; */
        writeNID();
        USART_RQR(LPUART1) |= USART_RQR_TXFRQ;
    }
    USART_ICR(LPUART1) = USART_ISR(LPUART1);
}

void setAsInput(uint32_t port, uint32_t pin)
{
	
	// setup gpio as an input pin
	gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, pin);
	gpio_set_output_options(port, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, pin);

	// setup interrupt for the pin going high
	exti_select_source(pin, port);
	exti_set_trigger(pin, EXTI_TRIGGER_RISING);
	exti_enable_request(pin);
	exti_reset_request(pin);

}

void setAsOutput(uint32_t port, uint32_t pin)
{
	// disable input interrupts
	exti_disable_request(pin);

	// setup gpio as an output pin. pulldown
	gpio_mode_setup(port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, pin);
}

void exti2_3_isr(void)
{
	if ((EXTI_PR & PIN_DEND1_EX) != 0){
		ACTIVATE_INPUT(2, PIN_DEND1_EX);
		EXTI_PR |= PIN_DEND1_EX;
	}
}

void exti4_15_isr(void)
{
    if ((EXTI_PR & PIN_DEND1_IN) != 0){
		ACTIVATE_INPUT(3, PIN_DEND1_IN);
		EXTI_PR |= PIN_DEND1_IN;
	}else if ((EXTI_PR & PIN_AXON2_IN) != 0){
		ACTIVATE_INPUT(1, PIN_AXON2_IN);
		EXTI_PR |= PIN_AXON2_IN;
	}else if ((EXTI_PR & PIN_AXON1_IN) != 0){
		ACTIVATE_INPUT(0, PIN_AXON1_IN);
		EXTI_PR |= PIN_DEND1_EX;
	}
}

void tim_setup(void)
{
	/* 	Enable and reset TIM2 clock */
	rcc_periph_clock_enable(RCC_TIM2);

	/* 	Set up TIM2 mode to no clock divider ratio, edge alignment, and up direction */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*	Set prescaler to 0: 16 MHz clock */
	timer_set_prescaler(TIM2, 1);

	/* 	Set timer period to 9600: 5 kHz PWM with 9600 steps */
	timer_set_period(TIM2, 9600);

	// 	Set TIM2 Output Compare mode to PWM1 on channel 1, 2, and 3
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);

	// 	Set initial output compare values to 0
	timer_set_oc_value(TIM2, TIM_OC1, 0);
	timer_set_oc_value(TIM2, TIM_OC2, 0);
	timer_set_oc_value(TIM2, TIM_OC3, 0);

	// 	Enable outputs 
	timer_enable_oc_output(TIM2, TIM_OC1);
	timer_enable_oc_output(TIM2, TIM_OC2);
	timer_enable_oc_output(TIM2, TIM_OC3);
	
	/*	Enable counter */
	timer_enable_counter(TIM2);
	
 	  /*	TIM21 prescaler (TIMx_PSC): */
	MMIO32((TIM21_BASE) + 0x28) = 0; //prescaler = clk

}

void tim21_isr(void)
{
	/*
		TIM21 is the communication clock. 
		Each interrupt is one-bit read and one-bit write of gpios.
		Interrupts occur every 100 us.
	*/

	
    MMIO32((TIM21_BASE) + 0x10) &= ~(1<<0); //clear the interrupt register
}


void LEDFullWhite(void) 
{
	timer_set_oc_value(TIM2, TIM_OC1, 9600);
	timer_set_oc_value(TIM2, TIM_OC2, 9600);
	timer_set_oc_value(TIM2, TIM_OC3, 9600);
}

void setLED(uint16_t r, uint16_t g, uint16_t b)
{
	if (r <= 1023) 
	{
		timer_set_oc_value(TIM2, TIM_OC2, gamma_lookup[r]);
	}
	else 
	{
		timer_set_oc_value(TIM2, TIM_OC2, 9600);
	}

	if (g <= 1023) 
	{
		timer_set_oc_value(TIM2, TIM_OC1, gamma_lookup[g]);
	}
	else
	{
		timer_set_oc_value(TIM2, TIM_OC1, 9600);
	}

	if (b <= 1023)
	{
		timer_set_oc_value(TIM2, TIM_OC3, gamma_lookup[b]);
	}
	else
	{
		timer_set_oc_value(TIM2, TIM_OC3, 9600);
	}
}

void start_touch(sensor_t sensor)
{
	int i, port_res, pin_res, port_sense, pin_sense, bit_addr, tim_af;
	
	if (sensor == SENSOR0) 
	{
		port_sense = PORT_TOUCH0_SENSE;
		pin_sense = PIN_TOUCH0_SENSE;
		port_res = PORT_TOUCH0_RES;
		pin_res = PIN_TOUCH0_RES;
		bit_addr = 0;
	}
	else if (sensor == SENSOR1)
	{
		port_sense = PORT_TOUCH1_SENSE;
		pin_sense = PIN_TOUCH1_SENSE;
		port_res = PORT_TOUCH1_RES;
		pin_res = PIN_TOUCH1_RES;
		bit_addr = 1;
	}
   	 
	MMIO32((RCC_BASE) + 0x34) |= (1<<2); //Enable TIM21
	MMIO32((RCC_BASE) + 0x24) |= (1<<2); //Set reset bit, TIM21
	MMIO32((RCC_BASE) + 0x24) &= ~(1<<2); //Clear reset bit, TIM21
	
  	//	TIM21 control register 1 (TIMx_CR1):
	//	Edge-aligned (default setting)
  	//	No clock division (default setting)
  	//	Up direction (default setting)

	//	TIM21 capture/compare mode register (TIMx_CCMR1)
	MMIO32((TIM21_BASE) + 0x18) |= (1<<bit_addr); //Compare/Capture 1 selection (bit 0 for PIN_TOUCH0, bit1 for PIN_TOUCH1)
	//	IC1F left at 0000, so sampling is done at clock speed with no digital filtering
	//	CC1P/CCNP left at 00, so trigger happens on rising edge
	//	IC1PS bits left at 00, no input prescaling	
	
	MMIO32((TIM21_BASE) + 0x20) |= (1<<0); // enable capture
   	MMIO32((TIM21_BASE) + 0x00) |= (1<<0); // enable TIM21 counter

	gpio_set(port_res, pin_res); // set resistor pin high
}

int get_touch(sensor_t sensor)
{
	int val;
	val = MMIO32((TIM21_BASE) + 0x34);
	
	// set resistor low for the next cycle:
	if (sensor == SENSOR0) 
	{
		gpio_clear(PORT_TOUCH0_RES, PIN_TOUCH0_RES);
	}
	if (sensor == SENSOR1)
	{
		gpio_clear(PORT_TOUCH1_RES, PIN_TOUCH1_RES);
	}

	return val;
}

int get_slider_position(void)
/*	returns touch slider position (0-255ish) based on PCB-specific calibration settings, or -1 if no touch is detected
	note that all touch timing is handled in systick_handler and updates sensorx_time global variables */
{
	int pos;

	if ((sensor0_time < 60) || (sensor1_time < 60))
	{
		return -1;
	}

	pos = sensor0_time - sensor1_time;

	if (pos < 0)
	{
		if (pos <= -110)
		{
			return 0;
		}
		else
		{
			return (pos + 110);
		}
	}
	else
	{
		return (pos + 110);
	}
}

static const uint16_t gamma_lookup[1024] = {
	/*	Gamma = 2, input range = 0-1023, output range = 0-2047 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
    2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,
    5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,
    8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12,
   13, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 17, 17, 17, 18,
   18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24,
   25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32,
   32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40,
   41, 41, 42, 42, 43, 43, 44, 45, 45, 46, 46, 47, 48, 48, 49, 49,
   50, 51, 51, 52, 53, 53, 54, 55, 55, 56, 57, 57, 58, 59, 59, 60,
   61, 61, 62, 63, 63, 64, 65, 66, 66, 67, 68, 68, 69, 70, 71, 71,
   72, 73, 74, 74, 75, 76, 77, 77, 78, 79, 80, 81, 81, 82, 83, 84,
   85, 85, 86, 87, 88, 89, 90, 90, 91, 92, 93, 94, 95, 96, 96, 97,
   98, 99,100,101,102,103,103,104,105,106,107,108,109,110,111,112,
  113,114,115,115,116,117,118,119,120,121,122,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,142,143,144,
  145,146,147,148,149,150,151,152,153,154,156,157,158,159,160,161,
  162,163,164,166,167,168,169,170,171,173,174,175,176,177,178,180,
  181,182,183,184,186,187,188,189,190,192,193,194,195,197,198,199,
  200,202,203,204,205,207,208,209,210,212,213,214,216,217,218,220,
  221,222,223,225,226,227,229,230,231,233,234,236,237,238,240,241,
  242,244,245,247,248,249,251,252,253,255,256,258,259,261,262,263,
  265,266,268,269,271,272,274,275,277,278,279,281,282,284,285,287,
  288,290,291,293,294,296,298,299,301,302,304,305,307,308,310,311,
  313,315,316,318,319,321,322,324,326,327,329,330,332,334,335,337,
  338,340,342,343,345,347,348,350,352,353,355,357,358,360,362,363,
  365,367,368,370,372,374,375,377,379,380,382,384,386,387,389,391,
  393,394,396,398,400,401,403,405,407,409,410,412,414,416,417,419,
  421,423,425,427,428,430,432,434,436,438,439,441,443,445,447,449,
  451,453,454,456,458,460,462,464,466,468,470,472,473,475,477,479,
  481,483,485,487,489,491,493,495,497,499,501,503,505,507,509,511,
  513,515,517,519,521,523,525,527,529,531,533,535,537,539,541,543,
  545,547,549,552,554,556,558,560,562,564,566,568,570,572,575,577,
  579,581,583,585,587,590,592,594,596,598,600,602,605,607,609,611,
  613,616,618,620,622,624,627,629,631,633,636,638,640,642,644,647,
  649,651,653,656,658,660,663,665,667,669,672,674,676,679,681,683,
  686,688,690,692,695,697,699,702,704,707,709,711,714,716,718,721,
  723,725,728,730,733,735,737,740,742,745,747,749,752,754,757,759,
  762,764,767,769,771,774,776,779,781,784,786,789,791,794,796,799,
  801,804,806,809,811,814,816,819,821,824,826,829,831,834,837,839,
  842,844,847,849,852,855,857,860,862,865,868,870,873,875,878,881,
  883,886,889,891,894,896,899,902,904,907,910,912,915,918,920,923,
  926,929,931,934,937,939,942,945,948,950,953,956,958,961,964,967,
  969,972,975,978,980,983,986,989,992,994,997,1000,1003,1006,1008,1011,
  1014,1017,1020,1022,1025,1028,1031,1034,1037,1039,1042,1045,1048,1051,1054,1057,
  1060,1062,1065,1068,1071,1074,1077,1080,1083,1086,1089,1091,1094,1097,1100,1103,
  1106,1109,1112,1115,1118,1121,1124,1127,1130,1133,1136,1139,1142,1145,1148,1151,
  1154,1157,1160,1163,1166,1169,1172,1175,1178,1181,1184,1187,1190,1193,1196,1199,
  1202,1205,1208,1211,1215,1218,1221,1224,1227,1230,1233,1236,1239,1242,1246,1249,
  1252,1255,1258,1261,1264,1268,1271,1274,1277,1280,1283,1286,1290,1293,1296,1299,
  1302,1306,1309,1312,1315,1318,1322,1325,1328,1331,1335,1338,1341,1344,1347,1351,
  1354,1357,1361,1364,1367,1370,1374,1377,1380,1383,1387,1390,1393,1397,1400,1403,
  1407,1410,1413,1417,1420,1423,1427,1430,1433,1437,1440,1443,1447,1450,1453,1457,
  1460,1464,1467,1470,1474,1477,1480,1484,1487,1491,1494,1498,1501,1504,1508,1511,
  1515,1518,1522,1525,1529,1532,1535,1539,1542,1546,1549,1553,1556,1560,1563,1567,
  1570,1574,1577,1581,1584,1588,1591,1595,1598,1602,1606,1609,1613,1616,1620,1623,
  1627,1630,1634,1638,1641,1645,1648,1652,1656,1659,1663,1666,1670,1674,1677,1681,
  1684,1688,1692,1695,1699,1703,1706,1710,1714,1717,1721,1725,1728,1732,1736,1739,
  1743,1747,1750,1754,1758,1762,1765,1769,1773,1776,1780,1784,1788,1791,1795,1799,
  1803,1806,1810,1814,1818,1821,1825,1829,1833,1837,1840,1844,1848,1852,1856,1859,
  1863,1867,1871,1875,1879,1882,1886,1890,1894,1898,1902,1905,1909,1913,1917,1921,
  1925,1929,1933,1936,1940,1944,1948,1952,1956,1960,1964,1968,1972,1976,1980,1983,
  1987,1991,1995,1999,2003,2007,2011,2015,2019,2023,2027,2031,2035,2039,2043,2047 };
