/*
 * P04_Triac.cpp
 *
 *  Created on: 17/03/2015
 *      Author: jctejero
 * 
 * Modified on: 17/03/2025
 *
 * description:
 * 	Práctica 5. Comunicaciones ModBus
 ***************************************************************************/

/***********************************************************/
/***                    Include files                    ***/
/***********************************************************/
#include <Arduino.h>
#include "Gest_Modbus.h"
#include "debuglog.h"
#include "cfg_loop.h"
#include "QueueList.h"  
#include "DomoBoard/domoBoard.h"
#include "config_practicas.h"

/*******************************************************/
/***                    Variables                    ***/
/*******************************************************/
QueueList<void_callback_f> _loop_callbacks;

/*********************************************************************/
/***                    Prototipos de funciones                    ***/
/*********************************************************************/
void main_loop();

void epdRegisterLoop(void_callback_f callback) {
    _loop_callbacks.push(callback);
}

void leerSensors(){
	/*if(Aregs[MB_SELPRACT] < PRACTICAS_MODBUS){
		leerSensores();
	}else*/ leeSensoresmb();
}

void setup() {

    //Iniciamos Modbus en Modo RTU
	Init_RTU_Modbus();
	
	//Leer Configuración actual
	
	load_Config();
	//Registramos la lectura de sensores
    epdRegisterLoop(leerSensors);
	
	//Registramos comprobación comunicaciones ModBus
	epdRegisterLoop(RTU_ModBus);

   
}

void loop() {
    EXECUTELOOP(){
		UPDATELOOP();

		main_loop();
		UpDown_Garaje();  // ← Esta línea es esencial para que funcione tu lógica del garaje
		Ctrl_PosicionGaraje(&ctrlPosPer, tsStaPer(Aregs[MB_STAPER]&0xFF));
		LOOP_x1s{
			mbDomoboard.leerSensor(&mbDomoboard.TEMPSEN);
		}

	}
}

void main_loop(){
	// llamada a las funciones registradas
	for (uint8_t i = 0; i < _loop_callbacks.count(); i++) {
		(_loop_callbacks.peek(i))();
	}
}
