/*
 * config_practicas.h
 *
 *  Created on: 10/03/2015
 *      Author: jctejero
 * 
 *  Modified on: 13/05/2025
 */

#ifndef CONFIG_PRACTICAS_H_
#define CONFIG_PRACTICAS_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include    <Arduino.h>
#include	"utils_domoBoard.h"
#include	"DomoBoard/domoBoard.h"
#include	"DomoBoard/ModbusDomoboard.h"

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
	uint8_t mbCmd;
	String	NamePractica;
    void_callback_f config_practica;
}TPractica;

/****************************************************************************/
/***        				Macro Definitions                             ***/
/****************************************************************************/
//Defininición de comando de configuración
#define	P1_PULSADORES				0x11
#define	P1_INTERRUPTOR				0x12
#define	P1_CONMUTADOR				0x13
#define	P3_CONMUTADOR				0x32
#define	P4_PULSADORES				0x42
#define	P4_INTERRUPTOR				0x43
#define	P4_CONMUTADOR_INT			0x44
#define	P4_CONMUTADOR_2SAL	    	0x45
#define PRACTICAS_MODBUS			0x50
#define	P5_INTERRUPTOR				0x54
#define	P6_INTERRUPTOR				0x62
#define P7_PIR					    0x70
#define P9_ALL						0x90
#define P10_PER						0xA1
#define P11_GARAGE					0xA2

/****************************************************************************/
/***        				Exported Functions                            ***/
/****************************************************************************/
void SelectionConfiguration(uint8_t selConf);

#endif /* CONFIG_PRACTICAS_H_ */
