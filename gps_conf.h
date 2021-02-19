/**
 ********************************************************************************
 * @file    gps_conf.h
 * @author  Embedded Area
 * @date    Jan 9, 2021
 * @brief   Configuration file of the NEO_6M GPS Module
 *
 * Configution file of the library.
 * There are 6 address frames in single response
 * from the module. In default configuration user can be accessed information from
 * all of the frames. In case of some specific information access, if there is any
 * frame that its elements are not used, it is best to comment out its definition.
 * Therefore, its components will be discarded. This will provide enhancement in
 * both performance and memory.
 * Checksum control is implemented in library, but it is up to user to enable it.
 * It is disabled in default.
 ********************************************************************************
 */

#ifndef INC_GPS_CONF_H_
#define INC_GPS_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************************
 * INCLUDES
 ************************************/

/************************************
 * MACROS AND DEFINES
 ************************************/

/**
 * Comment out to disable an address frame that are not going to be used
 *
 */
#define GPRMC_FRAME_PARSE_ON_DEFINED
#define GPVTG_FRAME_PARSE_ON_DEFINED
#define GPGGA_FRAME_PARSE_ON_DEFINED
#define GPGSA_FRAME_PARSE_ON_DEFINED
#define GPGSV_FRAME_PARSE_ON_DEFINED
#define GPGLL_FRAME_PARSE_ON_DEFINED

#ifdef GPRMC_FRAME_PARSE_ON_DEFINED
#define GPRMC_FRAME_PARSE_ON 1
#else
#define GPRMC_FRAME_PARSE_ON 0
#endif


#ifdef GPVTG_FRAME_PARSE_ON_DEFINED
#define GPVTG_FRAME_PARSE_ON 1
#else
#define GPVTG_FRAME_PARSE_ON 0
#endif


#ifdef GPGGA_FRAME_PARSE_ON_DEFINED
#define GPGGA_FRAME_PARSE_ON 1
#else
#define GPGGA_FRAME_PARSE_ON 0
#endif


#ifdef GPGSA_FRAME_PARSE_ON_DEFINED
#define GPGSA_FRAME_PARSE_ON 1
#else
#define GPGSA_FRAME_PARSE_ON 0
#endif


#ifdef GPGSV_FRAME_PARSE_ON_DEFINED
#define GPGSV_FRAME_PARSE_ON 1
#else
#define GPGSV_FRAME_PARSE_ON 0
#endif


#ifdef GPGLL_FRAME_PARSE_ON_DEFINED
#define GPGLL_FRAME_PARSE_ON 1
#else
#define GPGLL_FRAME_PARSE_ON 0
#endif


/**
 *  Comment out to enable checksum control of the frames
 *
 */
//#define GPS_CHECKSUM_CONTROL_ON_DEFINED
#ifdef GPS_CHECKSUM_CONTROL_ON_DEFINED
#define GPS_CHECKSUM_CONTROL_ON 1
#else
#define GPS_CHECKSUM_CONTROL_ON 0
#endif

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * EXPORTED VARIABLES
 ************************************/

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/


#ifdef __cplusplus
}
#endif

#endif
