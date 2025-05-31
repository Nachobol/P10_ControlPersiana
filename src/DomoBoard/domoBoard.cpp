/*
 * domoBoard.cpp
 *
 *  Created on: 09/03/2015
 *      Author: jctejero
 * 
 * 	Modified on: 13/05/2025
 * 
 * 	Descripción:
 * 	Clase que gestiona la placa de prácticas de la asignatura de Domótica.
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "domoBoard.h"
#include "debuglog.h"
#include "ModbusDomoboard.h"


// Constructors ////////////////////////////////////////////////////////////////

DomoBoard::DomoBoard()
{
	//Definimos pin's DomoBoard.
	pinMode(BUTTON1_P, INPUT);
	pinMode(BUTTON2_P, INPUT);
	pinMode(BTN_OPT_P, INPUT);      	//Pin Entrada Optocoplada
	pinMode(PIR_P, INPUT);      	

	pinMode(RELE_P, OUTPUT);
	pinMode(TRIAC_P, OUTPUT);
	// Configura el pin PER_ONOFF_P como salida digital.
	// Este pin se usará para activar o desactivar la persiana (alimentación o relé principal).
	pinMode(PER_ONOFF_P, OUTPUT);

	// Configura el pin PER_UPDOWN_P como salida digital.
	// Este pin se usará para definir la dirección de movimiento de la persiana:
	// por ejemplo, HIGH para subir, LOW para bajar (según la lógica del hardware).
	pinMode(PER_UPDOWN_P, OUTPUT);

	BOTON1.pin 			= BUTTON1_P;
	BOTON1.valor 		= digitalRead(BUTTON1_P);		//Leer Valor por defecto
	BOTON1.valor_Df 	= BOTON1.valor;
	BOTON1.Activo 		= true;
	BOTON1.eSensor  	= S_DIGITAL;
	BOTON1.name			= F("BOTÓN 1");

	BOTON2.pin 			= BUTTON2_P;
	BOTON2.valor 		= digitalRead(BUTTON2_P);
	BOTON2.valor_Df 	= BOTON2.valor;
	BOTON2.Activo 		= true;
	BOTON2.eSensor  	= S_DIGITAL;
	BOTON2.name			= F("BOTÓN 2");
	
	BTN_OPT.pin 		= BTN_OPT_P;
	BTN_OPT.valor 		= digitalRead(BTN_OPT_P);
	BTN_OPT.valor_Df 	= BTN_OPT.valor;
	BTN_OPT.Activo 		= true;
	BTN_OPT.eSensor  	= S_DIGITAL;
	BTN_OPT.name		= F("Pulsador Optocoplado");

	//Inicialización del sensor de movimiento
	PIR_MOV.pin 		= PIR_P;
	PIR_MOV.valor 		= digitalRead(PIR_P);
	PIR_MOV.valor_Df 	= PIR_MOV.valor;  
	PIR_MOV.Activo 		= true;
	PIR_MOV.eSensor 	= S_DIGITAL;
	PIR_MOV.name 		= F("PIR (Sensor de Movimiento)");

	//==================================
	//Inicialización Sensores Analógicos
	//==================================
	POT1.pin 			= POT1_P;
	POT1.Activo 		= true;
	POT1.eSensor 		= S_ANALOGICO;
	POT1.name 			= F("POT 1");

	POT2.pin 			= POT2_P;
	POT2.Activo 		= true;
	POT2.eSensor 		= S_ANALOGICO;
	POT2.name 			= F("POT 2");

	PHOTORES.pin 		= PHOTORES_P;
	PHOTORES.Activo 	= true;
	PHOTORES.eSensor 	= S_ANALOGICO;
	PHOTORES.name 		= F("PHOTO RESISTOR");

	TEMPSEN.pin 		= TEMPSEN_P;
	TEMPSEN.Aux 		= -1;
	TEMPSEN.Activo 		= true;
	TEMPSEN.eSensor 	= S_ANALOGICO;
	TEMPSEN.name 		= F("Sensor Temperatura");

	PHOTOTTOR.pin 		= PHOTOTTOR_P;
	PHOTOTTOR.Activo 	= true;
	PHOTOTTOR.eSensor 	= S_ANALOGICO;
	PHOTOTTOR.name 		= F("PHOTO TRANSISTOR");

	//==================================
	//Inicialización Actuadores
	//==================================
	RELE.pin 			= RELE_P;
	RELE.estado 		= LOW;

	TRIAC.pin 			= TRIAC_P;
	TRIAC.estado 		= LOW;

	// Asigna al objeto PER_ONOFF el pin físico que controla el encendido/apagado de la persiana
	PER_ONOFF.pin = PER_ONOFF_P;
	// Inicializa el estado de PER_ONOFF en LOW (apagado o inactivo)
	PER_ONOFF.estado = LOW;

	// Asigna al objeto PER_UPDOWN el pin físico que controla la dirección de la persiana (subir o bajar)
	PER_UPDOWN.pin = PER_UPDOWN_P;
	// Inicializa el estado de PER_UPDOWN en LOW (por ejemplo, bajando o dirección neutra, según el hardware)
	PER_UPDOWN.estado = LOW;


	listSensors.push(&BOTON1);
	listSensors.push(&BOTON2);
	listSensors.push(&BTN_OPT);
	listSensors.push(&PIR_MOV);
	listSensors.push(&POT1);
	listSensors.push(&POT2);
	listSensors.push(&PHOTORES);
	listSensors.push(&TEMPSEN);	
	listSensors.push(&PHOTOTTOR);
}

void DomoBoard::leerAllSensor(void){
	for(uint8_t i = 0; i < listSensors.count(); i++){
		leerSensor(listSensors.peek(i));
	}
}


void  DomoBoard::leerSensor(ptsSensor Sensor){
	int valor = 0;

	if(Sensor->Activo){
		switch (Sensor->eSensor)
		{
		case S_DIGITAL:
			valor = digitalRead(Sensor->pin);
			break;

		case S_ANALOGICO:
			valor = analogRead(Sensor->pin);			

			break;
		}

		if(Sensor == &TEMPSEN){
			//DEBUGLOGLN("Sensor Temperatura: %d", valor)
		}

		if(Sensor->valor != valor)
		{
			Sensor->valor = valor;

			//Si hay un cambio en el estado del sensor llamamos a la aplicación asociada
			if(Sensor->SensorEvent != NULL)
				Sensor->SensorEvent(Sensor);
		}
	}
}

// Función miembro de la clase DomoBoard que controla el estado físico de la persiana
// en función del estado lógico `staPer` recibido (subiendo, bajando, parada, etc.)
void DomoBoard::SetPersiana(tsStaPer staPer)
{
	switch(staPer) {
	case PER_STOP:
	case PER_STOP2:
		// En ambos casos de parada (normal o tras cambio de dirección),
		// se apaga el motor (PER_ONOFF) y se deja la dirección en estado neutro (PER_UPDOWN).
		setActuator(&PER_ONOFF, false);   // Desactiva la persiana (corta la alimentación o desactiva el relé)
		setActuator(&PER_UPDOWN, false);  // Dirección en estado bajo (por seguridad o neutralidad)
		break;

	case PER_DOWN:
		// Para bajar la persiana:
		// Establece la dirección de bajada (LOW) y activa el motor.
		setActuator(&PER_UPDOWN, false);  // Dirección de bajada
		setActuator(&PER_ONOFF, true);    // Activa el motor
		break;

	case PER_UP:
		// Para subir la persiana:
		// Establece la dirección de subida (HIGH) y activa el motor.
		setActuator(&PER_UPDOWN, true);   // Dirección de subida
		setActuator(&PER_ONOFF, true);    // Activa el motor
		break;
	}
}

void DomoBoard::SetGaraje(tsStaPer staPer)
{
	static AsyncWait asyncWait;
	switch(staPer) {
	case PER_STOP:
	
		//activo timer de 5 segundos y luego bajo
		setActuator(&PER_ONOFF, false);   // Desactiva la persiana (corta la alimentación o desactiva el relé)
		asyncWait.startWaiting(&Aregs[MB_TMP5_GARAJE]);		//Configuramos el tiempo de espera al tiempo almacenado
		//activo cronometro de 5 segundos y luego sigo
		setActuator(&PER_ONOFF, true);   // Desactiva la persiana (corta la alimentación o desactiva el relé)
	case PER_STOP2:
	
		// En ambos casos de parada (normal o tras cambio de dirección),
		// se apaga el motor (PER_ONOFF) y se deja la dirección en estado neutro (PER_UPDOWN).
		setActuator(&PER_ONOFF, false);   // Desactiva la persiana (corta la alimentación o desactiva el relé)
		setActuator(&PER_UPDOWN, false);  // Dirección en estado bajo (por seguridad o neutralidad)
		asyncWait.startWaiting(&Aregs[MB_TMP1_GARAJE]);		//Configuramos el tiempo de espera al tiempo almacenado
		//activo cronometro de 1 y luego sigo	
		break;

	case PER_DOWN:
		// Para bajar la persiana:
		// Establece la dirección de bajada (LOW) y activa el motor.
		setActuator(&PER_UPDOWN, false);  // Dirección de bajada
		setActuator(&PER_ONOFF, true);    // Activa el motor
		break;

	case PER_UP:
		// Para subir la persiana:
		// Establece la dirección de subida (HIGH) y activa el motor.
		setActuator(&PER_UPDOWN, true);   // Dirección de subida
		setActuator(&PER_ONOFF, true);    // Activa el motor
		break;
	}
}

void  DomoBoard::setActuator(tpsActuator Actuator, bool val){
	if(Actuator->estado != val){
		digitalWrite(Actuator->pin, val);
		Actuator->estado = val;
	}
}

void leerSensores(void){
	//domoboard.leerAllSensor();
}

void DomoBoard::manageSensorActuators(TManagerActuators *managerActuators, bool val){
	if(managerActuators->count() > 0){
		for(uint8_t i = 0; i < managerActuators->count(); i++){
			DomoBoard::setActuator(managerActuators->peek(i), val);
		}
	}
}

void DomoBoard::Clear_SensorsConfiguration(){
	ptsSensor sensor;
	for(int i = 0; i < listSensors.count(); i++){
		sensor = listSensors.peek(i);
		sensor->managerActuators.clear();
		sensor->SensorEvent = NULL;
	};

}

void DomoBoard::RS485_RxTx(TRX485_rxtx rxtx){
	switch(rxtx){
	case RX485_RX:
		while (!(UCSR0A & (1 << TXC0)));
		digitalWrite(EN_485, LOW);
		break;

	case RX485_TX:
		digitalWrite(EN_485, HIGH);
		delay(1);
		break;
	}
}

//DomoBoard domoboard;

