/*
 * Gest_Modbus.h
 *
 *  Created on: 16/05/2014
 *      Author: jctejero
 *
 *  Modified on: 01/04/2025
 */

#ifndef GEST_MODBUS_H_
#define GEST_MODBUS_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include 	"ModbusSlave/ModbusSlave.h"
#include 	"DomoBoard/domoBoard.h"
#include    "DomoBoard/ModbusDomoboard.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
//Configuración de la conexión MODBUS
#define	ADDR_SLAVE		1		//Dirección Dispositivo Esclavo
#define	SERIAL_BPS		115200	//Velocidad Comunicación serie
#define	SERIAL_PARITY	'n'		//Paridad comunicación serie
#define	TX_EN_PIN		0		//Pin usado para la transmisión RS485; 0 No usado

// La cofiguración de domoboard usando ModBus, será almacenada en la EEPROM.
// Las direccioes de los valores a almacenar se indican acontinuación

#define	ADDR_SELPRACT		0		//Comando de Configuración actual de DomoBoad
#define	ADDR_ACTPIR			1		//Activa/Desactiva PIR
#define ADDR_TIEMPO_PIR_1	2		//Tiempo PIR Activo 1
#define ADDR_TIEMPO_PIR_2	3		//Tiempo PIR Activo 2
#define ADDR_ACTSRC			4		//Activa/desactiva SRC (Photorresistencia)
#define	ADDR_SRC_HL_1		5		//Nivel Alto Activación SRC
#define	ADDR_SRC_HL_2		6		//Nivel Alto Activación SRC
#define	ADDR_SRC_LL_1		7		//Nivel Bajo Activación SRC
#define	ADDR_SRC_LL_2		8		//Nivel Bajo Activación SRC
#define ADDR_ACTTOR			9       //Activa/desactiva Phototransistor
#define ADDR_TTOR_LL_1		10		//Nivel Bajo Activación Trigger ttor
#define ADDR_TTOR_LL_2		11		//Nivel Bajo Activación Trigger ttor


/****************************************************************************/
/***        DEFINICIÓN DE FUNCIONES    **************************************/
/****************************************************************************/
void Init_RTU_Modbus();
void RTU_ModBus();
void writeholdingregister();
void load_Config();

extern ModbusSlave mbs;

#endif /* GEST_MODBUS_H_ */
