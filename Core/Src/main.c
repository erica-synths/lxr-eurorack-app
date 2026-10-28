/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "main.h"
#include "fatfs.h"

#include "lcd.h"
#include "menu/menu.h"
#include "IO/DIO.h"
#include "menu/screensaver.h"
#include "ledHandler.h"
#include "buttonHandler.h"
#include "encoder.h"
#include "IO/adcPots.h"
#include "Preset/PresetManager.h"
#include "Soundengine/Soundengine.h"
#include "Soundengine/DSPAudio/random.h"
#include <Soundengine/ModMatrix_Processor.h>
#include "Soundengine/MIDI/Uart.h"

// The CMake build passes FIRMWARE_VERSION from project(VERSION); this is the fallback.
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.51"
#endif
//#define f_unmount(path) f_mount(0, path, 0)

//---- peripheral handles, shared with stm32f7xx_hal_msp.c, stm32f7xx_it.c and the drivers ----
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi3_tx;

RNG_HandleTypeDef hrng;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim14;

float debugWatch = 0;

void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void init_gpio(void);
static void init_dma(void);
static void init_adc1(void);
static void init_i2s(I2S_HandleTypeDef *hi2s, SPI_TypeDef *instance);
static void init_rng(void);
static void init_tim14(void);
static void init_tim2(void);
static void init_sdmmc1(void);

void sd_formatRoutine()
{

	lcd_process_display_interrupt = 0;
	//menu to ask user to format card
	lcd_clear();
	lcd_home();
	lcd_string("Format card? All");
	//goto 2nd line
	lcd_setcursor(0,2);
	lcd_string("data erased?");

	while(encode_readButton()==0);
	while(encode_readButton()==1);

	lcd_clear();
	lcd_home();
	lcd_string("Really Format");
	//goto 2nd line
	lcd_setcursor(0,2);
	lcd_string("card?");

	while(encode_readButton()==0);

	//make fs
	FRESULT res;
	FATFS fs;           /* Filesystem object */
	BYTE work[512]; /* Work area (larger is better for processing time) */


	lcd_clear();
	lcd_home();
	lcd_string("Formatting card");
	//goto 2nd line
	lcd_setcursor(0,2);
	lcd_string("...");

	res = f_mkfs("", FM_FAT32, 0, work, 512);

	if(res!=FR_OK)
	{
		//error, can't format card
		lcd_clear();
		lcd_home();
		lcd_string("Error-Please try");
		//goto 2nd line
		lcd_setcursor(0,2);
		lcd_string("another card!");
		while(1);
	}
	else
	{
		/* Gives a work area to the default drive */
		f_mount(&fs, "", 0);

		//create glo.cfg file
		preset_createGlobals();

		//load init sound
		preset_init(0);
		//save init sound to card

		sprintf(savefileNames[0][PROJECT_NAMES], "Init");
		//preset_saveDrumsetSD(0);

		//success
		lcd_clear();
		lcd_home();
		lcd_string("Success!");
		//goto 2nd line
		lcd_setcursor(0,2);
		lcd_string("Please reboot.");

		while(1);
	}
}

__attribute__((section(".itcm_text"))) float foo(float a)
{
	return ((a + 105)*a + 945) / ((15*a + 420)*a + 945);
}

