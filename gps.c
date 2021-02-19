/**
 ********************************************************************************
 * @file    gps.c
 * @author  Embedded Area
 * @date    Jan 9, 2021
 * @brief   Source file of the NEO_6M GPS Module
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "gps.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/


#if GPRMC_FRAME_PARSE_ON
/**
 * @brief Lookup table for the types of GPRMC address frame in an order.
 */
static const gps_infoType_t typeTableGPRMC[GPRMC_FRAME_BUFFER_SIZE] = {
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_FLOAT_TYPE,
		GPS_FLOAT_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_CHARACTER_TYPE
};
#endif


#if GPVTG_FRAME_PARSE_ON
/**
 * @brief Lookup table for the types of GPVTG address frame in an order.
 */
static const gps_infoType_t typeTableGPVTG[GPVTG_FRAME_BUFFER_SIZE] = {
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_CHARACTER_TYPE
};
#endif


#if GPGGA_FRAME_PARSE_ON
/**
 * @brief Lookup table for the types of GPGGA address frame in an order.
 */
static const gps_infoType_t typeTableGPGGA[GPGGA_FRAME_BUFFER_SIZE] = {
		GPS_FLOAT_TYPE,
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE
};
#endif


#if GPGSA_FRAME_PARSE_ON
/**
 * @brief Lookup table for the types of GPGSA address frame in an order.
 */
static const gps_infoType_t typeTableGPGSA[GPGSA_FRAME_BUFFER_SIZE] = {
		GPS_CHARACTER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE
};
#endif


#if GPGSV_FRAME_PARSE_ON
/**
 * @brief Lookup table for the types of GPGSV address frame in an order.
 */
static const gps_infoType_t typeTableGPGSV[GPGSV_FRAME_BUFFER_SIZE] = {
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE,
		GPS_INTEGER_TYPE
};
#endif


#if GPGLL_FRAME_PARSE_ON
/**
 * @brief Lookup table for the types of GPGLL address frame in an order.
 */
static const gps_infoType_t typeTableGPGLL[GPGLL_FRAME_BUFFER_SIZE] = {
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_FLOAT_TYPE,
		GPS_CHARACTER_TYPE,
		GPS_CHARACTER_TYPE
};
#endif

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/

/************************************
 * GLOBAL FUNCTIONS
 ************************************/

/**
 * @brief Finds the index of the value in given array
 * @param str Address of input array
 * @param size Size of the input array, also it can be used as ending index
 * @param startIndex Starting index of the search
 * @param value Desired value to find
 * @return Index of the searched value
 */
size_t gps_findIndex(uint8_t * str, size_t size, size_t startIndex, uint8_t value){

	for(size_t i = startIndex; i < size; i++){
		if(str[i] == value){
			return i;
		}
	}
	return GPS_ERROR; /* The value is could not find in given array */
}

/**
 * @brief Finds the nth index of the value in given array. Returns 7 for the 3rd 'a' in "abvdasdaf".
 * @param str Address of input array
 * @param size Size of the input array, also it can be used as ending index
 * @param startIndex Starting index of the search
 * @param value Desired value to find
 * @param Xth Order of the desired value
 * @return Index of the searched value
 */
size_t gps_findXthIndex(uint8_t * str, size_t size, size_t startIndex, uint8_t value, uint8_t Xth){

	size_t nextStartIndex = startIndex;
	uint8_t count = 0;
	do{
		nextStartIndex = gps_findIndex(str, size, nextStartIndex, value);
		if(GPS_ERROR == nextStartIndex){
			return GPS_ERROR;
		}
		++count;
		++nextStartIndex;
	}while(count != Xth);

	return nextStartIndex - 1;
}

