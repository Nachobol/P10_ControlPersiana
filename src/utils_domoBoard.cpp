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
#include "./DomoBoard/domoboard.h"

// #define DEBUG_TEMP

/*************************************************************************/
/***                   Constantes Control Persiana                     ***/
/*************************************************************************/

#define STEP_CTRL_BLIND 5
#define PERCENTAGE(x) ((x % STEP_CTRL_BLIND) > 2) ? x + (STEP_CTRL_BLIND - (x % STEP_CTRL_BLIND)) : x - (x % STEP_CTRL_BLIND);

#define V_PERCENTAGE(x) PERCENTAGE((((TPCtrlTime)x)->actTime) * 100 / (*((TPCtrlTime)x)->maxTime));

const uint16_t persianaTiempoParada = 1; // Seg.
// const uint16_t	persianaTiempoAbierta 	= 5; //seg.

/************************************************************************/
/***                   Variables Control Persiana                     ***/
/************************************************************************/
TCtrlTime ctrlPosPer; // Control de posición persiana

/*============================================*/
/*		 		  INTERRUPTOR                 */
/*============================================*/
void Interruptor(void *Sensor)
{
	if (((ptsSensor)Sensor)->valor_Df == ((ptsSensor)Sensor)->valor)
	{
		DEBUG(((ptsSensor)Sensor)->name);
		DEBUG(F(" : Interruptor --> "));
		if (((ptsSensor)Sensor)->Aux == OFF)
		{
			((ptsSensor)Sensor)->Aux = ON;
			DEBUGLNF("ON");
		}
		else
		{
			((ptsSensor)Sensor)->Aux = OFF;
			DEBUGLNF("OFF");
		}
	}
}

