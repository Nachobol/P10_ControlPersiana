/* 		
 Modbus over serial line - RTU Slave Arduino Sketch 
 
 By Juan Pablo Zometa : jpmzometa@gmail.com
 http://sites.google.com/site/jpmzometa/
 Samuel Marco: sammarcoarmengol@gmail.com
 and Andras Tucsni.
 
 These functions implement functions 3, 6, and 16 (read holding registers,
 preset single register and preset multiple registers) of the 
 Modbus RTU Protocol, to be used over the Arduino serial connection.
 
 This implementation DOES NOT fully comply with the Modbus specifications.
 
 This Arduino adaptation is derived from the work
 By P.Costigan email: phil@pcscada.com.au http://pcscada.com.au
 
 These library of functions are designed to enable a program send and
 receive data from a device that communicates using the Modbus protocol.
 
 Copyright (C) 2000 Philip Costigan  P.C. SCADA LINK PTY. LTD.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
 The functions included here have been derived from the 
 Modicon Modbus Protocol Reference Guide
 which can be obtained from Schneider at www.schneiderautomation.com.
 
 This code has its origins with 
 paul@pmcrae.freeserve.co.uk (http://www.pmcrae.freeserve.co.uk)
 who wrote a small program to read 100 registers from a modbus slave.
 
 I have used his code as a catalist to produce this more functional set
 of functions. Thanks paul.
 */

//#include "WProgram.h"
#include	"Arduino.h"
#include 	"ModbusSlave.h"

/****************************************************************************
 * BEGIN MODBUS RTU SLAVE FUNCTIONS
 ****************************************************************************/

/****************************************************************************/
/***         Definitions                                                  ***/
/****************************************************************************/
/* constants */
enum { 
	MAX_READ_REGD 		= 0x08,
	MAX_READ_REGS 		= 0x19,
	MAX_WRITE_REGS 		= 0x19,
	MAX_MESSAGE_LENGTH	= 64
};


enum { 
	RESPONSE_SIZE = 6,
	EXCEPTION_SIZE = 3,
	CHECKSUM_SIZE = 2
};

/* exceptions code */
enum { 
	NO_REPLY = -1,
	EXC_FUNC_CODE = 1,
	EXC_ADDR_RANGE = 2,
	EXC_REGS_QUANT = 3,
	EXC_EXECUTE = 4
};

/* positions inside the query/response array */
enum { 
	SLAVE = 0,
	FUNC,
	START_H,
	START_L,
	REGS_H,
	REGS_L,
	BYTE_CNT
};


/* enum of supported modbus function codes. If you implement a new one, put its function code here ! */
enum { 
	FC_READ_COILS   	=	0x01,	//Lectura de señales discretas de salida
	FC_READ_REG_DI		=	0x02,	//Read contiguous block of discrete inputs
	FC_READ_REG_AO		= 	0x03,   //Read contiguous block of holding register
	FC_READ_INPUT_REGS	=   0x04,	//Read contiguous block of input registers
	FC_WRITE_COIL   	=   0x05,	//Escritura de una señal discreta
	FC_WRITE_REG  		= 	0x06,   //Write single holding register
	FC_WRITE_REGS 		= 	0x10    //Write block of contiguous registers
};

/****************************************************************************/
/***        Variables Locales                                             ***/
/****************************************************************************/
/* supported functions. If you implement a new one, put its function code into this array! */
const unsigned char fsupported[] = { FC_READ_COILS, FC_READ_REG_DI, FC_READ_REG_AO, FC_READ_INPUT_REGS, 
									FC_WRITE_COIL, FC_WRITE_REG, FC_WRITE_REGS };
HardwareSerial	*Serial485;

ModbusSlave::ModbusSlave()
{
	txenpin = 0;
	slave = 0;
	T35 = 0;
};

/*
 * set_BankCoils
 *
 *   Configure the register used to Digital Outputs (Coils)
 */
