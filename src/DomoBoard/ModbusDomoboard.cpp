/*
 * ModbusDomoboard.cpp
 *
 *  Created on: 11 mar. 2020
 *      Author: jctejero
 *
 *  Modified on: 17/03/2025
 */

#include "ModbusDomoboard.h"
#include "utils_domoboard.h"
#include "debuglog.h"

/****************************************************************************/
/***        Variables Locales                                             ***/
/****************************************************************************/

/***************************************************
 * Definición Bancos de registros usados en ModBus *
 ***************************************************/
uint16_t	Cregs[MB_O_COILS];		//Registros para "Dicrete Output Coils"
uint16_t	Dregs[MB_I_CONTATCS];	//Registros para "Dicrete Input Contacts"
uint16_t	Aregs[MB_A_REGS];		//Registros para "Analog Output Holding Registers"
uint16_t	Iregs[MB_I_REGS];		//Registros para "Analog Input Registers"

ModbusDomoboard mbDomoboard;

ModbusDomoboard::ModbusDomoboard():DomoBoard() {
	//*****  Initialize ModBus Sensors  ****

	//Initialize BOTON1 for ModBus
	BOTON1.Sensor = &(DomoBoard::BOTON1);
	BOTON1.mbReg = &Dregs[MB_BOTON1];
	*(BOTON1.mbReg) = BOTON1.Sensor->valor;		//Actualizamos el registro ModBus con el estado del sensor

	listmbSensors.push(&BOTON1);

	//Initialize BOTON2 for ModBus
	BOTON2.Sensor = &(DomoBoard::BOTON2);
	BOTON2.mbReg = &Dregs[MB_BOTON2];
	*(BOTON2.mbReg) = BOTON2.Sensor->valor;		//Actualizamos el registro ModBus con el estado del sensor

	listmbSensors.push(&BOTON2);


	//Initialize Pulsador Optocoplado for ModBus
	BTN_OPT.Sensor = &(DomoBoard::BTN_OPT);
	BTN_OPT.mbReg = &Dregs[MB_BTNOPT];
	*(BTN_OPT.mbReg) = BTN_OPT.Sensor->valor;	//Actualizamos el registro ModBus con el estado del sensor

	listmbSensors.push(&BTN_OPT);

	//Inicializamos el registros ModBus Sensor PIR
	PIR_MOV.Sensor 			= &(DomoBoard::PIR_MOV);
	PIR_MOV.Sensor->Activo 	= false;					    //Inicialmente, Configuramos el sensor como no activo
	Cregs[MB_ACTPIR] 		= 0x00; 					//Actualizamos el registro ModBus que monitoriza el PIR
	PIR_MOV.mbReg 			= &Dregs[MB_PIRMOV];
	Dregs[MB_PIRMOV] 		= PIR_MOV.Sensor->valor;	//Actualizamos el registro ModBus con el estado del sensor
	Aregs[MB_TMP_PIR] 		= 0x03;						//Configuración inicial 3 Segundos activo

	listmbSensors.push(&PIR_MOV);

	//Initialize ModBus Analog sensors
	POT1.Sensor = &(DomoBoard::POT1);
	POT1.mbReg = &Iregs[MB_POT1];

	listmbSensors.push(&POT1);

	POT2.Sensor = &(DomoBoard::POT2);
	POT2.mbReg = &Iregs[MB_POT2];
	
	listmbSensors.push(&POT2);

	PHOTORES.Sensor			=   &(DomoBoard::PHOTORES);
	PHOTORES.mbReg			=	&Iregs[MB_PHOTORES];

	listmbSensors.push(&PHOTORES);

	TEMPSEN.Sensor			=   &(DomoBoard::TEMPSEN);
	TEMPSEN.mbReg			=	&Iregs[MB_TEMPSEN];
	TEMPSEN.mbSensorEvent	=   Calc_Temperatura;

	//listmbSensors.push(&TEMPSEN);

	PHOTOTTOR.Sensor		=   &(DomoBoard::PHOTOTTOR);
	PHOTOTTOR.mbReg			=	&Iregs[MB_PHOTOTTOR];

	listmbSensors.push(&PHOTOTTOR);

	RELE.actuator 	= &(DomoBoard::RELE);
	RELE.mbReg 		= &Cregs[MB_RELE];

	TRIAC.actuator 	= &(DomoBoard::TRIAC);
	TRIAC.mbReg 	= &Cregs[MB_TRIAC];

	// Configuración del estado inicial de los actuadores de la persiana

	// Asociamos el puntero del actuador PERUP (subir persiana) al registro de control MB_PERUP.
	// De esta forma, el sistema sabrá qué registro ModBus consultar para saber si debe subir la persiana.
	PERUP.mbReg = &Cregs[MB_PERUP];
	// Inicializamos el valor del registro MB_PERUP en OFF (estado inactivo)
	Cregs[MB_PERUP] = OFF;

	// Asociamos el puntero del actuador PERDOWN (bajar persiana) al registro de control MB_PERDOWN.
	// Esto permite al sistema controlar la bajada de la persiana leyendo/escribiendo ese registro.
	PERDOWN.mbReg = &Cregs[MB_PERDOWN];
	// Inicializamos el valor del registro MB_PERDOWN en OFF (estado inactivo)
	Cregs[MB_PERDOWN] = OFF;
	Aregs[MB_TMP1_GARAJE] 		= 0x01;						//Configuración inicial 1 Segundos activo
	Aregs[MB_TMP5_GARAJE] 		= 0x05;						//Configuración inicial 5 Segundos activo
}

