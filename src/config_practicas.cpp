/*
 * config_practicas.cpp
 *
 *  Created on: 10/03/2015
 *      Author: jctejero
 * 
 *  Modified on: 13/05/2025
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include	"config_practicas.h"
#include    "debuglog.h"

void config_practica1_apt_1();
void config_practica1_apt_2();
void config_practica1_apt_3();
void config_practica3_apt_2();
void config_practica4_apt_2(void);
void config_practica4_apt_3(void);
void config_practica4_apt_4(void);
void config_practica4_apt_5(void);
void config_practica5_apt_4(void);
void Config_P7_SensorMovimiento(void);
void Config_P9_SRC_Interruptor(void);
void Config_P10_CtrlPersiana(void);

TPractica Practicas[] = {
    //{0x11, "P1_PULSADORES", config_practica1_apt_1},
    //{0x12, "P1_INTERRUPTOR", config_practica1_apt_2},
	//{0x13, "P1_CONMUTADOR", config_practica1_apt_3},
	//{0x32, "P3_CONMUTADOR", config_practica3_apt_2},
	//{0x42, "P4_PULSADORES", config_practica4_apt_2},
	//{0x43, "P4_INTERRUPTOR", config_practica4_apt_3},
	//{0x44, "P4_CONMUTADOR_INT", config_practica4_apt_4},
	//{0x45, "P4_CONMUTADOR_2SAL", config_practica4_apt_5},
	{0x50, "PRACTICAS_MODBUS", NULL}, // Placeholder for Modbus practices
	{0x54, "P5_INTERRUPTOR", config_practica5_apt_4},
	{0x62, "P6_INTERRUPTOR", config_practica5_apt_4},
	{0x70, "P7_PIR", Config_P7_SensorMovimiento},
	{0x90, "P9_ALL ANALOG SENSORS", Config_P9_SRC_Interruptor},
	{0xA1, "P10_CONTROL_PERSIANA", Config_P10_CtrlPersiana}
};

/****************************************************************************/
/***                 Functions                                            ***/
/****************************************************************************/
/*
void config_practica1_apt_1(){
	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = Pulsado_Soltado;

	domoboard.BOTON2.SensorEvent = Pulsado_Soltado;

	domoboard.BTN_OPT.SensorEvent = Pulsado_Soltado;
}

void config_practica1_apt_2(){
	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = Interruptor;
	domoboard.BOTON1.Aux = OFF;

	domoboard.BOTON2.SensorEvent = Interruptor;
	domoboard.BOTON2.Aux = OFF;

	domoboard.BTN_OPT.SensorEvent = Interruptor;
	domoboard.BTN_OPT.Aux = OFF;
}

void config_practica1_apt_3(){
	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = conmutador;

	domoboard.BOTON2.SensorEvent = conmutador;

	domoboard.BTN_OPT.SensorEvent = conmutador;
}

void config_practica3_apt_2(){
	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = conmutador_sal;
	domoboard.BOTON1.managerActuators.push(&(domoboard.RELE));

	domoboard.BOTON2.SensorEvent = conmutador_sal;
	domoboard.BOTON2.managerActuators.push(&(domoboard.RELE));

	domoboard.BTN_OPT.SensorEvent = conmutador_sal;
	domoboard.BTN_OPT.managerActuators.push(&(domoboard.RELE));
}

void config_practica4_apt_2(void){

	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = Pulsado_Soltado_sal;
	domoboard.BOTON1.managerActuators.push(&(domoboard.RELE));

	domoboard.BOTON2.SensorEvent = Pulsado_Soltado_sal;
	domoboard.BOTON2.managerActuators.push(&(domoboard.TRIAC));

	domoboard.BTN_OPT.SensorEvent = Pulsado_Soltado_sal;
}

void config_practica4_apt_3(void){

	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.managerActuators.clear();
	domoboard.BOTON1.SensorEvent = interruptor_sal;
	domoboard.BOTON1.managerActuators.push(&(domoboard.RELE));


	domoboard.BOTON2.SensorEvent = interruptor_sal;
	domoboard.BOTON2.managerActuators.push(&(domoboard.TRIAC));

	domoboard.BTN_OPT.SensorEvent = interruptor_sal;
}

void config_practica4_apt_4(void){

	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = conmutador_sal;
	domoboard.BOTON1.managerActuators.push(&(domoboard.TRIAC));

	domoboard.BOTON2.SensorEvent = conmutador_sal;

	domoboard.BOTON2.managerActuators.push(&(domoboard.TRIAC));

	domoboard.BTN_OPT.SensorEvent = interruptor_sal;
	domoboard.BTN_OPT.managerActuators.push(&(domoboard.RELE));
}

void config_practica4_apt_5(void){

	domoboard.Clear_SensorsConfiguration();

	domoboard.BOTON1.SensorEvent = conmutador_sal;
	domoboard.BOTON1.managerActuators.push(&(domoboard.RELE));
	domoboard.BOTON1.managerActuators.push(&(domoboard.TRIAC));

	domoboard.BOTON2.SensorEvent = conmutador_sal;
	domoboard.BOTON2.managerActuators.push(&(domoboard.RELE));
	domoboard.BOTON2.managerActuators.push(&(domoboard.TRIAC));

	domoboard.BTN_OPT.SensorEvent = conmutador_sal;
	domoboard.BTN_OPT.managerActuators.push(&(domoboard.RELE));
	domoboard.BTN_OPT.managerActuators.push(&(domoboard.TRIAC));
}
*/