void ModbusSlave::set_BankCoils(uint16_t *Regs, uint16_t lengthRegs){
	CRegs = Regs;
	N_CRegs = lengthRegs;
}

/*
 * set_BankDiscreteInputs
 *
 *   Configure the register used to Discrete Inputs signals
 */
void ModbusSlave::set_BankDiscreteInputs(uint16_t *Regs, uint16_t lengthRegs){
	DRegs = Regs;
	N_DRegs = lengthRegs;
}

/*
 * set_BankDiscreteInputs
 *
 *   Configure the Output Holding Bank Register
 */
void ModbusSlave::set_BankOutputHoldingRegister(uint16_t *Regs, uint16_t lengthRegs){
	ARegs = Regs;
	N_ARegs = lengthRegs;
}

/*
 * set_BankInputRegister
 *
 *   Configure the Analog Input Register
 */
void ModbusSlave::set_BankInputRegister(uint16_t *Regs, uint16_t lengthRegs) {
	IRegs = Regs;
	N_IRegs = lengthRegs;
}

/*
CRC
 
 INPUTS:
 	buf   ->  Array containing message to be sent to controller.            
 	start ->  Start of loop in crc counter, usually 0.
 	cnt   ->  Amount of bytes in message being sent to controller/
 OUTPUTS:
 	temp  ->  Returns crc byte for message.
 COMMENTS:
 	This routine calculates the crc high and low byte of a message.
 	Note that this crc is only used for Modbus, not Modbus+ etc. 
 ****************************************************************************/

unsigned int ModbusSlave::crc(unsigned char *buf, unsigned char start, unsigned char cnt)
{
	unsigned char i, j;
	unsigned temp, temp2, flag;

	temp = 0xFFFF;

	for (i = start; i < cnt; i++) {
		temp = temp ^ buf[i];

		for (j = 1; j <= 8; j++) {
			flag = temp & 0x0001;
			temp = temp >> 1;
			if (flag)
				temp = temp ^ 0xA001;
		}
	}

	/* Reverse byte order. */
	temp2 = temp >> 8;
	temp = (temp << 8) | temp2;
	temp &= 0xFFFF;

	return (temp);
}

/***********************************************************************
 * 
 * 	The following functions construct the required query into
 * 	a modbus query packet.
 * 
 ***********************************************************************/

/* 
 * Start of the packet of a read_holding_register response 
 */
void ModbusSlave::build_read_packet(unsigned char function,
unsigned char count, unsigned char *packet) 
{
	packet[SLAVE] = slave;
	packet[FUNC] = function;
	packet[2] = count * 2;
} 

/* 
 * Start of the packet of a preset_multiple_register response 
 */
void ModbusSlave::build_write_packet(unsigned char function,
unsigned int start_addr, 
unsigned char count,
unsigned char *packet) 
{
	packet[SLAVE] = slave;
	packet[FUNC] = function;
	packet[START_H] = start_addr >> 8;
	packet[START_L] = start_addr & 0x00ff;
	packet[REGS_H] = 0x00;
	packet[REGS_L] = count;
} 

/* 
 * Start of the packet of a write_single_register response 
 */
void ModbusSlave::build_write_single_packet(unsigned char function,
        unsigned int write_addr, unsigned int reg_val, unsigned char* packet) 
{
	packet[SLAVE] = slave;
	packet[FUNC] = function;
	packet[START_H] = write_addr >> 8;
	packet[START_L] = write_addr & 0x00ff;
	packet[REGS_H] = reg_val >> 8;
	packet[REGS_L] = reg_val & 0x00ff;
} 

/* 
 * Start of the packet of an exception response 
 */
void ModbusSlave::build_error_packet( unsigned char function,
unsigned char exception, unsigned char *packet) 
{
	packet[SLAVE] = slave;
	packet[FUNC] = function + 0x80;
	packet[2] = exception;
} 


