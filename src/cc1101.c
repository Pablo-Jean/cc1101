/*
 * cc1101.c (former name rf_driver.c)
 *
 *  Created on: 29 mar. 2019
 *      Author: dsoldevila
 *      The following code was made by SpaceTeddy (https://github.com/SpaceTeddy/CC1101):
 *      	Patable presets
 *      	Modulation presets
 *      	rf_set_modulation_mode(mode);
			rf_set_ISMband(ism_band);
			rf_set_output_power_level(-30);
			wor_enable()
			Some enter x state functions (but they are so simple that it doesn't really matter)
		The following code is from EMCU.EU (http://www.emcu.eu/how-to-implement-printf-for-send-message-via-usb-on-stm32-nucleo-boards-using-atollic)
			_io_putchar()

		TODO NOTE: THE MODULATION PRESET GFSK_1_2KB HAS THE RIGHT VALUES. THE OTHER PRESETS MAY REQUIRE SOME MODIFICATION TO WORK.
		ONLY THE GFSK_1_2KB MODULATION HAS BEEN TESTED, DO NOT EXPECT OTHERS TO WORK.
 *
 */
//#define RF_DRIVER_H_
#include "cc1101.h"

/* Private defines -------------------------------------*/
#ifndef TRUE
#define TRUE 		true
#endif

#ifndef FALSE
#define FALSE 		false
#endif

#define _CS_ASSERT_LEVEL				FALSE
#define _CS_DEASSERT_LEVEL				TRUE

/* Private variables -------------------------------------------------------------*/



//----------------------[PATABLES]---------------------------------------------
/*PATABLES Registers presets for various frequencies. This values are the (suposed) optimal values for -30, -20, -15,
-10, 0, 5, 7, 10 dBm for each carrier frequency.
*/
const uint8_t patable_power_315[]  = {0x17,0x1D,0x26,0x69,0x51,0x86,0xCC,0xC3};
const uint8_t patable_power_433[]  = {0x6C,0x1C,0x06,0x3A,0x51,0x85,0xC8,0xC0};
const uint8_t patable_power_868[]  = {0x03,0x17,0x1D,0x26,0x50,0x86,0xCD,0xC0};
const uint8_t patable_power_915[]  = {0x0B,0x1B,0x6D,0x67,0x50,0x85,0xC9,0xC1};


//----------------------[REGISTER BASIC CONFIGURATION]------------------------
//Preset for Gaussian Frequency Shift Keying mod at 1.2KBits/s
const uint8_t cc1100_GFSK_1_2_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
                    0xD8,  // PKTCTRL1      Packet Automation Control //TODO changed from DC to disable lqi and rssi appending
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x08,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0xF5,  // MDMCFG4       Modem Configuration
                    0x83,  // MDMCFG3       Modem Configuration
                    0x13,  // MDMCFG2       Modem Configuration
                    0xC0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x15,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x00,  // MCSM1         Main Radio Control State Machine Configuration //TODO was 0x0C
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x16,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x6C,  // BSCFG         Bit Synchronization Configuration
                    0x03,  // AGCCTRL2      AGC Control
                    0x40,  // AGCCTRL1      AGC Control
                    0x91,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0x56,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xA9,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_GFSK_38_4_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x06,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0xCA,  // MDMCFG4       Modem Configuration
                    0x83,  // MDMCFG3       Modem Configuration
                    0x13,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x34,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x16,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x6C,  // BSCFG         Bit Synchronization Configuration
                    0x43,  // AGCCTRL2      AGC Control
                    0x40,  // AGCCTRL1      AGC Control
                    0x91,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0x56,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xA9,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_GFSK_100_kb[]  = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x08,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x5B,  // MDMCFG4       Modem Configuration
                    0xF8,  // MDMCFG3       Modem Configuration
                    0x13,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x47,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x1C,  // BSCFG         Bit Synchronization Configuration
                    0xC7,  // AGCCTRL2      AGC Control
                    0x00,  // AGCCTRL1      AGC Control
                    0xB2,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0xB6,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_MSK_250_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x0B,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x2D,  // MDMCFG4       Modem Configuration
                    0x3B,  // MDMCFG3       Modem Configuration
                    0x73,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x00,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x1C,  // BSCFG         Bit Synchronization Configuration
                    0xC7,  // AGCCTRL2      AGC Control
                    0x00,  // AGCCTRL1      AGC Control
                    0xB2,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0xB6,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_MSK_500_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x0C,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x0E,  // MDMCFG4       Modem Configuration
                    0x3B,  // MDMCFG3       Modem Configuration
                    0x73,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x00,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x1C,  // BSCFG         Bit Synchronization Configuration
                    0xC7,  // AGCCTRL2      AGC Control
                    0x40,  // AGCCTRL1      AGC Control
                    0xB2,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0xB6,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x19,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_OOK_4_8_kb[] = { //In fact it's 2.4Kb/s because of the Manhattan codification, see Datasheet.
                    0x02,  // IOCFG2        GDO2 Output Pin Configuration //0x06 --> 0x02 (deasserts when below threshold)
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x06,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x48,  // FIFOTHR       RX FIFO and TX FIFO Thresholds //0x47 --> 0x48
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0xFF,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x05,  // PKTCTRL0      Packet Automation Control
                    0x00,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x06,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x87,  // MDMCFG4       Modem Configuration
                    0x83,  // MDMCFG3       Modem Configuration
                    0x3B,  // MDMCFG2       Modem Configuration
                    0x22,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x15,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x30,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x14,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x6C,  // BSCFG         Bit Synchronization Configuration
                    0x07,  // AGCCTRL2      AGC Control
                    0x00,  // AGCCTRL1      AGC Control
                    0x92,  // AGCCTRL0      AGC Control
                    0x87,  // WOREVT1       High Byte Event0 Timeout
                    0x6B,  // WOREVT0       Low Byte Event0 Timeout
                    0xFB,  // WORCTRL       Wake On Radio Control
                    0x56,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xE9,  // FSCAL3        Frequency Synthesizer Calibration
                    0x2A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x1F,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x35,  // TEST1         Various Test Settings
                    0x09,  // TEST0         Various Test Settings
};
//----------------------[END REGISTER BASIC CONFIGURATION]--------------------

