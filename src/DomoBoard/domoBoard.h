/*
 * domoBoard.h
 *
 *  Created on: 09/03/2015
 *      Author: jctejero
 *
 *  Modified on: 13/05/2025
 */

#ifndef DOMOBOARD_H_
#define DOMOBOARD_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "Arduino.h"
#include "types.h"
#include "QueueList.h"
#include "AsyncWait.h"

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* constants */
typedef enum
{
	RX485_RX,
	RX485_TX
} TRX485_rxtx;

typedef enum
{
	S_DIGITAL,
	S_ANALOGICO
} teSensor;

typedef struct
{
	byte pin;
	bool estado;
} tsActuator, *tpsActuator;

typedef QueueList<tpsActuator> TManagerActuators;

typedef struct
{
	byte pin;		  // Pin asignado al sensor
	int valor;		  // Valor leido
	int valor_Df;	  // Valor Sensor por defecto
	bool Activo;	  // Activar/Desactivas sensor
	teSensor eSensor; // Tipos de sensor ANALÓGICO/DIGITAL
	int Aux;		  // Variables para ser usadas en la gestión del sensor
	String name;
	TNotifyEvent SensorEvent;
	TManagerActuators managerActuators;
} tsSensor, *ptsSensor;

typedef QueueList<ptsSensor> TManagerSensors;

typedef enum
{
	PER_STOP,
	PER_DOWN,
	PER_UP,
	PER_STOP2
} tsStaPer; // Control persianas

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define BUTTON1_P 3	   // Pulsador 1
#define BUTTON2_P 10   // Pulsador 2
#define BTN_OPT_P 11   // Entrada Optocoplada
#define RELE_P 4	   // RELE 1
#define PER_ONOFF_P 5  // ONOFF Persianas
#define PER_UPDOWN_P 6 // UPDOWN Persianas
#define TRIAC_P 7	   // ON/OFF Solid State Triac
#define EN_485 12	   // Bit to enable RS485 Conmunications
#define PIR_P 13	   // Sensor de Movimiento

#define PHOTORES_P A0  // Photo resistor SRC
#define TEMPSEN_P A2   // Sensor de temperatura
#define POT1_P A3	   // Potenciómetro 1
#define POT2_P A4	   // Potenciómetro 2
#define PHOTOTTOR_P A5 // Sensor de luminosidad (PhotoTransistor)

/****************************************************************************/
/***        Exported Class                                                ***/
/****************************************************************************/
class DomoBoard
{
private:
	TManagerSensors listSensors;

public:
	DomoBoard();			 // Constructor
	virtual ~DomoBoard() {}; // Destructor

	// Sensores Digitales
	tsSensor BOTON1;
	tsSensor BOTON2;
	tsSensor BTN_OPT;
	tsSensor PIR_MOV;

	// Sensores Analógicos
	tsSensor POT1;		// Potenciómetro 1
	tsSensor POT2;		// Potenciómetro 2
	tsSensor PHOTORES;	// Photoresistor SRC
	tsSensor TEMPSEN;	// Sensor de temperatura
	tsSensor PHOTOTTOR; // Foto transistor

	// Salidas
	tsActuator RELE;
	tsActuator TRIAC;
	tsActuator PER_ONOFF;
	tsActuator PER_UPDOWN;

	void leerAllSensor(void);
	void leerSensor(ptsSensor Sensor);
	void SetPersiana(tsStaPer staPer);
	void SetGaraje(tsStaPer staPer);

	virtual void Clear_SensorsConfiguration();

	static void setActuator(tsActuator *Actuator, bool val);
	static void manageSensorActuators(TManagerActuators *managerActuators, bool val);
	static void RS485_RxTx(TRX485_rxtx rxtx);
	AsyncWait garajeWait;
};

/*********************************************************************/
/***                    Prototipos de funciones                    ***/
/*********************************************************************/
void leerSensores(void);

/****************************************************************/
/***                    Exported Variables                    ***/
/****************************************************************/
// extern DomoBoard domoboard;

#endif /* DOMOBOARD_H_ */