/*************************************************************************
 * 
 * modbus_query( packet, length)
 * 
 * Function to add a checksum to the end of a packet.
 * Please note that the packet array must be at least 2 fields longer than
 * string_length.
 **************************************************************************/

void ModbusSlave::modbus_reply(unsigned char *packet, unsigned char string_length) 
{
	int temp_crc;

	temp_crc = crc(packet, 0, string_length);
	packet[string_length] = temp_crc >> 8;
	string_length++;
	packet[string_length] = temp_crc & 0x00FF;
} 

/***********************************************************************
 * 
 * send_reply( query_string, query_length )
 * 
 * Function to send a reply to a modbus master.
 * Returns: total number of characters sent
 ************************************************************************/

int ModbusSlave::send_reply(unsigned char *query, unsigned char string_length) 
{
	unsigned char i = 1;

	if (txenpin > 1) { // set MAX485 to speak mode
		UCSR0A=UCSR0A |(1 << TXC0);
		digitalWrite( txenpin, HIGH);
		delay(1);
	}

	modbus_reply(query, string_length);
	string_length = string_length + 2;

	Serial485->write(query,string_length);

	delay(10);
	if (txenpin > 1) {// set MAX485 to listen mode
		while (!(UCSR0A & (1 << TXC0)));
		digitalWrite( txenpin, LOW);
	}

	return string_length;
	return i; 		/* it does not mean that the write was succesful, though */
}

/***********************************************************************
 * 
 * 	receive_request( array_for_data )
 * 
 * Function to monitor for a request from the modbus master.
 * 
 * Returns:	Total number of characters received if OK
 * 0 if there is no request 
 * A negative error code on failure
 ***********************************************************************/

int ModbusSlave::receive_request(unsigned char *received_string) 
{
	int bytes_received = 0;

	/* FIXME: does Serial.available wait 1.5T or 3.5T before exiting the loop? */
	while (Serial485->available()) {
		received_string[bytes_received] = Serial485->read();
		bytes_received++;
		if (bytes_received >= MAX_MESSAGE_LENGTH)
			return NO_REPLY; 	/* port error */
	}

	return (bytes_received);
}

/*********************************************************************
 * 
 * 	modbus_request(request_data_array)
 * 
 * Function to the correct request is returned and that the checksum
 * is correct.
 * 
 * Returns:	string_length if OK
 * 		0 if failed
 * 		Less than 0 for exception errors
 * 
 * 	Note: All functions used for sending or receiving data via
 * 	      modbus return these return values.
 * 
 **********************************************************************/
int ModbusSlave::modbus_request(unsigned char *data) 
{
	int response_length;
	unsigned int crc_calc = 0;
	unsigned int crc_received = 0;
	//unsigned char recv_crc_hi;
	//unsigned char recv_crc_lo;

	response_length = receive_request(data);

	if (response_length > 0) {
		crc_calc = crc(data, 0, response_length - 2);
		//recv_crc_hi = (unsigned) data[response_length - 2];
		//recv_crc_lo = (unsigned) data[response_length - 1];
		crc_received = data[response_length - 2];
		crc_received = (unsigned) crc_received << 8;
		crc_received = crc_received | (unsigned) data[response_length - 1];

		/*********** check CRC of response ************/
		if (crc_calc != crc_received) {
			return NO_REPLY;
		}

		/* check for slave id */
		if (slave != data[SLAVE]) {
			return NO_REPLY;
		}
	}
	return (response_length);
}

/************************************************************************
 *
 * 	write_regs(first_register, data_array, registers_array)
 *
 * 	writes into the slave's holding registers the data in query,
 * starting at start_addr.
 *
 * Returns:   the number of registers written
 ************************************************************************/
int ModbusSlave::write_regs(unsigned int start_addr, unsigned char *query, uint16_t *regs){
	int temp;
	unsigned int i;

	for (i = 0; i < query[REGS_L]; i++) {
		/* shift reg hi_byte to temp */
		temp = (int) query[(BYTE_CNT + 1) + i * 2] << 8;
		/* OR with lo_byte           */
		temp = temp | (int) query[(BYTE_CNT + 2) + i * 2];

		regs[start_addr + i] = temp;
	}
	return i;
}

