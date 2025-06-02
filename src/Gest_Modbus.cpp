/*
 * Gest_Modbus.cpp
 *
 *  Created on: 16/05/2014
 *      Author: jctejero
 *
 *  Modified on: 01/04/2025
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include	<Arduino.h>
#include	<EEPROM.h>
#include	"Gest_Modbus.h"
#include 	"utils_domoBoard.h"
#include    "config_practicas.h"
#include	"debuglog.h"

/****************************************************************************/
/***        Variables Locales                                             ***/
/****************************************************************************/
/* First step MBS: create an instance */
ModbusSlave mbs;

/****************************************************************************/
/***                 Functions                                            ***/
/****************************************************************************/
void Init_RTU_Modbus()
{
	/* configure modbus communication
	 * 115200 bps, 8E1, two-device network */
	/* Second step MBS: configure */
	/* the Modbus slave configuration parameters */
	const unsigned char 	SLAVE 	= ADDR_SLAVE;		//Address SLAVE
	const long 				BAUD 	= SERIAL_BPS;
	const char 				PARITY 	= SERIAL_PARITY;
	const char 				TXENPIN = 0; //EN_485;

	//Inicialmente configuramos 485 para recibir
	/*
	if(TXENPIN != 0)
		digitalWrite(EN_485, LOW);
	else digitalWrite(EN_485, HIGH);
	*/

	//Para la conexión 485/ModBus usamos
	Serial485 = &Serial;


	//We configure the ModBus Register Banks
	mbs.set_BankCoils(Cregs, MB_O_COILS);
	mbs.set_BankDiscreteInputs(Dregs, MB_I_CONTATCS);
	mbs.set_BankOutputHoldingRegister(Aregs, MB_A_REGS);
	mbs.set_BankInputRegister(Iregs, MB_I_REGS);

	mbs.configure(SLAVE,BAUD,PARITY,TXENPIN);
}

/*
 *
 */

 void writecoil(){

	for (int addrReg = 0; addrReg < MB_O_COILS; addrReg++) {		
		switch (addrReg) {
			case MB_RELE:
				mbDomoboard.setActuator(mbDomoboard.RELE.actuator, Cregs[MB_RELE] != 0x00);
				break;

			case MB_TRIAC:
				mbDomoboard.setActuator(mbDomoboard.TRIAC.actuator, Cregs[MB_TRIAC] != 0x00);
				break;	

			case   MB_ACTPIR:
				mbDomoboard.PIR_MOV.Sensor->Activo = Cregs[addrReg]!=0x00;

				//Save PIR Activation
				EEPROM.update(ADDR_ACTPIR, Cregs[addrReg] != 0x00);
				break;

			case MB_ACTSRC:
				mbDomoboard.PHOTORES.Sensor->Activo = Cregs[addrReg] != 0x00;

				//Save SRC Activation
				EEPROM.update(ADDR_ACTSRC, Cregs[addrReg] != 0x00);
				break;

			case MB_ACTTOR:
				mbDomoboard.PHOTOTTOR.Sensor->Activo = Cregs[addrReg] != 0x00;

				//Save foto_ttor Activation
				EEPROM.update(ADDR_ACTTOR, Cregs[addrReg] != 0x00);
				break;

			case MB_PERUP:
			case MB_PERDOWN:
			// Si se ha escrito en alguno de los registros MB_PERUP o MB_PERDOWN
			// (botón virtual de subir o bajar la persiana desde Modbus),
			// se verifica si la práctica seleccionada actualmente es la P10 (práctica de persiana).
			if ((uint8_t)(Aregs[P11_GARAGE] & 0xFF) == P11_GARAGE)
				UpDown_Garaje();  // Ejecuta la lógica de subida/bajada/parada según el nuevo estado de los botones
			break;
					}
	}

}

