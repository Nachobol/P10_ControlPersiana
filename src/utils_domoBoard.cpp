/*
 * utils_domoBoard.cpp
 *
 *  Created on: 10/03/2015
 *      Author: jctejero
 * 
 *  modified on: 01/04/2025
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "utils_domoBoard.h"
#include "debuglog.h"

//#define DEBUG_TEMP

/*************************************************************************/
/***                   Constantes Control Persiana                     ***/
/*************************************************************************/

#define	STEP_CTRL_BLIND		5
#define PERCENTAGE(x)		((x % STEP_CTRL_BLIND) > 2) ? x + (STEP_CTRL_BLIND-(x % STEP_CTRL_BLIND)) : x - (x % STEP_CTRL_BLIND);

#define V_PERCENTAGE(x)    PERCENTAGE((((TPCtrlTime)x)->actTime)*100/(*((TPCtrlTime)x)->maxTime));

const uint16_t	persianaTiempoParada 	= 1; //Seg.
//const uint16_t	persianaTiempoAbierta 	= 5; //seg.

/************************************************************************/
/***                   Variables Control Persiana                     ***/
/************************************************************************/
TCtrlTime 	ctrlPosPer;			//Control de posición persiana

/*============================================*/
/*		 		  INTERRUPTOR                 */
/*============================================*/
void Interruptor(void *Sensor)
{
	if(((ptsSensor)Sensor)->valor_Df == ((ptsSensor)Sensor)->valor){
		DEBUG(((ptsSensor)Sensor)->name);
		DEBUG(F(" : Interruptor --> "));
		if(((ptsSensor)Sensor)->Aux == OFF){
			((ptsSensor)Sensor)->Aux = ON;
			DEBUGLNF("ON");
		}else{
			((ptsSensor)Sensor)->Aux = OFF;
			DEBUGLNF("OFF");
		}
	}
}

/*============================================*/
/*		 		    PULSADOR                  */
/*============================================*/
void Pulsado_Soltado(void *Sensor){
	ptsSensor sensor = reinterpret_cast<ptsSensor>(Sensor);

	DEBUG(sensor->name);
	if(sensor->valor_Df == sensor->valor){
		DEBUGLNF(" --> Soltado");
		sensor->Aux = OFF;
	}else{
		DEBUGLNF(" --> Pulsado");
		sensor->Aux = ON;
	}
}

/*============================================*/
/*		     	   CONMUTADOR                 */
/*============================================*/
void conmutador(void *Sensor)
{
	static bool valor = OFF;

	ptsSensor sensor = reinterpret_cast<ptsSensor>(Sensor);

	if(sensor->valor_Df == sensor->valor){
		DEBUG(F("Conmutador --> "));
		if(valor == OFF){
			valor = ON;
			DEBUGLN(F("ON"));
		}else{
			valor = OFF;
			DEBUGLN(F("OFF"));
		}
	}

	sensor->Aux = valor;
}

/*======================================================================*/
/*		 		    PULSADOR CON ACTIVACIÓN DE SALIDAS                  */
/*======================================================================*/
void Pulsado_Soltado_sal(void *Sensor)
{
	Pulsado_Soltado(Sensor);

	//Actualiza Actuadores
	for(uint8_t i = 0; i < ((ptsSensor)Sensor)->managerActuators.count(); i++){
		DomoBoard::setActuator(((ptsSensor)Sensor)->managerActuators.peek(i), ((ptsSensor)Sensor)->Aux);
	}
}

/*=========================================================================*/
/*		 		    INTERRUPTOR CON ACTIVACIÓN DE SALIDAS                  */
/*=========================================================================*/
void interruptor_sal(void *Sensor)
{
	Interruptor(Sensor);

	//Actualiza Actuadores
	for(uint8_t i = 0; i < ((ptsSensor)Sensor)->managerActuators.count(); i++){
		DomoBoard::setActuator(((ptsSensor)Sensor)->managerActuators.peek(i), ((ptsSensor)Sensor)->Aux);
	}
}


/*======================================================================*/
/*		     	   CONMUTADOR CON ACTIVACIÓN DE SALIDAS                 */
/*======================================================================*/
void conmutador_sal(void *Sensor)
{
	conmutador(Sensor);

	//Actualiza Actuadores
	for(uint8_t i = 0; i < ((ptsSensor)Sensor)->managerActuators.count(); i++){
		DomoBoard::setActuator(((ptsSensor)Sensor)->managerActuators.peek(i), ((ptsSensor)Sensor)->Aux);
	}
}