/* Private user code ---------------------------------------------------------*/

/* RF DRIVER ----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------[CC1101 Init and Settings]------------------------------*/
uint8_t cc1101_init(cc1101_t *cc1101, cc1101_params_t *params){
	/**
	 * @brief Calls all the functions needed to make the RF chip operative. This should be the first function used when
	 * using the RF chip.
	 *
	 */

	//Pinout linking
	assert(cc1101 != NULL);
	assert(params != NULL);
	assert(params->eIsmBand <= ISMBAND_MHz915);
	assert(params->eModulation <= MOD_OOK_4_8_kb);
	assert(params->fxCsGpio != NULL);
	assert(params->fxDelayUs != NULL);
	assert(params->fxSpiRead != NULL);
	assert(params->fxSpiWrite != NULL);
	assert(params->u32XtalFreqMhz >= 26 && params->u32XtalFreqMhz <= 27);

	// Initialize variables
	cc1101->timeout = MS_TO_US(30); // 30 seconds of timeout
	cc1101->gdo0_flag = false;
	cc1101->eIsmBand = params->eIsmBand;
	cc1101->eModulation = params->eModulation;
	cc1101->u32XtalFreqMhz = params->u32XtalFreqMhz;
	cc1101->fxCsGpio = params->fxCsGpio;
	cc1101->fxDelayUs = params->fxDelayUs;
	cc1101->fxSpiRead = params->fxSpiRead;
	cc1101->fxSpiWrite = params->fxSpiWrite;

	//Turn on the chip
	cc1101_reset(cc1101);

	//Check that the SPI works
	if(!cc1101_check(cc1101)){
 		return FALSE;
	}


	cc1101_write_strobe(cc1101, SFTX); //Flush TX FIFO
	cc1101->fxDelayUs(MS_TO_US(1));
	cc1101_write_strobe(cc1101, SFRX); //Flush RX FIFO
	cc1101->fxDelayUs(MS_TO_US(1));

	cc1101_set_modulation_mode(cc1101, cc1101->eModulation);

	cc1101_set_ISMband(cc1101, cc1101->eIsmBand);
	cc1101_set_channel(cc1101, 0);
	cc1101_set_output_power_level(cc1101, -30);

	return TRUE;
}

void cc1101_reset(cc1101_t *cc1101){
	/**
	 * @brief Turns on the RF chip with a specific sequence on the CS pin and a SRES command.
	 * The former is only needed on a cold start.
	 */

	cc1101->fxCsGpio(_CS_ASSERT_LEVEL);
	cc1101->fxDelayUs(10);

	cc1101->fxCsGpio(_CS_DEASSERT_LEVEL);
	cc1101->fxDelayUs(40);

	cc1101_write_strobe(cc1101, SRES);
	cc1101->fxDelayUs(MS_TO_US(1));
}

uint8_t cc1101_check(cc1101_t *cc1101){
	/**
	 * @brief Checks the version of the RF chip to check if SPI is OK. It checks 10 times to make sure wires are really OK.
	 */

	uint8_t ok = TRUE;
	uint8_t i;
	uint8_t version;
	for(i=0; i<10; i++){
		version = cc1101_read_register(cc1101, VERSION);
		if(version!=0x14)
			ok = FALSE;
	}

	return ok;

}