void config_practica5_apt_4(){
	mbDomoboard.Clear_SensorsConfiguration();

	mbDomoboard.BOTON1.mbSensorEvent = mbInterruptor;
	mbDomoboard.BOTON1.mbActuators.push(&mbDomoboard.RELE);

	mbDomoboard.BOTON2.mbSensorEvent = mbInterruptor;
	mbDomoboard.BOTON2.mbActuators.push(&mbDomoboard.TRIAC);

}

void Config_P7_SensorMovimiento(){

	static AsyncWait asyncWait;

	//Borramos los actuadores previos asignado a cada sensor de entrada
	mbDomoboard.Clear_SensorsConfiguration();

	//Inicialmente, habilitamos el sensor PIR
	mbDomoboard.PIR_MOV.Sensor->Activo = true;		//Habilitamos sensor PIR
	Cregs[MB_ACTPIR] = 0x01;  						//Actualiza registro que monitoriza la habilitación del sensor

	//DEBUGLOGLN("Valor MB_TMP_PIR: %d", Aregs[MB_TMP_PIR]);

	asyncWait.startWaiting(&Aregs[MB_TMP_PIR]);		//Configuramos el tiempo de espera al tiempo almacenado
													//en el registro ModBus usado para tal fin
	mbDomoboard.PIR_MOV.asyncWait = &asyncWait;		//Asignamos el elemento de temporización al sensor
	mbDomoboard.PIR_MOV.mbSensorEvent = mbInterruptorTemporizado;
	mbDomoboard.PIR_MOV.mbActuators.push(&(mbDomoboard.TRIAC));

	mbDomoboard.BOTON1.mbSensorEvent = mbConmutador;
	mbDomoboard.BOTON1.mbActuators.push(&(mbDomoboard.RELE));

	mbDomoboard.BOTON2.mbSensorEvent = mbConmutador;
	mbDomoboard.BOTON2.mbActuators.push(&(mbDomoboard.RELE));

	mbDomoboard.BTN_OPT.mbSensorEvent = mbConmutador;
	mbDomoboard.BTN_OPT.mbActuators.push(&(mbDomoboard.RELE));
}

