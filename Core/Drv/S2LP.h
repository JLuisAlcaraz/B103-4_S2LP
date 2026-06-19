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


/* ============================================================
 * S2LP REGISTER MAP (partial - extend as needed)
 * ============================================================ */

/* GPIO Configuration */
#define S2LP_GPIO0_CONF        0x00
#define S2LP_GPIO1_CONF        0x01
#define S2LP_GPIO2_CONF        0x02
#define S2LP_GPIO3_CONF        0x03

/* Synthesizer Registers */
#define S2LP_SYNT3             0x05
#define S2LP_SYNT2             0x06
#define S2LP_SYNT1             0x07
#define S2LP_SYNT0             0x08

/* Modem registers */
#define S2LP_PROTOCOL1         0x0C
#define S2LP_DATARATE_M_MSB    0x10
#define S2LP_DATARATE_M_LSB    0x11
#define S2LP_DATARATE_E        0x12
#define S2LP_CHFLT             0x13
#define S2LP_FDEV_M            0x15

/* Packet Control */
#define S2LP_RSSI_FLT          0x2B
#define S2LP_PCKTCTRL3_FECTRL  0x2C
#define S2LP_PCKTCTRL2         0x2D
#define S2LP_PCKTCTRL0         0x2E
#define S2LP_PCKTCTRL1         0x2F
#define S2LP_PCKTLEN           0x30
#define S2LP_PCKTCTRL3         0x38

#define REG_PCKTCTRL1          0x30
#define REG_PCKTLEN0           0x31

/* Sync Registers */
#define S2LP_SYNC0             0x33
#define S2LP_SYNC1             0x34
#define S2LP_SYNC2             0x35
#define S2LP_SYNC3             0x36

/* IRQ Registers */
#define S2LP_IRQ_MASK3         0x90
#define S2LP_IRQ_MASK2         0x91
#define S2LP_IRQ_MASK1         0x92
#define S2LP_IRQ_MASK0         0x93

#define S2LP_IRQ_STATUS3       0xFA
#define S2LP_IRQ_STATUS2       0xFB
#define S2LP_IRQ_STATUS1       0xFC
#define S2LP_IRQ_STATUS0       0xFD
#define REG_FIFO               0xFF

/* FIFO */
#define S2LP_FIFO              0xFF

/* ============================================================
 * BIT MASKS (example subset)
 * ============================================================ */

/* PCKTCTRL1 */
#define S2LP_PCKTCTRL1_CRC_MODE_MASK     0x07
#define S2LP_PCKTCTRL1_FIX_VAR_LEN       (1 << 3)
#define S2LP_PCKTCTRL1_MANCHESTER_EN     (1 << 4)

/* GPIO CONFIG */
#define S2LP_GPIO_MODE_MASK              0x03
#define S2LP_GPIO_SELECT_MASK            0xF8

/* IRQ FLAGS (example) */
#define S2LP_IRQ_TX_DATA_SENT            (1 << 2)
#define S2LP_IRQ_RX_DATA_READY           (1 << 0)
#define S2LP_IRQ_CRC_ERROR               (1 << 4)

/* ============================================================
 * COMMANDS (SPI strobe commands)
 * ============================================================ */

#define S2LP_CMD_TX              0x60
#define S2LP_CMD_RX              0x61
#define S2LP_CMD_READY           0x62
#define S2LP_CMD_STANDBY         0x63
#define S2LP_CMD_SLEEP           0x64
#define S2LP_CMD_LOCKRX          0x65
#define S2LP_CMD_LOCKTX          0x66
#define S2LP_CMD_SABORT          0x67
#define S2LP_CMD_LDC_RELOAD      0x68
#define S2LP_CMD_RCO_CALIB       0x69
#define S2LP_CMD_SRES            0x70
#define S2LP_CMD_FLUSHRXFIFO     0x71
#define S2LP_CMD_FLUSHTXFIFO     0x72


/* ================= Function prototypes ================= */
void    S2LP_Init(void);
void    S2LP_Reset(void);
void    S2LP_WriteReg(uint8_t addr, uint8_t value);
uint8_t S2LP_ReadReg(uint8_t addr);
void    S2LP_SendCommand(uint8_t cmd);
void    S2LP_WriteFifo(uint8_t *data, uint8_t len);
void    S2LP_Config868(void);
void    S2LP_SendPacket(uint8_t *data, uint8_t len);
uint8_t S2LP_ReceivePacket(S2LP_HandleTypeDef *dev, uint8_t *buffer, uint8_t maxLen);
uint8_t S2LP_GetRxData(uint8_t *buff, uint8_t len);
void    S2LP_StartRx(void);
void    S2LP_StartTx(void);

void S2LP_SetFrequency_868(void);
void S2LP_SetModem(void);
void S2LP_SetPacketFormat(void);
void S2LP_SetRxBW_38k4(void);
void S2LP_SendPacket(uint8_t *data, uint8_t len);
#endif /* DRV_S2LP_H_ */