int main(void)
{
	/* Load functions into ITCM RAM */
	extern const unsigned char _itcm_text_start;
	extern const unsigned char _itcm_text_end;
	extern const unsigned char _itcm_data;
	memcpy((void*)&_itcm_text_start, &_itcm_data, (int) (&_itcm_text_end - &_itcm_text_start));

	float foobar = 0;
	for(int i=0;i<1024;i++)
	{
		foobar += foo(1.8*i);
	}

  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();

  /*
   * In the original code, the systick_timer was changed to 0.25ms
   * No idea how and if HALL allows to change the systick frequency.
   * seems limited to 1ms with HAL_GetTick()
   *
   * So TIM14 is set to a 0.24ms interrupt to increase the old systick_timer variable here.
   */

  //Spread Spectrum Clock Generation
  //RCC_SSCGR |= INCSTEP
  /* Enable/Configure Spread Spectrum, when PLL isn't running */
  //TODO: check if enabled and working!
    uint16_t INCSTEP = 56;
    uint16_t MODPER = 500;
/*
    RCC->SSCGR = (1 << 31) | // Enable
    (0 << 30) | // Center
    (INCSTEP << 13) |
    (MODPER << 0);
    */

    //has to be written before PLL is activated
    //enabled, center,
    RCC->SSCGR = RCC_SSCGR_SSCGEN | (0 << RCC_SSCGR_SPREADSEL_Pos) |  (INCSTEP << RCC_SSCGR_INCSTEP_Pos) | (MODPER << RCC_SSCGR_MODPER_Pos) ;

  //--------------------------------

  SystemClock_Config();
  PeriphCommonClock_Config();

  DWT_Delay_Init();
  //SysTick_Config(SystemCoreClock / 1000);
  //HAL_InitTick(TICK_INT_PRIORITY);

  init_gpio();
  init_dma();		// before the peripherals that use DMA
  init_adc1();
  init_i2s(&hi2s2, SPI2);
  init_i2s(&hi2s3, SPI3);
  MX_FATFS_Init();
  init_rng();
  init_tim14();
  init_tim2();
  init_sdmmc1();
  dio_init();
  CV_Init();





  lcd_init();
  lcd_clear();
  lcd_home();





  menu_init();
//HAL_ADC_Start(&hadc1);




  //we have to read out the pots once to have a defined state before loading the preset data
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, ADC_BUF_LEN);
  adc_checkPots();
  adc_checkPots();

  CV_Init();

  MX_FATFS_ReInit();

	//print boot up message
lcd_clear();
lcd_string("Sonic Potions");
//goto 2nd line
lcd_setcursor(0,2);
lcd_string("LXR2 Drums V");
lcd_string(FIRMWARE_VERSION);

lcd_showSplash();


uint8_t sd_card_ok = 0;
//---- mount sd card ----  //todo move to preset manager or other matching .c file
//FatFs[0] = 0;
FRESULT res = f_mount((FATFS*)&SDFatFS, (TCHAR const*)SDPath,1);
if( res != FR_OK)
{

	lcd_home();

	//uint8_t error = 1;
	switch(res)
	{
		// There is no valid FAT volume on the drive or wrong lower layer implementation.
		case FR_NO_FILESYSTEM:
			lcd_clear();
			lcd_string("SD error:");
			//goto 2nd line
			lcd_setcursor(0,2);
			lcd_string("No FS found!");
			HAL_Delay(1000);
			sd_formatRoutine();
			break;

		// The lower layer, disk_read, disk_write or disk_ioctl function, reported that an unrecoverable hard error occured.
		case FR_DISK_ERR:
			lcd_clear();
			lcd_string("SD error:");
			//goto 2nd line
			lcd_setcursor(0,2);
			lcd_string("Disk error!");
			HAL_Delay(1000);
			break;

		/*The lower layer, disk_initialize function, reported that the storage device could not be got ready to work. One of the following possibilities is suspected.

		No medium in the drive.
		Wrong lower layer implementation.
		Wrong hardware configuration.
		The storage device has broken.
		*/
		case FR_NOT_READY:
			lcd_clear();
			lcd_string("SD error:");
			//goto 2nd line
			lcd_setcursor(0,2);
			lcd_string("No Card!");
			HAL_Delay(1000);
			break;



		default:
			lcd_clear();
			lcd_string("SD error:");
			//goto 2nd line
			lcd_setcursor(0,2);
			lcd_string("unknown!");
			HAL_Delay(1000);
			break;

	}



}
else
{
	sd_card_ok = 1;
}

