/*
 * utils_domoBoard.h
 *
 *  Created on: 10/03/2015
 *      Author: jctejero
 */

#ifndef UTILS_DOMOBOARD_H_
#define UTILS_DOMOBOARD_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include	"Arduino.h"
#include	"DomoBoard/domoBoard.h"
#include    "DomoBoard/ModbusDomoboard.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/*
enum PinState {
    OFF = LOW,
    ON = HIGH
};
*/
#define 	ON   				HIGH
#define     OFF					LOW

/****************************************************************************/
/***                        Constants Definitions                         ***/
/****************************************************************************/
const uint16_t	persianaTiempoSubida 	= 7200;  //milisegundos

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void Interruptor(void *Sensor);
void interruptor_sal(void *Sensor);
void Pulsado_Soltado(void *Sensor);
void Pulsado_Soltado_sal(void *Sensor);
void conmutador(void *Sensor);
void conmutador_sal(void *Sensor);

void mbInterruptor(void *mbSensor);
void mbConmutador(void *mbSensor);
void mbInterruptorTemporizado(void *Sensor);
void Calc_Temperatura(void *Sensor);
void interruptor_SRC(void *Sensor);
void trigger_Level(void *Sensor);
void trigger_Level_garaje(void *Sensor);
void Persiana(void *Sensor);
void Garaje(void *Sensor);
void Ctrl_PosicionPersiana(TPCtrlTime ctrlPosPer, tsStaPer staPer);
void Ctrl_PosicionGaraje(TPCtrlTime ctrlPosPer, tsStaPer staPer);
void UpDown_Persiana();
void UpDown_Garaje();
void update_PersianaState();
void update_garajeState();

/****************************************************************************/
/***        				Exported Variables                            ***/
/****************************************************************************/
extern TCtrlTime 	ctrlPosPer;


#endif /* UTILS_DOMOBOARD_H_ */