void cc1101_set_modulation_mode(cc1101_t *cc1101, cc1101_modulation_e mode){
	/*
	 * @brief Loads the wanted modulation preset to the CC1101.
	 */

    const uint8_t* cfg_reg;

    switch (mode)
    {
        case MOD_GFSK_1_2_kb:
        			cfg_reg = cc1100_GFSK_1_2_kb;
                    break;
        case MOD_GFSK_38_4_kb:
                    cfg_reg = cc1100_GFSK_38_4_kb;
                    break;
        case MOD_GFSK_100_kb:
        			cfg_reg = cc1100_GFSK_100_kb;
                    break;
        case MOD_MSK_250_kb:
        			cfg_reg = cc1100_MSK_250_kb;
                    break;
        case MOD_MSK_500_kb:
        			cfg_reg = cc1100_MSK_500_kb;
                    break;
        case MOD_OOK_4_8_kb:
        			cfg_reg = cc1100_OOK_4_8_kb;
                    break;
        default:
        			cfg_reg = cc1100_GFSK_38_4_kb;
                    break;
    }

    cc1101_write_data(cc1101, WRITE_BURST(0), (uint8_t*)cfg_reg, CFG_REGISTER);                            //writes all 47 config register


}

//(Semi)DEPRECATED
void cc1101_set_ISMband(cc1101_t *cc1101, cc1101_ismband_e band){
	/*
	 * Deprecated by rf_set_frequency(float), although the second still doesn't configure the PATABLES registers, so it is still needed.
	 */
    uint8_t freq2, freq1, freq0;
    const uint8_t* patable;

    switch (band)
    {
        case ISMBAND_MHz315:
                    freq2=0x0C;
                    freq1=0x1D;
                    freq0=0x89;
                    patable = patable_power_315;
                    break;
        case ISMBAND_MHz434:                                                          //433.92MHz
                    freq2=0x10;
                    freq1=0xB0;
                    freq0=0x71;
                    patable = patable_power_433;
                    break;
        case ISMBAND_MHz868:                                                          //868.3MHz
                    freq2=0x21;
                    freq1=0x65;
                    freq0=0x6A;
                    patable = patable_power_868;
                    break;
        case ISMBAND_MHz915:
                    freq2=0x23;
                    freq1=0x31;
                    freq0=0x3B;
                    patable = patable_power_915;
                    break;
        default:                                                          //868.3MHz
					freq2=0x21;
					freq1=0x65;
					freq0=0x6A;
					patable = patable_power_868;
					break;
    }
    cc1101_write_register(cc1101, FREQ2,freq2);
    cc1101_write_register(cc1101, FREQ1,freq1);
    cc1101_write_register(cc1101, FREQ0,freq0);
    cc1101_write_data(cc1101, PATABLE_BURST, (uint8_t*)patable, 8);
}

void cc1101_set_channel(cc1101_t *cc1101, uint8_t channel){
	/*
	 * @brief Set channel number.
	 */
	cc1101_write_register(cc1101, CHANNR, channel);
}

void cc1101_set_output_power_level(cc1101_t *cc1101, int8_t dBm)
/*
 * @brief Selects the entry of the PATABLES preset selected previously.
 */
{
    uint8_t pa = 0xC0;

    if      (dBm <= -30) pa = 0x00;
    else if (dBm <= -20) pa = 0x01;
    else if (dBm <= -15) pa = 0x02;
    else if (dBm <= -10) pa = 0x03;
    else if (dBm <= 0)   pa = 0x04;
    else if (dBm <= 5)   pa = 0x05;
    else if (dBm <= 7)   pa = 0x06;
    else if (dBm <= 10)  pa = 0x07;

    cc1101_write_register(cc1101, FREND0,pa);
}

float cc1101_set_carrier_offset(cc1101_t *cc1101, float offset){
	/*
	 * @Brief Configures frequency offset register to achieve the tergeted offset.
	 * @param offset Desired offset. Should be between -200KHz and +200KHz, depends on crystal.
	 * @returns The actual offset
	 */
	//rf_write_register(FSCTRL0, offset);
	int8_t freqoff = offset*(1<<14)/CRYSTAL_KHZ(cc1101->u32XtalFreqMhz);
	cc1101_write_register(cc1101, FSCTRL0, freqoff);
	return freqoff*(CRYSTAL_KHZ(cc1101->u32XtalFreqMhz)/(1<<14));
}