void Config_P9_SRC_Interruptor(void){

	//Borramos los actuadores previos asignado a cada sensor de entrada
	mbDomoboard.Clear_SensorsConfiguration();

	//Config Photoresistor
	mbDomoboard.PHOTORES.ctrlLevelPtr.lInf = &(Aregs[MB_SRC_LL]);
	mbDomoboard.PHOTORES.ctrlLevelPtr.lSup = &(Aregs[MB_SRC_HL]);
	mbDomoboard.PHOTORES.mbSensorEvent = interruptor_SRC;
	mbDomoboard.PHOTORES.mbActuators.push(&(mbDomoboard.TRIAC));

	
	mbDomoboard.PHOTOTTOR.mbSensorEvent = trigger_Level;
	mbDomoboard.PHOTOTTOR.mbActuators.push(&(mbDomoboard.RELE));

	mbDomoboard.BOTON1.mbSensorEvent = mbConmutador;
	mbDomoboard.BOTON1.mbActuators.push(&(mbDomoboard.RELE));

	mbDomoboard.BOTON2.mbSensorEvent = mbConmutador;
	mbDomoboard.BOTON2.mbActuators.push(&(mbDomoboard.RELE));

	mbDomoboard.BTN_OPT.mbSensorEvent = mbConmutador;
	mbDomoboard.BTN_OPT.mbActuators.push(&(mbDomoboard.RELE));
}

void Config_P10_CtrlPersiana() {
	// Borramos cualquier configuración previa de sensores y actuadores del sistema
	// para evitar que queden asociaciones antiguas de otras prácticas
	mbDomoboard.Clear_SensorsConfiguration();

	// Configuración inicial del controlador de la persiana
	ctrlPosPer.activa = false;  // Al iniciar, la persiana no está en movimiento
	ctrlPosPer.maxTime = (uint16_t *)&persianaTiempoSubida;  // Tiempo máximo que tarda en subir completamente
	ctrlPosPer.actTime = 0;  // Se resetea el tiempo actual (posición inicial 0%)

	// Asociamos el evento Persiana al BOTON1 (sensor de subida)
	mbDomoboard.BOTON1.mbSensorEvent = Persiana;
	// Cuando se active BOTON1, controlará el actuador PERUP (subida de persiana)
	mbDomoboard.BOTON1.mbActuators.push(&(mbDomoboard.PERUP));

	// Asociamos el evento Persiana al BOTON2 (sensor de bajada)
	mbDomoboard.BOTON2.mbSensorEvent = Persiana;
	// Cuando se active BOTON2, controlará el actuador PERDOWN (bajada de persiana)
	mbDomoboard.BOTON2.mbActuators.push(&(mbDomoboard.PERDOWN));
}

void Config_P11_Garaje(){
	// Borramos cualquier configuración previa de sensores y actuadores del sistema
	// para evitar que queden asociaciones antiguas de otras prácticas
	mbDomoboard.Clear_SensorsConfiguration();

	
	

	// Configuración inicial del controlador de la persiana
	ctrlPosPer.activa = false;  // Al iniciar, la persiana no está en movimiento
	ctrlPosPer.maxTime = (uint16_t *)&persianaTiempoSubida;  // Tiempo máximo que tarda en subir completamente
	ctrlPosPer.actTime = 0;  // Se resetea el tiempo actual (posición inicial 0%)

	// Asociamos el evento Persiana al BOTON1 (sensor de subida)
	mbDomoboard.BOTON1.mbSensorEvent = Garaje;

	// Cuando se active BOTON1, controlará el actuador PERUP (subida de persiana)
	mbDomoboard.BOTON1.mbActuators.push(&(mbDomoboard.PERUP));

	//static uint16_t umbralMinimoPHOTOTTOR = 200;

	// Configuración del sensor PHOTOTTOR
	//mbDomoboard.PHOTOTTOR.ctrlLevelPtr.lInf = &umbralMinimoPHOTOTTOR; // ← Umbral hardcoded
	//mbDomoboard.PHOTOTTOR.Aux = 0;
	//mbDomoboard.PHOTOTTOR.mbSensorEvent = trigger_Level_garaje;

}

void SelectionConfiguration(uint8_t selConf){
	static uint8_t actConf = 0x00;
	bool PSelect = false;

	if(selConf == actConf)
		return;

	actConf = selConf;

	for(auto &practica : Practicas) {
		if (practica.mbCmd == selConf) {
			DEBUGLOGLN("Práctica Seleccionada: %s", practica.NamePractica.c_str());
			if (practica.config_practica != NULL) {
				practica.config_practica();
			}
			PSelect = true;
			break;
		}
	}

	if(!PSelect) {
		DEBUGLOGLN("Práctica no válida: %02X", selConf);
	}
}