/*============================================*/
/*		 		    PULSADOR                  */
/*============================================*/
void Pulsado_Soltado(void *Sensor)
{
	ptsSensor sensor = reinterpret_cast<ptsSensor>(Sensor);

	DEBUG(sensor->name);
	if (sensor->valor_Df == sensor->valor)
	{
		DEBUGLNF(" --> Soltado");
		sensor->Aux = OFF;
	}
	else
	{
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

	if (sensor->valor_Df == sensor->valor)
	{
		DEBUG(F("Conmutador --> "));
		if (valor == OFF)
		{
			valor = ON;
			DEBUGLN(F("ON"));
		}
		else
		{
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

	// Actualiza Actuadores
	for (uint8_t i = 0; i < ((ptsSensor)Sensor)->managerActuators.count(); i++)
	{
		DomoBoard::setActuator(((ptsSensor)Sensor)->managerActuators.peek(i), ((ptsSensor)Sensor)->Aux);
	}
}

/*=========================================================================*/
/*		 		    INTERRUPTOR CON ACTIVACIÓN DE SALIDAS                  */
/*=========================================================================*/
void interruptor_sal(void *Sensor)
{
	Interruptor(Sensor);

	// Actualiza Actuadores
	for (uint8_t i = 0; i < ((ptsSensor)Sensor)->managerActuators.count(); i++)
	{
		DomoBoard::setActuator(((ptsSensor)Sensor)->managerActuators.peek(i), ((ptsSensor)Sensor)->Aux);
	}
}

/*======================================================================*/
/*		     	   CONMUTADOR CON ACTIVACIÓN DE SALIDAS                 */
/*======================================================================*/
void conmutador_sal(void *Sensor)
{
	conmutador(Sensor);

	// Actualiza Actuadores
	for (uint8_t i = 0; i < ((ptsSensor)Sensor)->managerActuators.count(); i++)
	{
		DomoBoard::setActuator(((ptsSensor)Sensor)->managerActuators.peek(i), ((ptsSensor)Sensor)->Aux);
	}
}

/*============================================*/
/*		      INTERRUPTOR MODBUS              */
/*============================================*/
void mbInterruptor(void *mbSensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(mbSensor);

	if (sensor->Sensor->valor_Df == sensor->Sensor->valor)
	{
		mbDomoboard.manager_mbActuators(&(sensor->mbActuators), TOGGLE);
	}
}

/*============================================*/
/*		 	   CONMUTADOR MODBUS              */
/*============================================*/
void mbConmutador(void *mbSensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(mbSensor);

	if (sensor->Sensor->valor_Df == sensor->Sensor->valor)
	{
		// Estado del sensor ha cambiado
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

// Función que gestiona un interruptor digital temporizado.
// Se activa cuando el sensor cambia su valor respecto al valor por defecto,
// y tras un tiempo sin cambios, apaga el actuador.

void mbInterruptorTemporizado(void *Sensor)
{

	// Reinterpreta el puntero genérico como un puntero del tipo TpmbSensor,
	// que contiene información del sensor, el actuador asociado y el temporizador.
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	// Verifica si el valor actual del sensor es diferente al valor por defecto,
	// lo que indica un cambio o activación del sensor.
	if (sensor->Sensor->valor_Df != sensor->Sensor->valor)
	{

		// Reinicia el temporizador asíncrono al detectar el cambio.
		sensor->asyncWait->restart();

		// Activa el actuador correspondiente (por ejemplo, encender una luz o un motor).
		mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)ON);
	}
	else
	{
		// Si no hay cambio en el valor del sensor (sigue en su estado por defecto)...

		// ...y además el temporizador ha terminado y aún no se ha verificado la acción final:
		if (!sensor->asyncWait->isWaiting() && !sensor->asyncWait->isVerified())
		{

			// Muestra un mensaje de depuración indicando que el temporizador ha concluido.
			DEBUGLNF("ASYNWAIT TERMINADO");

			// Marca el temporizador como verificado para evitar repetir esta acción.
			sensor->asyncWait->setVerified();

			// Desactiva el actuador (por ejemplo, apaga la luz).
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)OFF);
		}
	}
}

/*==============================================*/
/*		      Calcular Temperatura              */
/*==============================================*/
// mas/menos 1 - 2ºC de exactitud para el TMP36, por no obtenemos valores decimales
void Calc_Temperatura(void *Sensor)
{
	float valTMP;
	int temp;

	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	valTMP = sensor->Sensor->valor * .004882812; // Conviere resultado convertidor a voltios

	valTMP = (valTMP - .5) * 100; // Convierte Temperatura de 10 mV por grado con 500 mV de Offset

	temp = (valTMP - (int)valTMP) * 100;

	if (temp < 50)
		temp = 0;
	// else if ((25 < temp)&&(temp < 75)) temp = 5;
	// else if ((75 < temp)&&(temp <= 99)){
	else
	{
		temp = 0;
		valTMP = (int)valTMP + 1;
	}

	*(sensor->mbReg) = (((int)valTMP & 0xff) << 8) | (temp & 0xff);

#ifdef DEBUG_TEMP
	static float vT = 0;

	if (vT != valTMP)
	{
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
void interruptor_SRC(void *Sensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	if ((int)*(sensor->ctrlLevelPtr.lInf) > (int)*(sensor->ctrlLevelPtr.lSup))
		return;

	int state = sensor->Sensor->Aux;

	switch (state)
	{
	case 1:
		if (sensor->Sensor->valor <= (int)*(sensor->ctrlLevelPtr.lInf))
		{
			state = 2;
			// Estado del sensor ha cambiado
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)ON);
		}

		break;

	case 2:
		if (sensor->Sensor->valor >= (int)*(sensor->ctrlLevelPtr.lSup))
		{
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
// Función que evalúa si el valor de un sensor ha bajado por debajo de un umbral inferior.
// Si es así, activa el actuador correspondiente (una sola vez), y evita reactivarlo
// hasta que el valor vuelva a subir por encima del umbral.

void trigger_Level(void *Sensor)
{

	// Se convierte el puntero genérico recibido a un puntero de tipo TpmbSensor,
	// que contiene información del sensor, su configuración y su actuador.
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	// Se obtiene el valor del límite inferior desde el puntero lInf
	// que forma parte de la estructura ctrlLevelPtr.
	// Este límite se utiliza como umbral mínimo para decidir si se debe actuar.
	uint16_t limInfe = *(sensor->ctrlLevelPtr.lInf);

	// Se evalúa si el valor actual del sensor es menor o igual que el límite inferior.
	if (((uint16_t)sensor->Sensor->valor <= limInfe))
	{

		// Si el sensor está por debajo del límite, se verifica si aún no se ha actuado.
		// Esto se controla con el campo Aux, que funciona como una bandera de activación.
		if (sensor->Sensor->Aux == 0)
		{

			// Si no se ha actuado todavía, se activa el actuador asociado al sensor.
			// Por ejemplo, encender una bomba, una luz de aviso, una alarma, etc.
			mbDomoboard.manager_mbActuators(&(sensor->mbActuators), (TStateDigitalDev)ON);

			// Se establece Aux = 1 para evitar que el actuador se vuelva a activar
			// en cada ejecución de esta función mientras el valor siga por debajo del límite.
			sensor->Sensor->Aux = 1;
		}
	}
	else
	{
		// Si el valor del sensor ya NO está por debajo del límite inferior,
		// se reinicia la bandera Aux a 0 para permitir una futura activación.
		sensor->Sensor->Aux = 0;
	}
}
/*============================================*/

/*============================================*/
/*			    CONTROL DE PERSIANA           */
/*============================================*/

void trigger_Level_garaje(void *Sensor)
{

	// Se convierte el puntero genérico recibido a un puntero de tipo TpmbSensor,
	// que contiene información del sensor, su configuración y su actuador.
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	// Se obtiene el valor del límite inferior desde el puntero lInf
	// que forma parte de la estructura ctrlLevelPtr.
	// Este límite se utiliza como umbral mínimo para decidir si se debe actuar.
	uint16_t limInfe = *(sensor->ctrlLevelPtr.lInf);

	// Se evalúa si el valor actual del sensor es menor o igual que el límite inferior.
	if (((uint16_t)sensor->Sensor->valor <= limInfe))
	{

		// Si el sensor está por debajo del límite, se verifica si aún no se ha actuado.
		// Esto se controla con el campo Aux, que funciona como una bandera de activación.
		if (sensor->Sensor->Aux == 0)
		{

			mbDomoboard.SetGaraje(PER_STOP2);

			// Se establece Aux = 1 para evitar que el actuador se vuelva a activar
			// en cada ejecución de esta función mientras el valor siga por debajo del límite.
			sensor->Sensor->Aux = 1;
		}
	}
	else
	{
		// Si el valor del sensor ya NO está por debajo del límite inferior,
		// se reinicia la bandera Aux a 0 para permitir una futura activación.
		sensor->Sensor->Aux = 0;
	}
}

// Controla la posición de la persiana en función del estado (subiendo, bajando, parada, etc.)
void Ctrl_PosicionPersiana(TPCtrlTime ctrlPosPer, tsStaPer staPer)
{
	static tsStaPer lastStePer = PER_STOP; // Guarda el último estado para saber si se estaba subiendo o bajando

	// Función lambda que actualiza el tiempo de movimiento y el porcentaje de apertura de la persiana
	auto actualizeTime = [&](int8_t sign)
	{
		// Actualiza el tiempo activo sumando o restando según la dirección (1=sube, -1=baja)
		ctrlPosPer->actTime = ctrlPosPer->actTime + sign * (millis() - ctrlPosPer->lastAct);

		// Asegura que el tiempo activo esté entre 0 y el máximo permitido
		if (ctrlPosPer->actTime < 0)
		{
			ctrlPosPer->actTime = 0;
		}
		else if (uint16_t(ctrlPosPer->actTime) > *ctrlPosPer->maxTime)
		{
			ctrlPosPer->actTime = *ctrlPosPer->maxTime;
		}

		// Calcula el porcentaje actual de apertura de la persiana
		uint8_t per = PERCENTAGE((ctrlPosPer->actTime) * 100 / (*ctrlPosPer->maxTime));
		uint8_t per2 = V_PERCENTAGE(ctrlPosPer); // Alternativa (no se usa directamente aquí)

		// Si ha cambiado el porcentaje, actualiza el registro ModBus correspondiente
		if (per != Iregs[MB_POSPER])
		{
			Iregs[MB_POSPER] = per;
		}
	};

	// Según el nuevo estado de la persiana, se realiza una acción
	switch (staPer)
	{
	case PER_STOP:	// Se ha detenido
	case PER_STOP2: // Se ha detenido pero tras un cambio de dirección
		ctrlPosPer->activa = false;
		// Según el último estado activo, se actualiza el tiempo (último movimiento)
		switch (lastStePer)
		{
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

	case PER_DOWN: // Bajando
		if (ctrlPosPer->activa)
		{
			actualizeTime(-1); // Sigue bajando
		}
		else
		{
			ctrlPosPer->activa = true; // Comienza a bajar
		}
		ctrlPosPer->lastAct = millis(); // Actualiza el timestamp
		break;

	case PER_UP: // Subiendo
		if (ctrlPosPer->activa)
		{
			actualizeTime(1); // Sigue subiendo
		}
		else
		{
			ctrlPosPer->activa = true; // Comienza a subir
		}
		ctrlPosPer->lastAct = millis(); // Actualiza el timestamp
		break;
	}

	lastStePer = staPer; // Guarda el nuevo estado como último
}

// Controla la posición de la persiana en función del estado (subiendo, bajando, parada, etc.)
void Ctrl_PosicionGaraje(TPCtrlTime ctrlPosPer, tsStaPer staPer)
{
	static tsStaPer lastStePer = PER_STOP; // Guarda el último estado para saber si se estaba subiendo o bajando

	// Función lambda que actualiza el tiempo de movimiento y el porcentaje de apertura de la persiana
	auto actualizeTime = [&](int8_t sign)
	{
		// Actualiza el tiempo activo sumando o restando según la dirección (1=sube, -1=baja)
		ctrlPosPer->actTime = ctrlPosPer->actTime + sign * (millis() - ctrlPosPer->lastAct);

		// Asegura que el tiempo activo esté entre 0 y el máximo permitido
		if (ctrlPosPer->actTime < 0)
		{
			ctrlPosPer->actTime = 0;
		}
		else if (uint16_t(ctrlPosPer->actTime) > *ctrlPosPer->maxTime)
		{
			ctrlPosPer->actTime = *ctrlPosPer->maxTime;
		}

		// Calcula el porcentaje actual de apertura de la persiana
		uint8_t per = PERCENTAGE((ctrlPosPer->actTime) * 100 / (*ctrlPosPer->maxTime));
		uint8_t per2 = V_PERCENTAGE(ctrlPosPer); // Alternativa (no se usa directamente aquí)

		// Si ha cambiado el porcentaje, actualiza el registro ModBus correspondiente
		if (per != Iregs[MB_POSPER])
		{
			Iregs[MB_POSPER] = per;
		}
	};

	switch (staPer)
	{
	case PER_STOP:
		ctrlPosPer->activa = false;
		// Según el último estado activo, se actualiza el tiempo (último movimiento)
		switch (lastStePer)
		{
		case PER_DOWN:
			actualizeTime(-1);
			break;
		case PER_UP:
			actualizeTime(1);
			break;
		default:
			break;
		}
		break;		// Se ha detenido
	case PER_STOP2: // Se ha detenido pero tras un cambio de dirección //activar garaje  o fototransistor
		ctrlPosPer->activa = false;
		// Según el último estado activo, se actualiza el tiempo (último movimiento)
		switch (lastStePer)
		{
		case PER_DOWN:
			actualizeTime(-1);
			break;
		case PER_UP:
			actualizeTime(1);
			break;
		default:
			break;
		}
	case PER_DOWN: // Bajando
		if (ctrlPosPer->activa)
		{
			actualizeTime(-1); // Sigue bajando
		}
		else
		{
			ctrlPosPer->activa = true; // Comienza a bajar
		}
		ctrlPosPer->lastAct = millis(); // Actualiza el timestamp
		break;
	case PER_UP: // Subiendo
		if (ctrlPosPer->activa)
		{
			actualizeTime(1); // Sigue subiendo
		}
		else
		{
			ctrlPosPer->activa = true; // Comienza a subir
		}
		ctrlPosPer->lastAct = millis(); // Actualiza el timestamp
		break;
	}

	lastStePer = staPer; // Guarda el nuevo estado como último
}

// Actualiza el estado de la persiana en función del valor almacenado en el registro ModBus
void update_PersianaState()
{
	Ctrl_PosicionPersiana(&ctrlPosPer, tsStaPer(Aregs[MB_STAPER] & 0xFF)); // Aplica el estado actual
	mbDomoboard.SetPersiana(tsStaPer(Aregs[MB_STAPER] & 0xFF));			   // Llama a una función para reflejar el cambio
}

void update_garajeState()
{
	Ctrl_PosicionGaraje(&ctrlPosPer, tsStaPer(Aregs[MB_STAPER] & 0xFF)); // Aplica el estado actual
	mbDomoboard.SetGaraje(tsStaPer(Aregs[MB_STAPER] & 0xFF));			 // Llama a una función para reflejar el cambio
}
// Determina si la persiana debe subir, bajar o detenerse según entradas físicas (botones u otros sensores)
void UpDown_Persiana()
{
	bool UpP, DownP;
	tsStaPer state = (tsStaPer)(Aregs[MB_STAPER] & 0xFF); // Estado actual

	// Lee los registros ModBus que representan botones físicos o señales
	UpP = (bool)(*mbDomoboard.PERUP.mbReg);
	DownP = (bool)(*mbDomoboard.PERDOWN.mbReg);

	switch (state)
	{

	case PER_STOP:
		// Si la persiana está parada actualmente...
		if (UpP == ON)
		{
			// Si se pulsa el botón de subir, cambia el estado a PER_UP (subiendo)
			state = PER_UP;
		}
		if (DownP == ON)
		{
			// Si se pulsa el botón de bajar, cambia el estado a PER_DOWN (bajando)
			state = PER_DOWN;
		}
		break;

	case PER_UP:
		// Si la persiana está subiendo actualmente...
		if (!UpP && !DownP)
		{
			// Si se suelta el botón de subida y tampoco se pulsa el de bajada, se detiene la persiana
			state = PER_STOP;
		}
		if (!UpP && DownP)
		{
			// Si se suelta el de subir y se pulsa el de bajar, se cambia a PER_STOP2 (transición antes de invertir dirección)
			state = PER_STOP2;
		}
		break;

	case PER_DOWN:
		// Si la persiana está bajando actualmente...
		if (!UpP && !DownP)
		{
			// Si se sueltan ambos botones, se detiene
			state = PER_STOP;
		}
		if (UpP && !DownP)
		{
			// Si se pulsa el de subir mientras baja, cambia a PER_STOP2 (preparando cambio de dirección)
			state = PER_STOP2;
		}
		break;

	case PER_STOP2:
		// Estado intermedio usado para permitir inversión de dirección tras detenerse
		if (!UpP && !DownP)
		{
			// Si no se pulsa ningún botón tras el estado intermedio, vuelve al estado de parada normal
			state = PER_STOP;
		}
		break;
	}

	// Si el estado ha cambiado, actualiza el registro y ejecuta la actualización
	if (Aregs[MB_STAPER] != state)
	{
		Aregs[MB_STAPER] = state;
		update_PersianaState();
	}
}

void UpDown_Garaje()
{
	bool UpP, DownP;
	tsStaPer state = (tsStaPer)(Aregs[MB_STAPER] & 0xFF); // Estado actual

	// Lee los registros ModBus que representan botones físicos o señales
	UpP = (bool)(*mbDomoboard.PERUP.mbReg);
	DownP = (bool)(*mbDomoboard.PERDOWN.mbReg);
	static uint16_t umbralMinimoPHOTOTTOR = 200;

	// Configuración del sensor PHOTOTTOR
	switch (state)
	{

	case PER_STOP:
		// Si la persiana está parada actualmente...
		if (UpP == ON)
		{
			// Si se pulsa el botón de subir, cambia el estado a PER_UP (subiendo)
			state = PER_UP;
		}

		if (!mbDomoboard.garajeWait.isWaiting() && !mbDomoboard.garajeWait.isVerified())
		{
			mbDomoboard.garajeWait.setVerified(); // Marcamos que ya hicimos esta acción
			state = PER_DOWN;					  // Actualizamos el estado
		}
		break;

	case PER_UP:
		// Si la persiana está subiendo actualmente...
		// que siga subiendo
		break;

	case PER_DOWN:
		// Si la persiana está bajando actualmente...

		if (UpP && !DownP)
		{
			// Si se pulsa el de subir mientras baja, cambia a PER_STOP2 (preparando cambio de dirección)
			state = PER_STOP2;
		}
		// hay que añadir que si el fototransistor detecta luz, se detenga la persiana
		if (mbDomoboard.PHOTOTTOR.ctrlLevelPtr.lInf < &umbralMinimoPHOTOTTOR)
		{

			state = PER_STOP2;
		}
		break;

	case PER_STOP2:
		if (!mbDomoboard.garajeWait.isWaiting() && !mbDomoboard.garajeWait.isVerified())
		{
			mbDomoboard.garajeWait.setVerified(); // Marcamos que ya actuamos

			if (UpP)
			{

				state = PER_UP;
			}
			else if (DownP)
			{

				state = PER_DOWN;
			}
			else
			{
				state = PER_STOP; // Si no hay pulsación, queda parado
			}
			break;
		}
	}

	if (Aregs[MB_STAPER] != state)
	{
		Aregs[MB_STAPER] = state;
		update_garajeState();
	}
}
// Función principal que ejecuta la lógica de la persiana asociada a un sensor
void Persiana(void *Sensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	// Activa los actuadores si el valor del sensor es distinto del valor por defecto
	for (int n = 0; n < sensor->mbActuators.count(); n++)
	{
		*(sensor->mbActuators.peek(n))->mbReg = (sensor->Sensor->valor != sensor->Sensor->valor_Df);
	}

	// Llama a la función que decide si subir, bajar o parar según estado
	UpDown_Persiana();
}

void Garaje(void *Sensor)
{
	TpmbSensor sensor = reinterpret_cast<TpmbSensor>(Sensor);

	// Activa los actuadores si el valor del sensor es distinto del valor por defecto
	for (int n = 0; n < sensor->mbActuators.count(); n++)
	{
		*(sensor->mbActuators.peek(n))->mbReg = (sensor->Sensor->valor != sensor->Sensor->valor_Df);
	}

	// Llama a la función que decide si subir, bajar o parar según estado
	UpDown_Garaje();
}