float cc1101_set_carrier_frequency(cc1101_t *cc1101, float target_freq){
	/* Note that this functions depends on the value of CRYSTAL_FREQUENCY_M.
	 * @param target_freq Frequency targeted, in MHz. Positive number. Note that the actual frequency may vary.
	 * @return Actual configured frequency.
	 */
	target_freq = target_freq*1000000;
	float freqf = target_freq*65536.0/(float)CRYSTAL_MHZ(cc1101->u32XtalFreqMhz);
	uint32_t freq = (uint32_t)freqf;
	freq = freq&0x00FFFFFF;
	cc1101_write_register(cc1101, FREQ0, freq);
	cc1101_write_register(cc1101, FREQ1, (freq>>8));
	cc1101_write_register(cc1101, FREQ2, (freq>>16));
	float t = ((float)freq*(float)CRYSTAL_MHZ(cc1101->u32XtalFreqMhz))/65536.0;

	return t;
}

float cc1101_set_channel_spacing(cc1101_t *cc1101, float cspacing){
	/*
	 * @brief Configures channel spacing registers to achieve the closer spacing possible to the target spacing
	 * Note that this functions depends on the value of CRYSTAL_FREQUENCY_M.
	 * @param cspacing Target spacing, in KHz. Positive number.
	 * @returns The actual configured spacing, in KHz
	 */
	uint8_t chanspc_e = 0;
	uint8_t chanspc_m = 0;
	float tmp;

	tmp = cspacing*((1<<18)/((float)CRYSTAL_KHZ(cc1101->u32XtalFreqMhz)*(1<<chanspc_e)))-256.0;
	while(tmp>256 && chanspc_e<4){
		chanspc_e++;
		tmp = cspacing*((1<<18)/((float)CRYSTAL_KHZ(cc1101->u32XtalFreqMhz)*(1<<chanspc_e)))-256.0;
	}
	chanspc_m = (uint8_t)tmp;
	cc1101_write_register(cc1101, MDMCFG0, chanspc_m);

	uint8_t mdmcfg1 = cc1101_read_register(cc1101, MDMCFG1);
	mdmcfg1 &= 0xFC;
	mdmcfg1 |= (chanspc_e & 0x2);
	cc1101_write_register(cc1101, MDMCFG1, mdmcfg1);

	cspacing = ((float)CRYSTAL_KHZ(cc1101->u32XtalFreqMhz)/(1<<18))*((float)chanspc_m+256.0)*(1<<chanspc_e);
	return cspacing;
}

void cc1101_set_preamble(cc1101_t *cc1101, uint8_t nbytes){
	/*
	 * @brief Sets the preamble size. The preamble is a stream of 1s and 0s that is sent before the packet.
	 */
	cc1101_sidle(cc1101);
	//TODO Rright now it is harcoded to be 8 bytes.

}

void cc1101_set_preamble_threshold(cc1101_t *cc1101, uint8_t nbytes){
	/*
	 * @brief Sets the minimum preamble bytes to detect.
	 */
	cc1101_sidle(cc1101);
	//TODO Right now it is hardocded to 4 bytes.

}


/*----------------------------[CC1101 States]----------------------------------------------*/
void cc1101_sidle(cc1101_t *cc1101){
	/**
	 * @brief Set RF chip to idle state
	 */
    uint8_t marcstate;

    cc1101_write_strobe(cc1101, SIDLE);              //sets to idle first. must be in

    marcstate = 0xFF;                     //set unknown/dummy state value

    while(marcstate != IDLE)
    {
        marcstate = (cc1101_read_register(cc1101, MARCSTATE) & 0x1F);
    }

}

void cc1101_receive(cc1101_t *cc1101){
	/*
	 * @brief Set RF chip to receive state (RX)
	 */
	//configure interruption, 1 when incoming packet
	cc1101_write_register(cc1101,  IOCFG0, 0x46);
	cc1101_write_strobe(cc1101, SFRX);
	cc1101_write_strobe(cc1101, SRX);

	uint8_t marcstate = 0xFF;
	while(marcstate != RX){
		marcstate = (cc1101_read_register(cc1101, MARCSTATE)); //read out state of cc1100 to be sure in RX
	}
	cc1101->gdo0_flag = 0;

}

//TODO Not tested
void cc1101_power_down(cc1101_t *cc1101){
    cc1101_sidle(cc1101);
    cc1101_write_strobe(cc1101, SPWD);               // CC1100 Power Down
}

//TODO Not tested
void cc1101_wakeup(cc1101_t *cc1101){
	/*
	 * @brief Wakes up the c1101 from power down.
	 */
    cc1101->fxCsGpio(_CS_ASSERT_LEVEL);
    cc1101->fxDelayUs(MS_TO_US(10));
    cc1101->fxCsGpio(_CS_DEASSERT_LEVEL);
    cc1101->fxDelayUs(MS_TO_US(10));
    //TODO rf_receive();                            // go to RX Mode
}