/*********************************************************************
 * 
 * 	validate_request(request_data_array, request_length, available_regs)
 * 
 * Function to check that the request can be processed by the slave.
 * 
 * Returns:	0 if OK
 * 		A negative exception code on error
 * 
 **********************************************************************/

int ModbusSlave::validate_request(unsigned char *data, unsigned char length)
{
	(void)length;

	unsigned int i, fcnt = 0;
	unsigned int regs_num = 0;
	unsigned int start_addr = 0;
	unsigned char max_regs_num = 0;
	//char msg[50];

	unsigned int regs_size = 0;

	/* check function code */
	for (i = 0; i < sizeof(fsupported); i++) {
		if (fsupported[i] == data[FUNC]) {
			fcnt = 1;
			break;
		}
	}

	if (0 == fcnt)
		return EXC_FUNC_CODE;
                
	if (FC_WRITE_REG == data[FUNC]) {
		/* For function write single reg, this is the target reg.*/
		return 0;
	}

	if (FC_WRITE_COIL == data[FUNC]) {
		/* For function write single reg, this is the target reg.*/
		regs_num = ((int) data[START_H] << 8) + (int) data[START_L];
		if (regs_num >= N_CRegs){
			return EXC_ADDR_RANGE;
		}
		return 0;
	}

	/* For functions read/write regs, this is the range. */
	regs_num = ((int) data[REGS_H] << 8) + (int) data[REGS_L];

	/* check quantity of registers */
	if (FC_READ_REG_DI == data[FUNC]){
		regs_size = N_DRegs;
		max_regs_num = MAX_READ_REGD;
	}

	/* check quantity of registers */
	if (FC_READ_COILS == data[FUNC]){
		regs_size = N_CRegs;
		max_regs_num = MAX_READ_REGD;
	}

	if (FC_READ_REG_AO == data[FUNC]){
		regs_size = N_ARegs;
		max_regs_num = MAX_READ_REGS;
	}
	else if (FC_READ_INPUT_REGS == data[FUNC]){
		regs_size = N_IRegs;
		max_regs_num = MAX_READ_REGS;
	}
	else if (FC_WRITE_REGS == data[FUNC]){

	}

	if ((regs_num < 1) || (regs_num > max_regs_num))
		return EXC_REGS_QUANT;

	/* check registers range, start address is 0 */
	start_addr = ((int) data[START_H] << 8) + (int) data[START_L];

	if ((start_addr + regs_num) > regs_size)
		return EXC_ADDR_RANGE;

	return 0; 		/* OK, no exception */
}

/************************************************************************
 * 
 * 	preset_multiple_registers(first_register, number_of_registers,
 * data_array, registers_array)
 * 
 * 	Write the data from an array into the holding registers of the slave. 
 * 
 *************************************************************************/

int ModbusSlave::preset_multiple_registers(unsigned int start_addr,
unsigned char count, 
unsigned char *query,
uint16_t *regs)
{
        unsigned char function = FC_WRITE_REGS;	/* Preset Multiple Registers */
        int status = 0;
        unsigned char packet[RESPONSE_SIZE + CHECKSUM_SIZE];

        build_write_packet(function, start_addr, count, packet);

        if (write_regs(start_addr, query, regs)) {
                status = send_reply(packet, RESPONSE_SIZE);
        }

        return (status);
}

/************************************************************************
 * 
 * write_single_register(slave_id, write_addr, data_array, registers_array)
 * 
 * Write a single int val into a single holding register of the slave. 
 * 
 *************************************************************************/

