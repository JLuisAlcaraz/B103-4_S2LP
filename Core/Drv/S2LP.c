/*
 * S2LP.c
 *
 *  Created on: 10 jun 2026
 *      Author: jluis
 */


#include "S2LP.h"

void S2LP_SPI_Configurar_GPIO(void);
void S2LP_SetPaquetePalabraSincronizacion(void);
void S2LP_SetDataRateDesviacionFrecuenciaFiltro(void);
void S2LP_SetProtocoloFiltrado();
void S2LP_SPI_Configurar_Potencia(void);

void S2LP_CsLow(void);
void S2LP_CsHigh(void);
void S2LP_SdnLow(void);
void S2LP_SdnHigh(void);

void S2LP_DelayMs(uint32_t ms);

extern SPI_HandleTypeDef hspi1;

//-------------------------- INTERNAS -----------------------------------
void S2LP_CsLow(void)
{
	HAL_GPIO_WritePin(S2LP_CSN_GPIO_Port, S2LP_CSN_Pin, GPIO_PIN_RESET);
}


void S2LP_CsHigh(void)
{
	HAL_GPIO_WritePin(S2LP_CSN_GPIO_Port, S2LP_CSN_Pin, GPIO_PIN_SET);
}

void S2LP_SdnLow(void)
{
	HAL_GPIO_WritePin(S2LP_SDN_GPIO_Port, S2LP_SDN_Pin, GPIO_PIN_RESET);
}


void S2LP_SdnHigh(void)
{
	HAL_GPIO_WritePin(S2LP_SDN_GPIO_Port, S2LP_SDN_Pin, GPIO_PIN_SET);
}
//--------------------------------------------------------------------


void S2LP_Reset(void)
{
	S2LP_SdnHigh();
	HAL_Delay(10);
	S2LP_SdnLow();
	HAL_Delay(10);
	S2LP_SendCommand(S2LP_CMD_SRES);
	HAL_Delay(5);
}