//TODO Not tested
uint8_t *cc1101_get_settings(cc1101_t *cc1101){
   static uint8_t settings[CFG_REGISTER];
   cc1101_read_data(cc1101, 0, settings, CFG_REGISTER);
   return settings;
}

//TODO Not tested
void cc1101_wor_enable(cc1101_t *cc1101){
	/*
	 * @brief enables WOR Mode  EVENT0 ~1890ms; rx_timeout ~235ms
	 */
	/*
		EVENT1 = WORCTRL[6:4] -> Datasheet page 88
		EVENT0 = (750/Xtal)*(WOREVT1<<8+WOREVT0)*2^(5*WOR_RES) = (750/26Meg)*65407*2^(5*0) = 1.89s
							(WOR_RES=0;RX_TIME=0)               -> Datasheet page 80
	i.E RX_TimeOut = EVENT0*       (3.6038)      *26/26Meg = 235.8ms
							(WOR_RES=0;RX_TIME=1)               -> Datasheet page 80
	i.E.RX_TimeOut = EVENT0*       (1.8029)      *26/26Meg = 117.9ms
	*/
    cc1101_sidle(cc1101);

    cc1101_write_register(cc1101, MCSM0, 0x18);    //FS Autocalibration
    cc1101_write_register(cc1101, MCSM2, 0x01);    //MCSM2.RX_TIME = 1b

    // configure EVENT0 time
    cc1101_write_register(cc1101, WOREVT1, 0xFF);  //High byte Event0 timeout
    cc1101_write_register(cc1101, WOREVT0, 0x7F);  //Low byte Event0 timeout

    // configure EVENT1 time
    cc1101_write_register(cc1101, WORCTRL, 0x78);  //WOR_RES=0b; tEVENT1=0111b=48d -> 48*(750/26MHz)= 1.385ms

    cc1101_write_strobe(cc1101, SFRX);             //flush RX buffer
    cc1101_write_strobe(cc1101, SWORRST);          //resets the WOR timer to the programmed Event 1
    cc1101_write_strobe(cc1101, SWOR);             //put the radio in WOR mode when CSn is released

    cc1101->fxDelayUs(MS_TO_US(1)); //TODO Really necessary?
}

//TODO Not tested
void cc1101_wor_disable(cc1101_t *cc1101){
	cc1101_sidle(cc1101);                            //exit WOR Mode
	cc1101_write_register(cc1101, MCSM2, 0x07); //stay in RX. No RX timeout
}

//TODO Not tested
void cc1101_wor_reset(cc1101_t *cc1101){
    cc1101_sidle(cc1101);                            //go to IDLE
    cc1101_write_register(cc1101, MCSM2, 0x01);    //MCSM2.RX_TIME = 1b
    cc1101_write_strobe(cc1101, SFRX);             //flush RX buffer
    cc1101_write_strobe(cc1101, SWORRST);          //resets the WOR timer to the programmed Event 1
    cc1101_write_strobe(cc1101, SWOR);             //put the radio in WOR mode when CSn is released

    cc1101->fxDelayUs(MS_TO_US(100)); //Really necessary?
}

/*----------------------------[CC1101 Data Flow]----------------------------------------------*/

uint8_t _keep_transmiting_data(cc1101_t *cc1101, uint8_t* data, int len){
	/**
	 * @brief This function CONTINUES the transmission of data, but DOES NOT start it. Controls the data flow from the MCU to C1101 once
	 * started.
	 */
	int len_transmited = 0;
	int32_t count;
	uint8_t last_chunk = len%DATA_CHUNK_SIZE;
	cc1101->gdo0_flag = 0;
	count = cc1101->timeout;
	while(len_transmited <len-last_chunk){
		if(cc1101->gdo0_flag){
		//while(rf_read_register(TXBYTES)>DATA_CHUNK_SIZE); //Suing polling because OBC Int. does not work.
			cc1101->gdo0_flag = 0;
			cc1101_write_data(cc1101, TXFIFO, &data[len_transmited], DATA_CHUNK_SIZE);
			len_transmited +=DATA_CHUNK_SIZE;
			// restart the timeout counter
			count = cc1101->timeout;
		}
		cc1101->fxDelayUs(100);
		count -= 100;
		if(count < 0) return FALSE;
	}
	if(last_chunk){
		while(!cc1101->gdo0_flag);
		//while(rf_read_register(TXBYTES)>DATA_CHUNK_SIZE); //Using polling because OBC Int. does not work.
		cc1101->gdo0_flag = 0;
		cc1101_write_data(cc1101, TXFIFO, &data[len_transmited], last_chunk);
		if(count < 0) return FALSE;
	}

	return TRUE;
}