int ModbusSlave::write_single_register(unsigned int write_addr, unsigned char *query, uint16_t *regs)
{
        unsigned char function = FC_WRITE_REG; /* Function: Write Single Register */
        int status = 0;
        unsigned int reg_val;
        unsigned char packet[RESPONSE_SIZE + CHECKSUM_SIZE];

        reg_val = query[REGS_H] << 8 | query[REGS_L];
        build_write_single_packet(function, write_addr, reg_val, packet);
        regs[write_addr] = (int) reg_val;

        status = send_reply(packet, RESPONSE_SIZE);

        return (status);
}

int ModbusSlave::write_coil(unsigned int write_addr, unsigned char *query, uint16_t *regs){
	unsigned char function = FC_WRITE_COIL; /* Function: Write Single Register */
	int status = 0;
	unsigned int reg_val;
	unsigned char packet[RESPONSE_SIZE + CHECKSUM_SIZE];

	reg_val = query[REGS_H] << 8 | query[REGS_L];
	build_write_single_packet(function, write_addr, reg_val, packet);
	regs[write_addr] = (int) reg_val;

	status = send_reply(packet, RESPONSE_SIZE);

	return (status);
}


/************************************************************************
 * 
 * 	read_holding_registers(first_register, number_of_registers,
 * registers_array)
 * 
 * reads the slave's holdings registers and sends them to the Modbus master
 * 
 *************************************************************************/

int ModbusSlave::read_discrete_registers(unsigned int Func, unsigned int start_addr, unsigned char reg_count, uint16_t *regs)
{
	int packet_size = 3;
	byte	regnum = 0;
	int status;
	unsigned int i;
	unsigned char packet[MAX_MESSAGE_LENGTH];

	//build_read_packet(function, reg_count, packet);
	packet[SLAVE]	= slave;
	packet[FUNC] 	= Func; 		//FC_READ_REG_DI;
	packet[2] 		= 1;             //Sólo enviaremos un máximo de 8 registros;

	packet[packet_size] = 0;

	for (i = start_addr; i < (start_addr + (unsigned int) reg_count);i++) {
		packet[packet_size] |= (regs[i] != 0x0000) << regnum;
		regnum++;
	}

	packet_size++;


	status = send_reply(packet, packet_size);

	return (status);
}

/************************************************************************
 *
 * 	read_Analog_registers(first_register, number_of_registers, registers_array)
 *
 * reads the slave's analog registers and sends them to the Modbus master
 *
 *************************************************************************/
int ModbusSlave::read_Analog_registers(unsigned char Func, unsigned int start_addr,unsigned char reg_count, uint16_t *regs)
{
	//unsigned char Func = FC_READ_REG_AO; 	/* Read Analog Input Registers */
	int packet_size = 3;
	int status;
	unsigned int i;
	unsigned char packet[MAX_MESSAGE_LENGTH];

	build_read_packet(Func, reg_count, packet);

	for (i = start_addr; i < (start_addr + (unsigned int) reg_count);i++) {
		packet[packet_size] = regs[i] >> 8;
		packet_size++;
		packet[packet_size] = regs[i] & 0x00FF;
		packet_size++;
	}

	status = send_reply(packet, packet_size);

	return (status);
}

/* 
 * configure(slave, baud, parity, txenpin)
 *
 * sets the communication parameters for of the serial line.
 *
 * slave: identification number of the slave in the Modbus network (1 to 127)
 * baud: baudrate in bps (typical values 9600, 19200... 115200)
 * parity: a single character sets the parity mode (character frame format): 
 *         'n' no parity (8N1); 'e' even parity (8E1), 'o' for odd parity (8O1).
 * txenpin: arduino pin number that controls transmision/reception
 *        of an external half-duplex device (e.g. a RS485 interface chip).
 *        0 or 1 disables this function (for a two-device network)
 *        >2 for point-to-multipoint topology (e.g. several arduinos)
 */