HAL_Delay(1000);
lcd_clear();

adc_checkPots();
adc_checkPots();

//-----------------------

sprintf(currentFolder, "/proj00");
if(sd_card_ok)
{




	FRESULT res;
	FILINFO fno;

	res = f_stat(currentFolder, &fno);
	if(res == FR_NO_FILE)
	{
		res = f_mkdir(currentFolder);
		if(res == FR_OK)
		{
			f_chdir(currentFolder);
		}
	}
	else
	{
		f_chdir(currentFolder);
	}
}

	soundengine_init();

	if(sd_card_ok)
	{
		if(!preset_loadGlobals())
		{
			lcd_clear();
			lcd_home();
			lcd_string(PSTR("glo.cfg error"));
			lcd_setcursor(0,2);
			lcd_string("Using defaults.");
			preset_createGlobals();
			HAL_Delay(1000);

		}
	}

	//load init preset
	if(sd_card_ok)
	{



		f_chdir(currentFolder);

		//load all 64 kits from sd card
		lcd_clear();
		lcd_home();
		lcd_string(PSTR("Loading kits"));
		lcd_setcursor(0,2);
		lcd_string("from SD card");

		for(uint8_t i=0;i<NUM_KIT_SLOTS;i++)
		{
			preset_loadDrumsetSD(i);
		}

		//init morph target to peset 0
		//morphParameters = mainParameters[0];
		activeParameters[PAR_MORPH_TARGET] = 0;
	}

if(sd_card_ok)
{
	//return to project 0
	f_chdir("/proj00");

	uint8_t lastUsedPreset;
	preset_loadLastUsedPresetNrFromSD(&lastUsedPreset);
	preset_loadDrumsetRam(lastUsedPreset,0, 1);
	menu_selectedPresetNr[SAVE_TYPE_KIT] = lastUsedPreset;
}
else
{
  preset_init(0);
  preset_loadDefaultGlobals();
}

//f_unmount((TCHAR const*)SDPath);

  HAL_TIM_Base_Start_IT(&htim14); 	//our own own systick - prio 0 - higher prio comes first
#if CALC_SAMPLES_IN_INTERRUPT
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ

#else
  HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
#endif
#endif
 // HAL_TIM_Base_Start_IT(&htim13);


uint8_t lastSdState = BSP_SD_IsDetected();
buttonHandler_init(); //to make sure all leds are properly lit after boot

//make sure to have the tmp folder active
preset_initTmpFolder();
//----- load tmp stuff --------
//manually saved files
//loadTmpFolderFiles();
//autosave kit
preset_LoadTmpKit();
//autosave pattern
//preset_LoadTmpPat();


preset_resetAllSaveTimers();
//-----------------------------


lcd_process_display_interrupt = 1;


//start voice control
voiceControl_start();


  while (1)
  {

	  //check SD card
	  if((BSP_SD_IsDetected() == SD_PRESENT) &&  (lastSdState == SD_NOT_PRESENT))
	  {
			MX_FATFS_ReInit();
			f_mount(0, (TCHAR const*)SDPath, 0); //un-mount
			memset((FATFS*)&SDFatFS,0,sizeof(FATFS)); //clear FS object to force reinit
			res = f_mount((FATFS*)&SDFatFS, (TCHAR const*)SDPath,1);
			if( res != FR_OK)
			{
				menu_showSdError( res);
			}
	  }
	  lastSdState = BSP_SD_IsDetected();

//	  soundengine_process();
	  //TODO clean up this multi call mess!!!!

	//  menu_tick();
	  CV_process();
	  soundengine_process();
	  adc_checkPots();
	  soundengine_process();
	//  menu_tick();
	  //buttonHandler_tick();
	  adc_checkPots();
	  soundengine_process();
	//  menu_tick();

	  //get the encoder values
	  menu_parseEncoder(encode_read4(),encode_readButton());
	  soundengine_process();
	//  menu_tick();
	  led_tickHandler();
	  soundengine_process();
	//  menu_tick();
	  adc_checkPots();
	  soundengine_process();
	//  menu_tick();
	  screensaver_check();
	  soundengine_process();
	//  menu_tick();

	  //update the menu according to the encoder values
	  dio_process();
	  soundengine_process();
	//  menu_tick();
	  adc_checkPots();
	//  menu_tick();
	  soundengine_process();
	//  menu_tick();
	//  adc_checkFaders();
	  adc_checkCv();
	  soundengine_process();
	//  menu_tick();

	  //check if we have to save something
	  preset_processTimer();
  }
}