cc1101_frame_status_e cc1101_send_frame(cc1101_t *cc1101, uint8_t* frame, int len){
	/**
	 * @brief Sends and unlimited length frame
	 * TODO RSSI and LQI values are appended to the packet, what to do with them?
	 */

	cc1101_sidle(cc1101); //Sets RF to idle state
	uint8_t pktcrtl0 = cc1101_read_register(cc1101, PKTCTRL0);
	uint8_t frame_len = len%256;
	pktcrtl0 = pktcrtl0 & 0b11111100; //reset len mode
	int len_sent = 0;

	//configure interruption, high to low when below threshold
	uint8_t iocfg0 = 0x2;
	cc1101_write_register(cc1101, IOCFG0, iocfg0);
	cc1101->gdo0_flag = 0;

	cc1101_write_strobe(cc1101, SFTX); //flush TX
	//TODO check if flushed

	//set packet length
	cc1101_write_register(cc1101, PKTLEN, frame_len);

	if(len>FIXED_LENGTH_LIMIT){ //Use infinite packet length mode
		//Set len mode to infinit
		pktcrtl0 = pktcrtl0 | 0x2;
		cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);

		//we need to fill the buffer before activating TX mode, or the chip will get into tx underflow state.
		cc1101_write_data(cc1101, TXFIFO, frame, FIFO_SIZE); //fill the buffer completely
		cc1101_write_strobe(cc1101, STX); //Start transmision
		len_sent +=FIFO_SIZE;

		int times = (len-len_sent)/FIFO_SIZE-1; //-1 to assure at bytes left to send them in receive mode
		//transmit (len -d -255) bytes of data, where d are the number of bytes already sent
		if(!_keep_transmiting_data(cc1101, &frame[len_sent], times*FIFO_SIZE)) return TIMEOUT;
		len_sent += times*FIFO_SIZE;

		//transmit remaining bytes in fixed length mode.

		//Set len mode to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);

		if(!_keep_transmiting_data(cc1101, &frame[len_sent], len-len_sent)) return TIMEOUT;

	}else{
		//Set len mode to fixed mode (default)
		cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);

		if(len>FIFO_SIZE){ //Use variable packet length mode
			cc1101_write_data(cc1101, TXFIFO, frame, FIFO_SIZE);
			cc1101_write_strobe(cc1101, STX);
			len_sent+=FIFO_SIZE;
			if(!_keep_transmiting_data(cc1101, &frame[len_sent], len-len_sent)) return TIMEOUT;
		}else{ //If len <= FIFO_SIZE, the FIFO needs to be filled once
			cc1101_write_data(cc1101, TXFIFO, frame, len);
			cc1101_write_strobe(cc1101, STX);
		}
	}

	int32_t count;
	uint8_t state = cc1101_read_register(cc1101, MARCSTATE);

	count = cc1101->timeout;
	while(state!=IDLE){
		//printf("%#20x\n\r", state);
		state = cc1101_read_register(cc1101, MARCSTATE);
		cc1101->fxDelayUs(100);
		count -= 100;
		if(count < 0){
			if(state==TXFIFO_UNDERFLOW){
				cc1101_write_strobe(cc1101, SFTX);
			}else{
				cc1101_sidle(cc1101);
			}
			return FRAME_BAD;
		}
	}

	printf("FRAME SENDED\n\r");
    return FRAME_OK;
}

uint8_t cc1101_incoming_packet(cc1101_t *cc1101){

	return cc1101->gdo0_flag;
}

uint8_t  _keep_receiving_data(cc1101_t *cc1101, uint8_t* data, int len){
	/**
	 * @brief This function CONTINUES the reception of data, but DOES NOT start it. Controls the data flow from the C1101 to MCU
	 * TODO RSSI and LQI values are appended to the packet, what to do with them?
	 */
	int len_received = 0;
	int32_t count;
	uint8_t last_chunk = len%DATA_CHUNK_SIZE;
	cc1101->gdo0_flag = 0;
	count = cc1101->timeout;
	while(len_received <len-last_chunk){

		if(cc1101->gdo0_flag){ //if buffer is half empty
			cc1101->gdo0_flag = 0;
			cc1101_read_data(cc1101, RXFIFO, &data[len_received], DATA_CHUNK_SIZE);
			len_received +=DATA_CHUNK_SIZE;
		}
		cc1101->fxDelayUs(50);
		count -= 50;
		if(count < 0) return FALSE;
	}
	if(last_chunk){
		if(!polling_while_lower(cc1101, RXBYTES, last_chunk)) return FALSE; //Polling because it won't trigger the threshold.
		cc1101->gdo0_flag = 0;
		cc1101_read_data(cc1101, RXFIFO, &data[len_received], last_chunk);
	}

	return TRUE;
}