#if GPS_CHECKSUM_CONTROL_ON
/**
 * @brief Checksum control for the NMEA GPS Frames, sums all the values between '$' and '*'
 *        then compares with the value after '*'.
 * @param str Address of input array, Complete single address frame such as "$GPRMC....\r\n"
 * @param size Size of the input array
 * @param startIndex Starting index, should be the index of '$' in a single address frame
 * @return GPS_VALID_CHECKSUM: for the correct frames
 *         GPS_INVALID_CHECKSUM: for the incorrect frames
 *         GPS_NO_CHECKSUM_INDICATOR: for the frames that does not include '*'
 */
gps_Status_t gps_controlChecksum(uint8_t * str, size_t size, size_t startIndex){

	uint32_t checksum = 0;

	for(size_t i = startIndex + 1; i < size; i++){
		if(str[i] == CHECKSUM_INDICATOR){
			return ((checksum & 0xFF) == (((str[i + 1] - '0')*16) + (str[i + 2]- '0') )) ? GPS_VALID_CHECKSUM : GPS_INVALID_CHECKSUM;
		}
		checksum ^= str[i];
	}
	return GPS_NO_CHECKSUM_INDICATOR;
}
#endif

/**
 * @brief Finds the desired GPS Address Frame
 * @param str Address of input array
 * @param size Size of the input array, also it can be used as ending index
 * @param startIndex Starting index of the search
 * @param address Desired Address Frame such as GPS_GPRMC, GPS_GPVTG...
 * @return The starting index of the wanted frame, index of the '$' at the start of the frame
 */
int32_t gps_findAddress(uint8_t * str, size_t size, size_t startIndex, gps_addressIdentifier_t address){

	size_t i = startIndex;
	gps_addressIdentifier_t result;
	do{
		i = gps_findIndex(str, size, i, ADDRESS_INDICATOR);
		if((GPS_ERROR == i) || (i + 5 > size)){
			return GPS_ERROR; //TODO:different error returns
		}
		/* $GPRMC: current index->$GPRMC sum = R + M + C -> distinctive for NMEA GPS frames */
		result = str[i + 3] + str[i + 4] + str[i + 5];
		++i;
	}while(address != result);

	return (i - 1);
}

/**
 * @brief Simple converter from ascii to unsigned
 * @param str Address of input array
 * @param startIndex Starting index of the conversion
 * @param endIndex Ending index of the conversion
 * @return Calculated value
 */
uint32_t gps_ascii2num(uint8_t * str, size_t startIndex, size_t endIndex){

	uint32_t sum = 0;
	for(size_t i = 0; i < endIndex - startIndex + 1 ; i++){
		sum += (str[endIndex - i] - '0') * pow(10, i);
	}
	return sum;

}
/**
 * @brief Simple converter from ascii to float
 * @param str Address of input array
 * @param startIndex Starting index of the conversion
 * @param endIndex Ending index of the conversion
 * @return Calculated value
 */
float gps_floatAscii2num(uint8_t * str, size_t startIndex, size_t endIndex){

	float sum = 0.0f;
	uint8_t float_flag = 0, float_index;

	for(size_t i = 0; i < endIndex - startIndex + 1 ; i++){
		if(str[endIndex - i] == FLOAT_SEPERATION_INDICATOR){
			float_flag = 1; //
			float_index = i;
			sum = (float) (sum * ((float) pow(10, (int) (-i))));
		}
		else{
			if(!float_flag){
				sum = sum + (float) ((str[endIndex - i] - '0') * pow(10, i));
			}
			else{
				sum = sum + (float) ((str[endIndex - i] - '0') * pow(10, i - 1 - float_index));
			}
		}
	}
	return (float) sum;
}

/**
 * @brief Simple wrapper for to get value from the single index in array
 * @param str Address of input array
 * @param index Desired index
 * @return Value of the index
 */
uint8_t gps_getSingleChar(uint8_t * str, size_t index){
	return str[index];
}

/**
 * @brief Returns the address of the lookup table for desired address
 * @param address Desired address
 * @return Address the address array
 */