//---- clocks ----

/*
 * 16 MHz HSE -> PLL (/8, *216, /2) = 216 MHz SYSCLK, which needs over-drive mode.
 * PLLQ (/9) gives the 48 MHz clock that SDMMC1 runs from.
 * AHB 216 MHz, APB1 54 MHz (its timers 108 MHz), APB2 108 MHz.
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef osc = {0};
	RCC_ClkInitTypeDef clk = {0};

	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc.HSEState = RCC_HSE_ON;
	osc.PLL.PLLState = RCC_PLL_ON;
	osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc.PLL.PLLM = 8;
	osc.PLL.PLLN = 216;
	osc.PLL.PLLP = RCC_PLLP_DIV2;
	osc.PLL.PLLQ = 9;
	osc.PLL.PLLR = 2;
	if(HAL_RCC_OscConfig(&osc) != HAL_OK)
	{
		Error_Handler();
	}

	if(HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		Error_Handler();
	}

	clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk.APB1CLKDivider = RCC_HCLK_DIV4;
	clk.APB2CLKDivider = RCC_HCLK_DIV2;
	if(HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_7) != HAL_OK)
	{
		Error_Handler();
	}
}

/*
 * I2S clock from PLLI2S: 2 MHz (HSE / PLLM) * 113 / 5 = 45.2 MHz, which gives the
 * 44.1 kHz audio sample rate. Only the R output is used; P and Q just need valid values.
 */
void PeriphCommonClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef periph = {0};

	periph.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	periph.PLLI2S.PLLI2SN = 113;
	periph.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
	periph.PLLI2S.PLLI2SR = 5;
	periph.PLLI2S.PLLI2SQ = 2;
	periph.PLLI2SDivQ = 1;
	periph.I2sClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
	if(HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK)
	{
		Error_Handler();
	}
}

//---- peripherals ----
// Pins, DMA streams and peripheral clocks are set up in stm32f7xx_hal_msp.c.

/*
 * ADC1 scans the pots and CV inputs continuously, and DMA copies the results to
 * adc_buf (IO/adcPots.c) in the order of this table. The CV inputs get a longer
 * sampling time.
 */
static const struct
{
	uint32_t channel;
	uint32_t samplingTime;
} adc1_scan[ADC_BUF_LEN] =
{
	{ADC_CHANNEL_8,  ADC_SAMPLETIME_3CYCLES},	// PB0 pot 1B
	{ADC_CHANNEL_9,  ADC_SAMPLETIME_3CYCLES},	// PB1 pot 1A
	{ADC_CHANNEL_14, ADC_SAMPLETIME_3CYCLES},	// PC4 pot 2B
	{ADC_CHANNEL_15, ADC_SAMPLETIME_3CYCLES},	// PC5 pot 2A
	{ADC_CHANNEL_12, ADC_SAMPLETIME_3CYCLES},	// PC2 pot 3B
	{ADC_CHANNEL_13, ADC_SAMPLETIME_3CYCLES},	// PC3 pot 3A
	{ADC_CHANNEL_10, ADC_SAMPLETIME_3CYCLES},	// PC0 pot 4B
	{ADC_CHANNEL_11, ADC_SAMPLETIME_3CYCLES},	// PC1 pot 4A
	{ADC_CHANNEL_4,  ADC_SAMPLETIME_28CYCLES},	// PA4 CV 1
	{ADC_CHANNEL_3,  ADC_SAMPLETIME_28CYCLES},	// PA3 CV 2
	{ADC_CHANNEL_2,  ADC_SAMPLETIME_28CYCLES},	// PA2 CV 3
	{ADC_CHANNEL_1,  ADC_SAMPLETIME_28CYCLES},	// PA1 CV 4
	{ADC_CHANNEL_0,  ADC_SAMPLETIME_28CYCLES},	// PA0 CV 5
};

