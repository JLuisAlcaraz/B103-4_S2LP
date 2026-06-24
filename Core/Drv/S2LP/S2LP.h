/*
 * S2LP.h
 *
 *  Created on: 10 jun 2026
 *      Author: jluis
 */

#ifndef DRV_S2LP_H_
#define DRV_S2LP_H_


#include "main.h"
#include <stdint.h>
#include <stdbool.h>


#define S2LP_DUMMY_BYTE    0xFF
#define S2LP_WRITE_HEADER  0x00
#define S2LP_READ_HEADER   0x01
#define S2LP_SEND_STROBE   0x02

//Comandos strobe
#define S2LP_CMD_STX       0x60  // Entrar en modo Transmisión
#define S2LP_CMD_SRX       0x61  // Entrar en modo Recepción
#define S2LP_CMD_SREADY    0x62
#define S2LP_CMD_SABORT    0x71
#define S2LP_CMD_SFLUSH_TX 0x72  // Vaciar FIFO de Transmisión
#define S2LP_CMD_SFLUSH_RX 0x73  // Vaciar FIFO de Recepción

//Registros
#define S2LP_REG_SYNT3      0x01  // Frecuencia Sintetizador byte 3
#define S2LP_REG_SYNT2      0x02  // Frecuencia Sintetizador byte 2
#define S2LP_REG_SYNT1      0x03  // Frecuencia Sintetizador byte 1
#define S2LP_REG_SYNT0      0x04  // Frecuencia Sintetizador byte 0
#define S2LP_REG_CH_NUM     0x05  // Número de canal
#define S2LP_REG_PA_POWER0  0x09  // Potencia de salida del Amplificador de Potencia
#define S2LP_REG_MOD1       0x11  // Modulación byte 1
#define S2LP_REG_MOD0       0x12  // Modulación byte 0
#define S2LP_REG_PCKT_LEN   0x19  // Configuración de longitud del paquete
#define S2LP_REG_SYNT_WORD3 0x1A  // Palabra de Sincronismo byte 3
#define S2LP_REG_SYNT_WORD2 0x1B  // Palabra de Sincronismo byte 2
#define S2LP_REG_SYNT_WORD1 0x1C  // Palabra de Sincronismo byte 1
#define S2LP_REG_SYNT_WORD0 0x1D  // Palabra de Sincronismo byte 0
#define S2LP_REG_PCKTCTRL1  0x1E  // Control del motor de paquetes 1
#define S2LP_REG_PCKTCTRL0  0x1F  // Control del motor de paquetes 0
#define S2LP_REG_TX_FIFO    0xFF
#define S2LP_REG_RX_FIFO    0xFE


/* ================= Function prototypes ================= */
void S2LP_CsLow(void);
void S2LP_CsHigh(void);
void S2LP_SdnLow(void);
void S2LP_SdnHigh(void);
void S2LP_DelayMs(uint32_t ms);
void S2LP_SendCommand(uint8_t commandCode);
void S2LP_WriteTXFIFO(uint8_t *pData, uint8_t length);
void S2LP_ReadRXFIFO(uint8_t *pBuffer, uint8_t length);
void S2LP_WriteReg(uint8_t addr, uint8_t value);
uint8_t S2LP_ReadReg(uint8_t addr);
uint8_t S2LP_GetStatus(void);
void S2LP_Init_Minima_Registros(void);
#endif /* DRV_S2LP_H_ */