/*
 *
 */

 void writeholdingregister(){
	//char msg[50];

	for (int addrReg = 0; addrReg < MB_A_REGS; addrReg++) {
		switch (addrReg) {
			case MB_SELPRACT:
				//Save pratice selected
				EEPROM.update(ADDR_SELPRACT, Aregs[ADDR_SELPRACT]&0xFF);
				
				//Go To Selecction Configuration
				SelectionConfiguration((uint8_t) (Aregs[MB_SELPRACT] & 0xFF));
				break;

			case MB_TMP_PIR:
				//Almacenar Tiempo de activación Sensor PIR (Big Endian)
				EEPROM.update(ADDR_TIEMPO_PIR_1, (Aregs[MB_TMP_PIR]>>8)&0xFF);
				EEPROM.update(ADDR_TIEMPO_PIR_2, (Aregs[MB_TMP_PIR]&0xFF));
				break;
			
			case MB_TMP1_GARAJE:
   				 // Almacenar el tiempo de activación del sensor PIR (en milisegundos)
				// en la EEPROM en formato Big Endian para conservarlo tras apagados/reinicios.

				// Guarda el byte más significativo (bits 15-8) del registro Aregs[MB_TMP1_GARAGE]
				// en la dirección ADDR_TIEMPO_PIR_1 de la EEPROM.
				EEPROM.update(ADDR_TMP1_GARAJE, (Aregs[MB_TMP1_GARAJE] >> 8) & 0xFF);

				// Guarda el byte menos significativo (bits 7-0) del registro Aregs[MB_TMP1_GARAGE]
				// en la dirección ADDR_TIEMPO_PIR_2 de la EEPROM.
				EEPROM.update(ADDR_TMP12_GARAJE, (Aregs[MB_TMP1_GARAJE] & 0xFF));

				break; 
			
			case MB_TMP5_GARAJE:
				// Almacenar el tiempo de activación del sensor PIR (en milisegundos)
				// en la EEPROM en formato Big Endian para conservarlo tras apagados/reinicios.

				// Guarda el byte más significativo (bits 15-8) del registro Aregs[MB_TMP5_GARAGE]
				// en la dirección ADDR_TIEMPO_PIR_1 de la EEPROM.
				EEPROM.update(ADDR_TMP5_GARAJE, (Aregs[MB_TMP5_GARAJE] >> 8) & 0xFF);

				// Guarda el byte menos significativo (bits 7-0) del registro Aregs[MB_TMP5_GARAGE]
				// en la dirección ADDR_TIEMPO_PIR_2 de la EEPROM.
				EEPROM.update(ADDR_TMP52_GARAJE, (Aregs[MB_TMP5_GARAJE] & 0xFF));

				break; 

			
			case MB_SRC_HL:
				//Almacenar nivel superior de activación SRC (Big Endian)
				EEPROM.update(ADDR_SRC_HL_1, (Aregs[MB_SRC_HL] >> 8) & 0xFF);
				EEPROM.update(ADDR_SRC_HL_2, (Aregs[MB_SRC_HL] & 0xFF));
				break;

			case MB_SRC_LL:
				//Almacenar nivel inferior de activación SRC (Big Endian)
				EEPROM.update(ADDR_SRC_LL_1, (Aregs[MB_SRC_LL] >> 8) & 0xFF);
				EEPROM.update(ADDR_SRC_LL_2, (Aregs[MB_SRC_LL] & 0xFF));
				break;

			case MB_TTOR_LL:
				//Almacenar nivel inferior de activación SRC (Big Endian)
				EEPROM.update(ADDR_TTOR_LL_1, (Aregs[MB_TTOR_LL] >> 8) & 0xFF);
				EEPROM.update(ADDR_TTOR_LL_2, (Aregs[MB_TTOR_LL] & 0xFF));
				break;

			case MB_STAPER:
			// Si se detecta una escritura en el registro MB_STAPER (registro de estado de la persiana),
			// se llama a la función que actualiza la lógica de control de la persiana.
			// Esto se ejecuta, por ejemplo, cuando desde el SCADA o interfaz Modbus
			// se cambia el estado de la persiana manualmente (subir, bajar, parar).
				update_garajeState();
				

				break;
		}
	}
}


void RTU_ModBus()
{
	if(mbs.update()){
		writecoil();

		writeholdingregister();
	}
}

/*
 * void load_config().- load initial configuration for arduino aplication
 */
void load_Config(){
	//Leemos configuración Actual "Selección Práctica"
	Aregs[MB_SELPRACT] = EEPROM.read(ADDR_SELPRACT);	//Read EEPROM

	Aregs[MB_SELPRACT] = (Aregs[MB_SELPRACT] == 0xFF) ? 0xA2 : Aregs[MB_SELPRACT];	//Seleccionamos la práctica 1 apartado 1 por defecto

	SelectionConfiguration((uint8_t) (Aregs[MB_SELPRACT] & 0xFF));

	/*
	 * Config PIR
	 */
	//Leer activación del PIR
	Cregs[MB_ACTPIR] = EEPROM.read(ADDR_ACTPIR);

	//DEBUGLOGLN("PIR activado: %d", Cregs[MB_ACTPIR]);

	//Leer Tiempo activación PIR
	Aregs[MB_TMP_PIR] = ((EEPROM.read(ADDR_TIEMPO_PIR_1)&0xFF) << 8) + (EEPROM.read(ADDR_TIEMPO_PIR_2)&0xFF) ;
	Aregs[MB_TMP1_GARAJE] = ((EEPROM.read(ADDR_TMP1_GARAJE)&0xFF) << 8) + (EEPROM.read(ADDR_TMP12_GARAJE)&0xFF);
	Aregs[MB_TMP5_GARAJE] = ((EEPROM.read(ADDR_TMP5_GARAJE)&0xFF) << 8) + (EEPROM.read(ADDR_TMP52_GARAJE)&0xFF);

	//DEBUGLOGLN("Tiempo Activación del PIR: %d Segundos", Aregs[MB_TMP_PIR]);

	/*
	 * Configuration SRC
	 */
	Cregs[MB_ACTSRC] = EEPROM.read(ADDR_ACTSRC);

//	DEBUGLOGLN("PhotoResistor activado: %d", Cregs[MB_ACTSRC]);


	Aregs[MB_SRC_HL] = (EEPROM.read(ADDR_SRC_HL_1) << 8)
			+ (EEPROM.read(ADDR_SRC_HL_2) & 0xFF);
	Aregs[MB_SRC_LL] = (EEPROM.read(ADDR_SRC_LL_1) << 8)
			+ (EEPROM.read(ADDR_SRC_LL_2) & 0xFF);

//	DEBUGLOGLN("Nivel Activación SRC: Bajo --> %d | Alto --> %d", Aregs[MB_SRC_LL], Aregs[MB_SRC_HL]);

	/*
	 * Configuration Phototransistor
	 */

	Cregs[MB_ACTTOR] = EEPROM.read(ADDR_ACTTOR);

//	DEBUGLOGLN("PhotoTransistor activado: %d", Cregs[MB_ACTTOR]);


	Aregs[MB_TTOR_LL] = (EEPROM.read(ADDR_TTOR_LL_1) << 8)
			+ (EEPROM.read(ADDR_TTOR_LL_2) & 0xFF);

//	DEBUGLOGLN("Nivel Activación PhotoTransistor: %d", Aregs[MB_TTOR_LL]);

	writecoil();
	writeholdingregister();
}