static void init_adc1(void)
{
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_LEFT;
	hadc1.Init.NbrOfConversion = ADC_BUF_LEN;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if(HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	ADC_ChannelConfTypeDef channel = {0};
	for(uint32_t i=0; i<ADC_BUF_LEN; i++)
	{
		channel.Channel = adc1_scan[i].channel;
		channel.Rank = i + 1;	// ADC_REGULAR_RANK_n is n
		channel.SamplingTime = adc1_scan[i].samplingTime;
		if(HAL_ADC_ConfigChannel(&hadc1, &channel) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

// I2S2 and I2S3 feed the audio codecs: master transmit, Philips standard, 16 bit, 44.1 kHz, with MCLK.
static void init_i2s(I2S_HandleTypeDef *hi2s, SPI_TypeDef *instance)
{
	hi2s->Instance = instance;
	hi2s->Init.Mode = I2S_MODE_MASTER_TX;
	hi2s->Init.Standard = I2S_STANDARD_PHILIPS;
	hi2s->Init.DataFormat = I2S_DATAFORMAT_16B;
	hi2s->Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	hi2s->Init.AudioFreq = I2S_AUDIOFREQ_44K;
	hi2s->Init.CPOL = I2S_CPOL_HIGH;
	hi2s->Init.ClockSource = I2S_CLOCK_PLL;
	if(HAL_I2S_Init(hi2s) != HAL_OK)
	{
		Error_Handler();
	}
}

static void init_rng(void)
{
	hrng.Instance = RNG;
	if(HAL_RNG_Init(&hrng) != HAL_OK)
	{
		Error_Handler();
	}
}

// Only sets the parameters: the SD card driver (FATFS/Target/bsp_driver_sd.c) calls HAL_SD_Init().
static void init_sdmmc1(void)
{
	hsd1.Instance = SDMMC1;
	hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
	hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
	hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_ENABLE;
	hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
	hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
	hsd1.Init.ClockDiv = 1;
}

// TIM2 drives the sample calculation interrupt (CALC_SAMPLES_IN_INTERRUPT); it is started and stopped where needed.
static void init_tim2(void)
{
	TIM_ClockConfigTypeDef clockSource = {0};
	TIM_MasterConfigTypeDef master = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1000;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}

	clockSource.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if(HAL_TIM_ConfigClockSource(&htim2, &clockSource) != HAL_OK)
	{
		Error_Handler();
	}

	master.MasterOutputTrigger = TIM_TRGO_RESET;
	master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if(HAL_TIMEx_MasterConfigSynchronization(&htim2, &master) != HAL_OK)
	{
		Error_Handler();
	}
}

// TIM14 is the firmware's own 0.25 ms tick (systick_ticks and menu_tick() in stm32f7xx_it.c):
// 108 MHz / (13500 + 1) / (1 + 1) = 4 kHz.
static void init_tim14(void)
{
	TIM_OC_InitTypeDef oc = {0};

	htim14.Instance = TIM14;
	htim14.Init.Prescaler = 13500;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 1;
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_Base_Init(&htim14) != HAL_OK)
	{
		Error_Handler();
	}
	if(HAL_TIM_OC_Init(&htim14) != HAL_OK)
	{
		Error_Handler();
	}

	oc.OCMode = TIM_OCMODE_TOGGLE;
	oc.Pulse = 0;
	oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc.OCFastMode = TIM_OCFAST_DISABLE;
	if(HAL_TIM_OC_ConfigChannel(&htim14, &oc, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
}

// DMA controller clocks and stream interrupts. The streams themselves are set up in stm32f7xx_hal_msp.c.
static void init_dma(void)
{
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);	// I2S2 TX
	HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);	// I2S3 TX
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 3, 0);	// ADC1
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);	// SDMMC1 RX
	HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);	// SDMMC1 TX
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

