/***************************************************************************//**
  @file     CAN_SPI.h
  @brief    CAN_SPI driver
  @author   Grupo 5
 ******************************************************************************/

#ifndef CAN_SPI_H_
#define CAN_SPI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BUFFER_SIZE2	8

typedef struct RXB_RAWDATA
{
    uint16_t SID;
    uint8_t DLC;
    uint8_t Dn[BUFFER_SIZE2];
}RXB_RAWDATA_t;

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/**
 * @brief
 * @return
 */
void CAN_SPI_Init (void);

/**
 * @brief
 * @return
 */
uint8_t CAN_SPI_Is_Read_Ready(void);

/**
 * @brief
 * @return
 */
RXB_RAWDATA_t CAN_SPI_Get_Data(void);

/**
 * @brief
 * @return
 */
void CAN_SPI_SendInfo(RXB_RAWDATA_t * rawdata);

uint8_t CAN_SPI_Attempt_to_read(void);

uint8_t CAN_SPI_Is_Busy(void);


/*******************************************************************************
 ******************************************************************************/

#endif /* CAN_SPI_H_ */