gps_infoType_t * gps_getTable(gps_addressIdentifier_t address){
	switch(address){
#if GPRMC_FRAME_PARSE_ON
	case GPS_GPRMC:
		return (gps_infoType_t *) typeTableGPRMC;
		break;
#endif
#if GPVTG_FRAME_PARSE_ON
	case GPS_GPVTG:
		return (gps_infoType_t *) typeTableGPVTG;
		break;
#endif
#if GPGGA_FRAME_PARSE_ON
	case GPS_GPGGA:
		return (gps_infoType_t *) typeTableGPGGA;
		break;
#endif
#if GPGSA_FRAME_PARSE_ON
	case GPS_GPGSA:
		return (gps_infoType_t *) typeTableGPGSA;
		break;
#endif
#if GPGSV_FRAME_PARSE_ON
	case GPS_GPGSV:
		return (gps_infoType_t *) typeTableGPGSV;
		break;
#endif
#if GPGLL_FRAME_PARSE_ON
	case GPS_GPGLL:
		return (gps_infoType_t *) typeTableGPGLL;
		break;
#endif
	default:
		return NULL;
		break;
	}
}

/**
 * @brief Returns the value and the type of the desired info
 * @param str Address of input array
 * @param startIndex Starting index of the array to be converted
 * @param endIndex Ending index of the array to be converted
 * @param address Lookup table array of the related address frame
 * @param index Order of the desired value
 * @return value and its type
 */
gps_infoData_t gps_getData(uint8_t * str, size_t startIndex, size_t endIndex, gps_infoType_t * address, uint8_t index){

	gps_infoData_t tmp;

	switch(address[index - 1]){
	case GPS_FLOAT_TYPE:
		tmp.typeOfData = GPS_FLOAT_TYPE;
		tmp.typesOfData.fdata = gps_floatAscii2num(str, startIndex + 1, endIndex - 1);
		break;
	case GPS_INTEGER_TYPE:
		tmp.typeOfData = GPS_INTEGER_TYPE;
		tmp.typesOfData.udata = gps_ascii2num(str, startIndex + 1, endIndex - 1);
		break;
	case GPS_CHARACTER_TYPE:
		tmp.typeOfData = GPS_CHARACTER_TYPE;
		tmp.typesOfData.cdata = gps_getSingleChar(str, startIndex + 1);
		break;
	default:
		tmp.typeOfData = GPS_INFO_ERROR;
		break;
	}
	return tmp;
}

/**
 * @brief Returns the desired information from the specific address frame with given order
 * @param str Address of input array
 * @param size Size of the input array, also it can be used as ending index.
               Complete frame of NMEA GPS that includes GPRMC, GPGGA...
 * @param address Address frame that includes desired information
 * @param index Order of the information in frame
 * @return Desired information data and its type
 */
gps_infoData_t gps_getInfo(uint8_t * str, size_t size, gps_addressIdentifier_t address, uint8_t index){

	size_t firstIndex, lastIndex;

	firstIndex = gps_findAddress(str, size, 0, address);
#if GPS_CHECKSUM_CONTROL_ON
		if(GPS_VALID_CHECKSUM != gps_controlChecksum(str, size, firstIndex)){
			gps_infoData_t tmp;
			tmp.typeOfData = GPS_INFO_ERROR;
			return tmp;
	}
#endif
	firstIndex = gps_findXthIndex(str, size, firstIndex, VALUE_SEPERATOR_HEXADECIMAL, index);
	lastIndex = gps_findIndex(str, size, firstIndex + 1, VALUE_SEPERATOR_HEXADECIMAL);
	gps_infoType_t * addressTable = gps_getTable(address);
	if(NULL != addressTable)
		return gps_getData(str, firstIndex, lastIndex, addressTable, index);
	else{
		gps_infoData_t tmp;
		tmp.typeOfData = GPS_INFO_ERROR;
		return tmp;
	}
}