static void gpio_config(GPIO_TypeDef *port, uint32_t pins, uint32_t mode, uint32_t pull, uint32_t speed)
{
	GPIO_InitTypeDef init = {0};
	init.Pin = pins;
	init.Mode = mode;
	init.Pull = pull;
	init.Speed = speed;
	HAL_GPIO_Init(port, &init);
}

// Buttons, LEDs, encoder, LCD, SD card detect, accent and trigger inputs. Pin names are in main.h.
static void init_gpio(void)
{
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// all outputs start low
	HAL_GPIO_WritePin(GPIOE, LED_3_Pin|LED_1_Pin|LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin|LCD_E_Pin|LCD_RS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, LED_9_Pin|LED_5_Pin|LED_12_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, LED_6_Pin|LED_7_Pin|LED_8_Pin|LED_13_Pin|LED_14_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LED_2_Pin|LED_4_Pin|LED_11_Pin|LED_10_Pin, GPIO_PIN_RESET);

	// port A
	gpio_config(GPIOA, LED_6_Pin|LED_7_Pin|LED_8_Pin|LED_13_Pin|LED_14_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
	gpio_config(GPIOA, Button_13_Pin|Button_14_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

	// port B
	gpio_config(GPIOB, LED_2_Pin|LED_4_Pin|LED_11_Pin|LED_10_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
	gpio_config(GPIOB, Button_4_Pin|Button_2_Pin|Button_12_Pin|Button_5_Pin|Button_3_Pin|Button_7_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

	// port C
	gpio_config(GPIOC, LED_9_Pin|LED_5_Pin|LED_12_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
	gpio_config(Button_1_GPIO_Port, Button_1_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

	// port D: SD card, accent inputs and the trigger inputs, which interrupt on the falling edge
	gpio_config(SDIO_CS_GPIO_Port, SDIO_CS_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
	gpio_config(GPIOD, Accent_1_Pin|SD_CD_Pin|Accent_5_Pin|Accent_3_Pin|Accent_4_Pin|Accent_2_Pin|Accent_6_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW);
	gpio_config(GPIOD, EXTI9_Trigger_7_Pin|EXT10I_Trigger_3_Pin|EXT11I_Trigger_2_Pin|EXT12I_Trigger_6_Pin|EXT13I_Trigger_5_Pin|EXTI14_Trigger_4_Pin|EXTI15_Trigger_1_Pin, GPIO_MODE_IT_FALLING, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW);

	// port E: LCD, encoder, LEDs and buttons
	gpio_config(GPIOE, LED_3_Pin|LED_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
	gpio_config(GPIOE, LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin|LCD_E_Pin|LCD_RS_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
	gpio_config(GPIOE, Button_6_Pin|Button_11_Pin|Button_10_Pin|Enc_A_Pin|Enc_B_Pin|Button_9_Pin|Button_8_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
	gpio_config(Enc_Switch_GPIO_Port, Enc_Switch_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW);

	// trigger input interrupts: trigger 7 on EXTI9_5, triggers 1-6 on EXTI15_10
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// Called when one of the HAL init calls above fails. The error is ignored and start-up continues.
void Error_Handler(void)
{
}

#ifdef USE_FULL_ASSERT
// Called by assert_param() in the HAL drivers, only when USE_FULL_ASSERT is defined.
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
