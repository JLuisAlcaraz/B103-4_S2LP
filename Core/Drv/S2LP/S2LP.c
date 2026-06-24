/*
 * S2LP.c
 *
 *  Created on: 10 jun 2026
 *      Author: jluis
 */

#include "string.h"
#include "S2LP.h"

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


void S2LP_DelayMs(uint32_t ms)
{
	HAL_Delay(ms);
}


void S2LP_SendCommand(uint8_t cmd)
{
	uint8_t buff[] = {S2LP_SEND_STROBE, cmd};

	S2LP_CsLow();
	HAL_SPI_Transmit(&hspi1, buff, sizeof(buff), HAL_MAX_DELAY);
	S2LP_CsHigh();
}


void S2LP_WriteTXFIFO(uint8_t *pData, uint8_t length)
{
    uint8_t header[] = {S2LP_WRITE_HEADER, S2LP_REG_TX_FIFO};

    S2LP_CsLow();
    HAL_SPI_Transmit(&hspi1, header, 2, HAL_MAX_DELAY); //1. Enviar el Header indicando Escritura en FIFO
    HAL_SPI_Transmit(&hspi1, pData, length, HAL_MAX_DELAY); //2. Enviar la ráfaga de datos (los 4 bytes)
    S2LP_CsHigh();
}


void S2LP_ReadRXFIFO(uint8_t *pBuffer, uint8_t length)
{
    uint8_t header[] = {S2LP_READ_HEADER, S2LP_REG_RX_FIFO};

    S2LP_CsLow();
    HAL_SPI_Transmit(&hspi1, header, 2, HAL_MAX_DELAY); //1. Enviar el Header indicando Lectura de FIFO
    HAL_SPI_Receive(&hspi1, pBuffer, length, HAL_MAX_DELAY); //2. Recibir los datos del aire guardados en la memoria del módulo
    S2LP_CsHigh();
}


void S2LP_WriteReg(uint8_t addr, uint8_t value)
{
	uint8_t buff[] = {S2LP_WRITE_HEADER, addr, value};

    S2LP_CsLow();
	HAL_SPI_Transmit(&hspi1, buff, sizeof(buff), HAL_MAX_DELAY);
    S2LP_CsHigh();
}


uint8_t S2LP_ReadReg(uint8_t addr)
{
	uint8_t buf_rx[2];
	uint8_t buf_tx[] = {S2LP_READ_HEADER, addr, 0xFF};

    S2LP_CsLow();
	HAL_SPI_TransmitReceive(&hspi1, buf_tx, buf_rx, 3, HAL_MAX_DELAY);
    S2LP_CsHigh();
    return buf_rx[2];
}


uint8_t S2LP_GetStatus(void)
{
    uint8_t header = S2LP_WRITE_HEADER;
    uint8_t status_byte = 0;

    S2LP_CsLow();
    // El primer byte recibido durante la transmisión del header contiene el estado del chip
    HAL_SPI_TransmitReceive(&hspi1, &header, &status_byte, 1, HAL_MAX_DELAY);
    S2LP_CsHigh();

    // Filtramos los bits 1 a 6 que contienen los estados principales (MC_STATE)
    return (status_byte >> 1) & 0x7F;
}


void S2LP_Init_Minima_Registros(void)
{
    // 1. Reset físico por comando para limpiar estados previos
    S2LP_SendCommand(0x70);
    HAL_Delay(20);

    // 2. CONFIGURACIÓN DE ENERGÍA DE ALTA ESTABILIDAD (Evita caídas analógicas)
    S2LP_WriteReg(0x76, 0x01); // REG_PM_CONF1: Forzar encendido del SMPS interno
    S2LP_WriteReg(0x77, 0x12); // REG_PM_CONF0: Voltaje analógico a 1.2V
    S2LP_WriteReg(0x73, 0x0A); // REG_BATTERY_TH: Ajuste de umbral de batería baja

    // 3. CONFIGURACIÓN DEL OSCILADOR (Cristal de 26 MHz)
    // Desactivamos la inyección externa y los condensadores internos adicionales
    S2LP_WriteReg(0x10, 0x00); // REG_XO_CONF: Configuración del oscilador base

    // 4. FRECUENCIA PORTADORA A 868.0 MHz (Cálculo matemático exacto para XTAL de 26 MHz)
    S2LP_WriteReg(0x01, 0x22); // REG_SYNT3
    S2LP_WriteReg(0x02, 0x26); // REG_SYNT2
    S2LP_WriteReg(0x03, 0x66); // REG_SYNT1
    S2LP_WriteReg(0x04, 0x66); // REG_SYNT0
    S2LP_WriteReg(0x05, 0x00); // REG_CH_NUM: Canal base 0

    // 5. MODULACIÓN 2-FSK Y FILTRO DE CANAL (Tasa de datos: 38.4 kbps)
    S2LP_WriteReg(0x11, 0x02); // REG_MOD1: Configura el modulador en modo 2-FSK
    S2LP_WriteReg(0x12, 0x3B); // REG_MOD0: Desactiva el conformado de pulso Gaussiano
    S2LP_WriteReg(0x14, 0x24); // REG_CHFLT: Filtro RX ancho (100kHz) para tolerar ruido

    // 6. POTENCIA DE SALIDA Y AUTO-CALIBRACIÓN DEL VCO (Obligatorio para salir de 0x29)
    S2LP_WriteReg(0x09, 0x25); // REG_PA_POWER0: Configura una potencia segura (+10 dBm)
    S2LP_WriteReg(0x0A, 0x04); // REG_PA_POWER1: Habilita la auto-calibración analógica del VCO

    // 7. MOTOR DE PAQUETES (Modo básico, longitud fija de 4 bytes, sin CRC)
    S2LP_WriteReg(0x1E, 0x00); // REG_PCKTCTRL1: Desactiva filtro de dirección y CRC
    S2LP_WriteReg(0x1F, 0x00); // REG_PCKTCTRL0: Modo de longitud de paquete fija
    S2LP_WriteReg(0x19, 0x04); // REG_PCKT_LEN: Fuerza al hardware a cortar en el byte 4

    // 8. PALABRA DE SINCRONISMO (0x344D344D)
    S2LP_WriteReg(0x1A, 0x34); // REG_SYNT_WORD3
    S2LP_WriteReg(0x1B, 0x4D); // REG_SYNT_WORD2
    S2LP_WriteReg(0x1C, 0x34); // REG_SYNT_WORD1
    S2LP_WriteReg(0x1D, 0x4D); // REG_SYNT_WORD0

    HAL_Delay(10); // Margen de estabilización tras la carga de registros
}