uint8_t polling_while_lower(cc1101_t *cc1101, uint8_t reg, uint8_t size){
	uint8_t t = cc1101_read_register(cc1101, reg);
	int32_t count;

	count = cc1101->timeout;
	while(t<size){
		t = cc1101_read_register(cc1101, reg);

		cc1101->fxDelayUs(MS_TO_US(1));
		count -= MS_TO_US(1);
		if(count < 0) return FALSE;
	}
	return TRUE;
}

uint8_t polling_while_bigger(cc1101_t *cc1101, uint8_t reg, uint8_t size){
	uint8_t t = cc1101_read_register(cc1101, reg);
	while(t>size){
		t = cc1101_read_register(cc1101, reg);
		cc1101->fxDelayUs(MS_TO_US(10));
	}
	return TRUE;
}

uint16_t _get_frame_size(cc1101_t *cc1101, uint8_t* header, uint8_t data_len_loc, uint8_t data_len_size){
	/*
	 * @Returns The length of the frame.
	 */
	uint16_t mask = 1;
	for(int i = 1; i<data_len_size; i++){
		mask= mask <<1;
		mask+=1;
	}
	uint16_t frame_size;
	frame_size = (header[data_len_loc] & 0xFF) | ((header[data_len_loc+1]<<8) & 0xFF00);
	frame_size &=mask;
	return frame_size;
}

cc1101_frame_status_e receive_frame(cc1101_t *cc1101, cc1101_frame_data_t *frame){
	/*
	 * @Brief Receives a frame. When this function returns, the chip goes back to IDLE mode. In principle it should be possible to
	 * maintain the RX mode to receive another packet if I understood correctly, but I haven't been able to achieve that.
	 * @param frame_buffer Buffer to store the received data.
	 * @param len The maximum len allowed (aka the buffer len). Used also to return the len of the received packet.
	 * @param position of the data length field in the frame header. Must be within the first 64 bytes.
	 * @param data_len_size Length of the data lengh field in bits. Used to mask 2 Bytes for a custom field size.
	 * @param lqi Link Quality indicator, the lower the better.
	 * @param rssi Received Signal Strengh Indicator.
	 * @Return CRC checksum ok?
	 */

	//init some variables
	uint16_t max_len = *frame->len;
	*frame->len = 0;
	uint16_t frame_len;
	uint8_t data_field_size = sizeof(*frame->len);

	//Clear flag, since this function should have been called because of it
	cc1101->gdo0_flag = false;

	//configure interruption. Trigger when RX Buffer above threshold.
	cc1101_write_register(cc1101, IOCFG0, 0x40);

	//Set to infinite len mode
	uint8_t pktcrtl0 = cc1101_read_register(cc1101, PKTCTRL0);
	pktcrtl0 = pktcrtl0 & 0b11111100; //reset len mode
	pktcrtl0 = pktcrtl0 | 0x2;
	cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);


	//check if receiving something
	uint8_t SFD = 0b00001000; //Sync Word OK? Addr (if enabled) OK?
	uint8_t status = 1; //rf_read_register(PKTSTATUS);

	//get frame size
	if(!polling_while_lower(cc1101, RXBYTES, frame->data_len_loc+data_field_size)) return TIMEOUT; //TODO reconfigure RX Threshold to detect the first bytes??
	cc1101_read_data(cc1101, RXFIFO, frame->frame_buffer, frame->data_len_loc+data_field_size);
	frame_len = _get_frame_size(cc1101, frame->frame_buffer, frame->data_len_loc, frame->data_len_size);
	*frame->len +=frame->data_len_loc+data_field_size;
	if(frame_len > max_len) frame_len = max_len;


	uint16_t remaining_len = frame_len-*frame->len;
	//set packet length
	cc1101_write_register(cc1101, PKTLEN, (frame_len)%256);


	if(remaining_len>FIXED_LENGTH_LIMIT){
		int times = (remaining_len)/FIFO_SIZE;
		if(!_keep_receiving_data(cc1101, &frame->frame_buffer[*frame->len], times*FIFO_SIZE)) return TIMEOUT;
		*frame->len += times*FIFO_SIZE;

		//set packet length to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);

		//receive remaining
		remaining_len = frame_len-*frame->len;
		_keep_receiving_data(cc1101, &frame->frame_buffer[*frame->len], remaining_len);
		*frame->len+=remaining_len;

	}else if(remaining_len>DATA_CHUNK_SIZE){
		//set packet length to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);

		//receive remaining
		if(!_keep_receiving_data(cc1101, &frame->frame_buffer[*frame->len], remaining_len)) return TIMEOUT;
		*frame->len+=remaining_len;

	}else{
		//set packet length to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		cc1101_write_register(cc1101, PKTCTRL0, pktcrtl0);

		//TODO using polling to not reconfigure interrupt
		if(!polling_while_lower(cc1101, RXBYTES, remaining_len)) return TIMEOUT;
		cc1101_read_data(cc1101, RXFIFO, &frame->frame_buffer[*frame->len], remaining_len);
		*frame->len+=remaining_len;
	}

	*frame->lqi = cc1101_read_register(cc1101, LQI);
	uint8_t crc = (*frame->lqi) & 0x80;
	*frame->lqi = *frame->lqi & 0x7F;

	*frame->rssi = cc1101_read_register(cc1101, RSSI);
	while((status&SFD)){
				status = cc1101_read_register(cc1101, PKTSTATUS);
	}

	status = 0xFF;
	int32_t count = cc1101->timeout;
	while(status!=IDLE){ //PKTCTRL0 configured to go back to IDLE when reception finnished
		status = cc1101_read_register(cc1101, MARCSTATE);
		cc1101->fxDelayUs(MS_TO_US(1));
		if(count < 0){
			if(status==RXFIFO_OVERFLOW){
				cc1101_write_strobe(cc1101, SFRX);
			}else{
				cc1101_sidle(cc1101);
			}
			return FRAME_BAD;
		}
		count -= 1000;
	}

	cc1101_frame_status_e frame_status;
	if(crc){
		frame_status = FRAME_OK;
	}else{
		frame_status = FRAME_BAD;
	}
	return frame_status;
}