/*============================================*/
/*		      INTERRUPTOR MODBUS              */
/*============================================*/
void mbInterruptor(void *mbSensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(mbSensor);

	if(sensor->Sensor->valor_Df == sensor->Sensor->valor){
		mbDomoboard.manager_mbActuators(&(sensor->mbActuators), TOGGLE);
	}
}

/*============================================*/
/*		 	   CONMUTADOR MODBUS              */
/*============================================*/
void mbConmutador(void *mbSensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(mbSensor);

	if(sensor->Sensor->valor_Df == sensor->Sensor->valor){
		//Estado del sensor ha cambiado
		mbDomoboard.manager_mbActuators(&(sensor->mbActuators), TOGGLE);
	}
}

/*========================================================*/
/*		      INTERRUPTOR TEMPORIZADO MODBUS              */
/*========================================================*/

/*
 * Este sensor será activado mediante Sensor (Variable de control) y aunque se vuelva inactivo,
 * el interruptor, permanecerá activo durante el tiempo que indique el temporizador.
 * 
 * Una vez el interruptor ha sido activado, este permanecerá activo mediante la variable de control asyncWait.
 * 
 * Cada vez que el sensor se activa, se reinicia el temporizador (asyncWait). De esta forma, el tiempo programado
 * para la desactivación del sensor siempre se computará desde la última vez que el sensor se activó.
 *
 */

 void	mbInterruptorTemporizado(void *Sensor){

	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	if(sensor->Sensor->valor_Df != sensor->Sensor->valor){
		sensor->asyncWait->restart();

		mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)ON);

	}else{
		if(!sensor->asyncWait->isWaiting() && !sensor->asyncWait->isVerified()){

			DEBUGLNF("ASYNWAIT TERMINADO");

			sensor->asyncWait->setVerified();
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)OFF);
		}
	}

}

/*==============================================*/
/*		      Calcular Temperatura              */
/*==============================================*/
//mas/menos 1 - 2ºC de exactitud para el TMP36, por no obtenemos valores decimales
void Calc_Temperatura(void *Sensor){
	float valTMP;
	int   temp;

	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	valTMP = sensor->Sensor->valor*.004882812;	//Conviere resultado convertidor a voltios

	valTMP = (valTMP - .5)*100;          //Convierte Temperatura de 10 mV por grado con 500 mV de Offset


	temp = (valTMP - (int)valTMP)*100;

    if(temp < 50) temp = 0;
    //else if ((25 < temp)&&(temp < 75)) temp = 5;
    //else if ((75 < temp)&&(temp <= 99)){
    else{
    	temp = 0;
    	valTMP = (int)valTMP +1;
    }

	*(sensor->mbReg) = (((int)valTMP & 0xff) << 8) | (temp & 0xff);

#ifdef DEBUG_TEMP
    static float vT = 0;

    if(vT != valTMP){
    	vT = valTMP;
    	/*
    	DEBUGF("Temperatura = " );
    	Serial.print((int)valTMP,DEC);
    	Serial.print(".");
    	Serial.println(temp,DEC);
    	*/
    	DEBUGLOG("Temperatura = %d.%d\n", (int)valTMP, temp);

    //	Serial.print("Temperatura F -> ");
    //	Serial.println(Sensor->Regs_App[Sensor->MBReg_App],BIN);
    }
#endif

}

/*============================================*/
/*			  INTERRUPTOR LUMINOSIDAD         */
/*============================================*/
/*
 * Interruptor por nivel de luminosidad. funcionará con una histeresis, es decir,
 * Si el interruptor está desactivado, se activará cuando alcance el "highlevel".
 *
 * Si el interruptor está ativado, se desactivará cuando alcanze el "lowlevel".
 */
void	interruptor_SRC(void *Sensor){
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	if((int)*(sensor->ctrlLevelPtr.lInf) > (int)*(sensor->ctrlLevelPtr.lSup))
		return;

	int state = sensor->Sensor->Aux;

	switch(state){
	case 1:
		if(sensor->Sensor->valor <= (int)*(sensor->ctrlLevelPtr.lInf)){
			state = 2;
			//Estado del sensor ha cambiado
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)ON);
		}

		break;

	case 2:
		if(sensor->Sensor->valor >= (int)*(sensor->ctrlLevelPtr.lSup)){
			state = 1;
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)OFF);
		}

		break;

	default:
		state = 2;
		mbDomoboard.set_coilRegister(MB_TRIAC, OFF);
	}

	sensor->Sensor->Aux = state;
}