void ModbusSlave::configure(unsigned char slave, long baud, char parity, char txenpin)
{
	this->slave = slave;
	this->txenpin = txenpin;
	
	Serial485->begin(baud);

	switch (parity) {
	case 'e': // 8E1
		UCSR0C |= ((1<<UPM01) | (1<<UCSZ01) | (1<<UCSZ00));
		//      UCSR0C &= ~((1<<UPM00) | (1<<UCSZ02) | (1<<USBS0));
		break;
	case 'o': // 8O1
		UCSR0C |= ((1<<UPM01) | (1<<UPM00) | (1<<UCSZ01) | (1<<UCSZ00));
		//      UCSR0C &= ~((1<<UCSZ02) | (1<<USBS0));
		break;
	case 'n': // 8N1
		UCSR0C |= ((1<<UCSZ01) | (1<<UCSZ00));
		//      UCSR0C &= ~((1<<UPM01) | (1<<UPM00) | (1<<UCSZ02) | (1<<USBS0));
		break;
	default:
		break;
	}

	if (txenpin > 1) { // pin 0 & pin 1 are reserved for RX/TX
		pinMode(txenpin, OUTPUT);
		digitalWrite(txenpin, LOW);
	}

	T35 = Calcular_T35(baud);

	//T35 = 3500;
	return;
}   

unsigned long ModbusSlave::Calcular_T35(long baud){
	//double c = 1.0e+6;

	double c = (35000000)/baud; //3.5 caracteres en uSeg

	return c;
}


/*
 * update(regs, regs_size)
 * 
 * checks if there is any valid request from the modbus master. If there is,
 * performs the requested action
 * 
 * regs: an array with the holding registers. They start at address 1 (master point of view)
 * regs_size: total number of holding registers, i.e. the size of the array regs.
 * returns: 0 if no request from master,
 * 	NO_REPLY (-1) if no reply is sent to the master
 * 	an exception code (1 to 4) in case of a modbus exceptions
 * 	the number of bytes sent as reply ( > 4) if OK.
 */
unsigned long Nowdt = 0;
int lastBytesReceived;

int ModbusSlave::update()
{
	unsigned char query[MAX_MESSAGE_LENGTH];
	unsigned char errpacket[EXCEPTION_SIZE + CHECKSUM_SIZE];
	unsigned int start_addr;
	int exception;

	int length = Serial485->available();

	unsigned long now = micros();


	if (length == 0) {
		lastBytesReceived = 0;
		return 0;
	}

	if (lastBytesReceived != length) {
		lastBytesReceived = length;
		Nowdt = now + T35;
		return 0;
	}

	if (now < Nowdt)
		return 0;

	lastBytesReceived = 0;

	length = modbus_request(query);

	if (length < 1)
		return length;

	exception = validate_request(query, length);

	if (exception) {
		build_error_packet( query[FUNC], exception,errpacket);
		send_reply(errpacket, EXCEPTION_SIZE);
		return (exception);
	}

	start_addr = ((int) query[START_H] << 8) + (int) query[START_L];

	switch (query[FUNC]) {
	case FC_READ_COILS:
		return read_discrete_registers(FC_READ_COILS, start_addr,query[REGS_L],CRegs);

	case FC_READ_REG_DI:
		return read_discrete_registers(FC_READ_REG_DI, start_addr,query[REGS_L],DRegs);

	case FC_READ_REG_AO:
		//return read_holding_registers(start_addr,query[REGS_L],ARegs);
		return read_Analog_registers(FC_READ_REG_AO, start_addr,query[REGS_L],ARegs);

	case FC_READ_INPUT_REGS:
		return read_Analog_registers(FC_READ_INPUT_REGS, start_addr, query[REGS_L], IRegs);
		
	case FC_WRITE_REGS:
//		return preset_multiple_registers(start_addr,query[REGS_L],query,A_Regs);
		break;

	case FC_WRITE_REG:
		return write_single_register(start_addr,query,ARegs);

	case FC_WRITE_COIL:
		return write_coil(start_addr,query,CRegs);	
	}

	return 0;
}