/* SPI Comm ----------------------------------------------------------------*/

void cc1101_write_strobe(cc1101_t *cc1101, uint8_t strobe){
	/**
	 * @brief Writes command to the CC1101 to change its state-machine state.
	 */
	strobe = WRITE(strobe);
	__spi_write(cc1101, &strobe, NULL, 0);
}

uint8_t cc1101_read_register(cc1101_t *cc1101, uint8_t reg){
	/**
	 * @brief Reads the content of a single 1-byte register.
	 * @Returns The register value.
	 */
	uint8_t data;
	reg= READ(reg);
	__spi_read(cc1101, &reg, &data, 1);
	return data;
}

void cc1101_write_register(cc1101_t *cc1101, uint8_t reg, uint8_t data){
	/**
	 * @brief Overwrites a register.
	 */
	reg = WRITE(reg);
	__spi_write(cc1101, &reg, &data, 1);
}

void cc1101_read_data(cc1101_t *cc1101, uint8_t addr, uint8_t* data, uint8_t size){
	/**
	 * @brief Reads multiple data.
	 * @param addr Base address.
	 * @param data The buffer where the read data will be stored.
	 * @param size Number of bytes to be read.
	 */
	if(size>1){
		addr = READ_BURST(addr);
	}else{
		addr = READ(addr);
	}
	__spi_read(cc1101, &addr, data, size);
}

void cc1101_write_data(cc1101_t *cc1101, uint8_t addr, uint8_t* data, uint8_t size){
	/**
	 * @brief Writes multiple data.
	 * @param addr Base address.
	 * @param data The buffer where the data to be written is located.
	 * @param size Number of bytes to be written.
	 */
	if(size>1){
		addr = WRITE_BURST(addr);
	}else{
		addr = WRITE(addr);
	}
	__spi_write(cc1101, &addr, data, size);
}

/* SPI Handling -------------------------------------------------------------*/

int8_t __spi_write(cc1101_t *cc1101, uint8_t *addr, uint8_t *pData, uint16_t size){
	int8_t status;

	assert(cc1101 != NULL);
	cc1101->fxCsGpio(_CS_ASSERT_LEVEL);
	status = cc1101->fxSpiWrite(addr, 1);
	if (status == 0 && pData != NULL){
		status = cc1101->fxSpiWrite(pData, size);
	}
	cc1101->fxCsGpio(_CS_DEASSERT_LEVEL);

	return status;
}

int8_t __spi_read(cc1101_t *cc1101, uint8_t *addr, uint8_t *pData, uint16_t size){
	int8_t status;

	assert(cc1101 != NULL);
	cc1101->fxCsGpio(_CS_ASSERT_LEVEL);
	status = cc1101->fxSpiWrite(addr, 1);
	if (status == 0 && pData != NULL){
		status = cc1101->fxSpiRead(pData, size);
	}
	cc1101->fxCsGpio(_CS_DEASSERT_LEVEL);

	return status;
}

/* Interrupts ---------------------------------------------------------------*/
void cc1101_gdo0_interrupt(cc1101_t *cc1101){
	assert (cc1101 != NULL);

	cc1101->gdo0_flag = 1;
}