void ModbusDomoboard::leerAllSensor(void){
	for(uint8_t i = 0; i < listmbSensors.count(); i++){
		leerSensor(listmbSensors.peek(i));
	}
}

void ModbusDomoboard::leerSensor(TpmbSensor Sensor){

	DomoBoard::leerSensor(Sensor->Sensor);
	
	//compueba si el valor leído por el sensor difiere del valor almacenado en el registro correspondiente
	//del banco de registros
	if((int16_t)(*(Sensor->mbReg)) != Sensor->Sensor->valor){
		//Estado Sensor ha cambiado


		//Se actualiza el registro correspondiente con el nuevo valor leído en el sensor.
		*(Sensor->mbReg) = Sensor->Sensor->valor;
		//Se inícia el evento asociado a la actualización del banco de registro correpondiente
		if(Sensor->mbSensorEvent != NULL){
			Sensor->mbSensorEvent(Sensor);
		}
	}else{
		if((Sensor->asyncWait != NULL) && (Sensor->mbSensorEvent != NULL)){   //Para gestión de acciones temporizadas
			Sensor->mbSensorEvent(Sensor);
		}
	}
}

void	ModbusDomoboard::Clear_SensorsConfiguration(){
	DomoBoard::Clear_SensorsConfiguration();

	for(uint8_t i = 0; i < listmbSensors.count(); i++){
		listmbSensors.peek(i)->mbActuators.clear();
		listmbSensors.peek(i)->mbSensorEvent 	= NULL;
		listmbSensors.peek(i)->asyncWait 		= NULL;
		listmbSensors.peek(i)->RelatedSensors 	= NULL;
	}	

}

void  ModbusDomoboard::setmbActuator(TmbActuator *Actuator, TStateDigitalDev val){
	bool newVal = (bool)val;

	if(val == TOGGLE){		
		newVal = (*Actuator->mbReg) > 0 ? false : true;
	}

	if(*(Actuator->mbReg) != newVal){
		*(Actuator->mbReg) = newVal;

		setActuator(Actuator->actuator, *(Actuator->mbReg));

	}
}

void ModbusDomoboard::manager_mbActuators(TmbActuators *Actuators, TStateDigitalDev val){
	for(int n = 0; n < Actuators->count(); n++)
		setmbActuator(Actuators->peek(n), val);

}

void ModbusDomoboard::set_coilRegister(uint16_t addReg, uint16_t value){
	Cregs[addReg] = value;
}

void leeSensoresmb(void){
	mbDomoboard.leerAllSensor();
}