void S2LP_SendCommand(uint8_t cmd)
{
	uint8_t buff[2];

	buff[0] = 0x80;
	buff[1] = cmd;

	S2LP_CsLow();
	HAL_SPI_Transmit(&hspi1, buff, 2, HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_WriteReg(uint8_t addr, uint8_t value)
{
	uint8_t buff[3];

	buff[0] = 0x00;
	buff[1] = addr;
	buff[3] = value;
    S2LP_CsLow();
    HAL_SPI_Transmit(&hspi1, buff, 3, HAL_MAX_DELAY);
    S2LP_CsHigh();
}


uint8_t S2LP_ReadReg(uint8_t addr)
{
	uint8_t txBuff[3];
	uint8_t rxBuff[3];

	txBuff[0] = 0x01;
	txBuff[1] = addr;
	txBuff[2] = 0xFF;
    S2LP_CsLow();
    HAL_SPI_TransmitReceive(&hspi1, txBuff, rxBuff, 3, HAL_MAX_DELAY);
    S2LP_CsHigh();
    return rxBuff[2];
}


void S2LP_WriteFifo(uint8_t *data, uint8_t len)
{
    uint8_t hdr[2];

    hdr[0] = 0x00;
    hdr[1] = 0xFF;
    S2LP_CsLow();
    HAL_SPI_Transmit(&hspi1, hdr, 2, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
    S2LP_CsHigh();
}


void S2LP_StartRx(void)
{
	S2LP_SendCommand(S2LP_CMD_SABORT);
	S2LP_SendCommand(S2LP_CMD_RX);
}


void S2LP_StartTx(void)
{
	S2LP_SendCommand(S2LP_CMD_SABORT);
	S2LP_SendCommand(S2LP_CMD_TX);
}


uint8_t S2LP_GetRxData(uint8_t *buff, uint8_t maxLen)
{
    uint8_t hdr[2];
    uint8_t len = 0;

    // Leer FIFO RX
    hdr[0] = 0x01;   // read
    hdr[1] = 0xFF;   // RX FIFO

    S2LP_CsLow();
    HAL_SPI_Transmit(&hspi1, hdr, 2, HAL_MAX_DELAY);
    // Primer byte normalmente es longitud (en packet mode básico)
    HAL_SPI_Receive(&hspi1, &len, 1, HAL_MAX_DELAY);
    if(len > maxLen)
	{
		len = maxLen;
	}
    HAL_SPI_Receive(&hspi1, hdr, len, HAL_MAX_DELAY);
    S2LP_CsHigh();
    return len;
}


void S2LP_Config868(void)
{
    S2LP_Reset();
    S2LP_SendCommand(S2LP_CMD_RCO_CALIB);
    HAL_Delay(10);
    S2LP_SendCommand(S2LP_CMD_STANDBY);
    S2LP_SetFrequency_868();
    S2LP_SetDataRateDesviacionFrecuenciaFiltro();
    S2LP_SetPaquetePalabraSincronizacion();
    S2LP_SetProtocoloFiltrado();
    S2LP_SPI_Configurar_Potencia();
    S2LP_SPI_Configurar_GPIO();

    S2LP_SendCommand(S2LP_CMD_STANDBY); // STANDBY

    S2LP_SendCommand(S2LP_CMD_FLUSHRXFIFO); // flush RX
    S2LP_SendCommand(S2LP_CMD_FLUSHTXFIFO); // flush TX
}


void S2LP_SPI_Configurar_GPIO(void)
{
	uint8_t buffDummy[10];
	uint8_t Peticion_Escritura[2] = { 0x00, 0 };
	uint8_t config_registro = 2;
	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, &config_registro, 1, HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_SetPaquetePalabraSincronizacion(void)
{
	uint8_t buffDummy[10];
	uint8_t Config_Registros_Consecutivos[10] = { 0x08, 0x00, 0x01, 0x30, 0x00, 0x05, 0x23, 0x25, 0x27, 0x29 };
	uint8_t Peticion_Escritura[2] = { 0x00, 45 };
	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Config_Registros_Consecutivos, 10, HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_SetProtocoloFiltrado()
{
	uint8_t buffDummy[10];
	uint8_t PROTOCOL2_VALUE = 0x00; ///< 0x00 -> CS, SQI y PQI = 0 (Rx timeout disabled)
	uint8_t PROTOCOL1_VALUE = 0x01; ///< 0x01 -> Enable auto package filter
	uint8_t Config_Registros_Consecutivos[2] = { PROTOCOL2_VALUE, PROTOCOL1_VALUE };
	uint8_t Peticion_Escritura[2] = { 0x00, 57 };
	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Config_Registros_Consecutivos, 2, HAL_MAX_DELAY);
	S2LP_CsHigh();

	/** 0x03 -> Auto discard if not valid CRC and filter RX packet RX_TIMEOUT_AND_OR_SEL = 0 (Rx timeout disabled) */
	S2LP_WriteReg(64, 0x03);
}


void S2LP_SPI_Configurar_Potencia(void)
{
	uint8_t buffDummy[10];
	S2LP_WriteReg(90, 0x02);
	S2LP_WriteReg(99, 0x01);

	uint8_t REG_PM_CONF1 = 0x35; ///< 0x35 -> Power Management set for Low Power Mode
	uint8_t REG_PM_CONF0 = 0x12; ///< 0x12 -> Power Management set for Low Power Mode
	uint8_t Config_Registros_Consecutivos[2] = { REG_PM_CONF1, REG_PM_CONF0 };
	uint8_t Peticion_Escritura[2] = { 0x00, 120 };
	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Config_Registros_Consecutivos, 2, HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_SendPacket(uint8_t *data, uint8_t len)
{
	uint8_t header = len;

    S2LP_SendCommand(S2LP_CMD_STANDBY);
    S2LP_SendCommand(S2LP_CMD_FLUSHTXFIFO);

    S2LP_CsLow();
    HAL_SPI_Transmit(&hspi1, &header, 1, 100);
    HAL_SPI_Transmit(&hspi1, data, len, 100);
    S2LP_CsHigh();
    S2LP_SendCommand(S2LP_CMD_TX);
}


uint8_t S2LP_ReceivePacket(S2LP_HandleTypeDef *dev, uint8_t *buffer, uint8_t maxLen)
{
    uint8_t rx_len = 0;

    // 1. comprobar si hay paquete disponible (IRQ pin)
    if (HAL_GPIO_ReadPin(dev->IRQ_Port, dev->IRQ_Pin) == GPIO_PIN_RESET)
        return 0;

    // 2. poner en RX (asegura estado correcto)
    S2LP_SendCommand(S2LP_CMD_RX);

    // 3. leer longitud del payload (modo fixed/variable depende config)
    uint8_t header = 0;

    S2LP_CsLow();
    HAL_SPI_Receive(&hspi1, &header, 1, 100);
    S2LP_CsHigh();

    rx_len = header;

    if (rx_len > maxLen)
        rx_len = maxLen;

    // 4. leer payload
    S2LP_CsLow();
    HAL_SPI_Receive(&hspi1, buffer, rx_len, 100);
    S2LP_CsHigh();

    // 5. limpiar FIFO RX
    S2LP_SendCommand(S2LP_CMD_FLUSHRX);

    return rx_len;
}


void S2LP_Init(void)
{
    S2LP_Reset();
    S2LP_Config868();
}



void S2LP_DelayMs(uint32_t ms)
{
	HAL_Delay(ms);
}



void S2LP_SetFrequency_868(void)
{
	// CONFIGURACIÓN DE LA FRECUENCIA BASE A 869.5 MHz
	uint8_t buffDummy[10];
	uint8_t REG_SYNT3 = 0x62;
	uint8_t REG_SYNT2 = 0x2B;
	uint8_t REG_SYNT1 = 0x7A;
	uint8_t REG_SYNT0 = 0xE1;
	uint8_t REG_IF_OFFSET_ANA = 0x2F;
	uint8_t REG_IF_OFFSET_DIG = 0xC2;
	uint8_t Config_Registros_Consecutivos[6] = { REG_SYNT3, REG_SYNT2, REG_SYNT1, REG_SYNT0, REG_IF_OFFSET_ANA, REG_IF_OFFSET_DIG };

	uint8_t Peticion_Escritura[2] = { 0x00, 0x05 };

	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Config_Registros_Consecutivos, 6, HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_SetDataRateDesviacionFrecuenciaFiltro(void)
{
	uint8_t buffDummy[10];
	uint8_t Peticion_Escritura[2] = { 0x00, 0x0E };

	// DATA RATE 38.399 kbps - FREQ DEVIATION 19.979KHz - CHANNEL FILTER 100KHz
	uint8_t REG_MOD4 = 0x92;
	uint8_t REG_MOD3 = 0xA5;
	uint8_t REG_MOD2 = 0x27;
	uint8_t REG_MOD1 = 0x03;
	uint8_t REG_MOD0 = 0xA3;
	uint8_t REG_CHFLT = 0x13;

	// DATA RATE 1.195 kbps - 2-FSK - FREQ DEVIATION 2.396KHz - CHANNEL FILTER 10KHz
//const uint8_t REG_MOD4 = 0x91;
//const uint8_t REG_MOD3 = 0x4D;
//const uint8_t REG_MOD2 = 0x02;
//const uint8_t REG_MOD1 = 0x00;
//const uint8_t REG_MOD0 = 0xC9;
//const uint8_t REG_CHFLT = 0x56;

	// DATA RATE 10kbps
//  const uint8_t REG_MOD4 = 0xA3;
//  const uint8_t REG_MOD3 = 0x6C;
//  const uint8_t REG_MOD2 = 0x25;
//  const uint8_t REG_MOD1 = 0x01;
//  const uint8_t REG_MOD0 = 0xA3;
//  const uint8_t REG_CHFLT = 0x15;

	// DATA RATE 16kbps - MEJOR freq dev 8.011 chan filter 40.962
//  const uint8_t REG_MOD4 = 0x4F;
//  const uint8_t REG_MOD3 = 0x8A;
//  const uint8_t REG_MOD2 = 0x26;
//  const uint8_t REG_MOD1 = 0x02;
//  const uint8_t REG_MOD0 = 0x50;
//  const uint8_t REG_CHFLT = 0x54;

	// DATA RATE 25kbps - freq dev 12.207 chan filter 64.905
//  const uint8_t REG_MOD4 = 0x06;
//  const uint8_t REG_MOD3 = 0x21;
//  const uint8_t REG_MOD2 = 0x27;
//  const uint8_t REG_MOD1 = 0x03;
//  const uint8_t REG_MOD0 = 0x00;
//  const uint8_t REG_CHFLT = 0x83;

	uint8_t Config_Registros_Consecutivos_1[6] = { REG_MOD4, REG_MOD3, REG_MOD2, REG_MOD1, REG_MOD0, REG_CHFLT };
	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Config_Registros_Consecutivos_1, 6, HAL_MAX_DELAY);
	S2LP_CsHigh();

	uint8_t REG_ANT_SELECT_CONF = 0x55; ///< 0x55 -> Enable CS blanking
	uint8_t REG_CLOCKREC2 = 0x00;       ///< 0x00 -> Clock recovery witouth gain config

	uint8_t Config_Registros_Consecutivos_2[2] = { REG_ANT_SELECT_CONF, REG_CLOCKREC2 };
	S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, Peticion_Escritura, buffDummy, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, Config_Registros_Consecutivos_2, 2, HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_SetModem(void)
{
    S2LP_WriteReg(S2LP_PROTOCOL1, 0xA3); //MODULATION = 2-GFSK
    S2LP_WriteReg(S2LP_FDEV_M, 0x06); //FREQ DEV (desviación ~20-40 kHz)
    S2LP_WriteReg(S2LP_DATARATE_M_MSB, 0x6B); //DATA RATE (38.4 kbps típico)
    S2LP_WriteReg(S2LP_DATARATE_M_LSB, 0x2C);
    S2LP_WriteReg(S2LP_DATARATE_E, 0x00);
}


void S2LP_SetPacketFormat(void)
{
    S2LP_WriteReg(S2LP_PCKTCTRL1, 0x90); //Enable packet mode + fixed length
    S2LP_WriteReg(S2LP_PCKTLEN, 4); //Payload length = 4 bytes ("HOLA")
    S2LP_WriteReg(S2LP_SYNC0, 0x88); //Sync word (0x88888858 típico ST)
    S2LP_WriteReg(S2LP_SYNC1, 0x88);
    S2LP_WriteReg(S2LP_SYNC2, 0x88);
    S2LP_WriteReg(S2LP_SYNC3, 0x58);
    S2LP_WriteReg(S2LP_PCKTCTRL3, 0x10); //CRC enabled (recomendado)
}


void S2LP_SetRxBW_38k4(void)
{
    //Ajuste típico para 38.4 kbps + FSK/GFSK en 868 MHz
    S2LP_WriteReg(S2LP_CHFLT, 0x8A); //BW ~100-150 kHz (estable)
}