/*============================================*/
/*			  TRIGGER WITH LUMINOSIDAD        */
/*============================================*/
/*
 * tRIGGER por nivel de luminosidad. Dispara un evento cuando el nivel del sensor
 * se encuentra por debajo de un determinado nivel
 */
void	trigger_Level(void *Sensor){
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	uint16_t limInfe = *(sensor->ctrlLevelPtr.lInf);

	if(((uint16_t) sensor->Sensor->valor <= limInfe)){
		if(sensor->Sensor->Aux == 0){
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)ON);
			sensor->Sensor->Aux = 1;
		}
	} else sensor->Sensor->Aux = 0;
}
/*============================================*/

/*============================================*/
/*			    CONTROL DE PERSIANA           */
/*============================================*/


void Ctrl_PosicionPersiana(TPCtrlTime ctrlPosPer, tsStaPer staPer){
	static tsStaPer lastStePer = PER_STOP;

	// lambda function
	auto actualizeTime = [&](int8_t sign){

		ctrlPosPer->actTime = ctrlPosPer->actTime + sign*(millis() - ctrlPosPer->lastAct);

		if(ctrlPosPer->actTime < 0){
			ctrlPosPer->actTime = 0;
		}else if(uint16_t(ctrlPosPer->actTime) > *ctrlPosPer->maxTime){
			ctrlPosPer->actTime = *ctrlPosPer->maxTime;
		}

		uint8_t per = PERCENTAGE((ctrlPosPer->actTime)*100/(*ctrlPosPer->maxTime));

		uint8_t per2 = V_PERCENTAGE(ctrlPosPer);

		//DEBUGLOGLN("POS PER: %d - %d", per, per2);

		if(per != Iregs[MB_POSPER]){
			Iregs[MB_POSPER] = per;
		}
	};

	switch(staPer){
	case PER_STOP:
	case PER_STOP2:
		ctrlPosPer->activa = false;
		switch(lastStePer){
		case PER_DOWN:
			actualizeTime(-1);
			break;
		case PER_UP:
			actualizeTime(1);
			break;
		default:
			break;
		}
		break;

	case PER_DOWN:
		if(ctrlPosPer->activa){
			//Continúa bajado
			actualizeTime(-1);
		}else{
			//Comienza a bajada
			ctrlPosPer->activa = true;
		}
		ctrlPosPer->lastAct = millis();
		break;

	case PER_UP:
		if(ctrlPosPer->activa){
			//Continúa subiendo
			actualizeTime(1);
		}else{
			//Comienza a subir
			ctrlPosPer->activa = true;
		}
		ctrlPosPer->lastAct = millis();
		break;
	}

	lastStePer = staPer;
}

void update_PersianaState(){
	//Actualiza posición Persiana
	Ctrl_PosicionPersiana(&ctrlPosPer, tsStaPer(Aregs[MB_STAPER]&0xFF));

	mbDomoboard.SetPersiana(tsStaPer(Aregs[MB_STAPER]&0xFF));
}

void UpDown_Persiana(){
	bool UpP,DownP;
	tsStaPer state = (tsStaPer)(Aregs[MB_STAPER]&0xFF);

	UpP = (bool)(*mbDomoboard.PERUP.mbReg);
	DownP = (bool)(*mbDomoboard.PERDOWN.mbReg);

    switch(state){
    	case PER_STOP: //Parada
    		if (UpP == ON) {
    			state = PER_UP; //Subiendo
    		}

    		if (DownP == ON) {
    			state = PER_DOWN; //Bajando
    		}

    		break;

    	case PER_UP: //Subiendo
    		if(!UpP && !DownP) {
    			state = PER_STOP;
    		}

    		if(!UpP && DownP){
    			state = PER_STOP2;
    		}
    		break;

    	case PER_DOWN: //Bajando
    		if(!UpP && !DownP) {
    			state = PER_STOP;
    		}

    		if(UpP && !DownP){
    			state = PER_STOP2;
    		}
    		break;

    	case PER_STOP2:
    		if(!UpP && !DownP) {
    			state = PER_STOP;
    		}
    		break;

    }

    if(Aregs[MB_STAPER] != state){
    	//mbDomoboard.set_holdingRegister(MB_STAPER, state);
    	Aregs[MB_STAPER] = state;

    	update_PersianaState();
    }
}

void Persiana(void *Sensor){

	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	//Se asigna un estado al actuador, en función del estado del sensor
	for(int n = 0; n < sensor->mbActuators.count(); n++){
		*(sensor->mbActuators.peek(n))->mbReg = (sensor->Sensor->valor != sensor->Sensor->valor_Df);
	}

	UpDown_Persiana();
}





