/*
 * mag_strip.h
 *
 *  Created on: 8 sep. 2025
 *      Author: Usuario
 */

#ifndef DRV_MAG_STRIP_H_
#define DRV_MAG_STRIP_H_

/**
 * @brief Initialize Magnetic Strip's communication.
 * @return TRUE if correctly initialized
 * @return FALSE if previously initialized
 */
int magStrip_Init(void);
/**
 * @brief Gets value of data flag
 * @return TRUE if data is ready for processing
 * @return FALSE if data is not ready for processing
 */
bool isMagDataReady(void);
/**
 * @brief Converts magnetic strip's data from string to an integer.
 * 		  It also disables data readability.
 * @param pan: Pointer to variable where primary account number is
 * 			   going to be stored.
 * @param add_data: Pointer to  where additional data
 * 					(Expiration data(YYMM) + Service code(SSC))
 * 					Its digits follow the format: YYMMSSC
 * 	@param disc_data: Pointer to discretionary data (PVKI(1 digit),
 * 					  PVV or Offset(4 digits) and
 * 					  CVV or CVC(3 digits))
 * 					  NOTE: Some or all of the above fields may not be found.
 *
 * @return: TRUE: Data has been successfully stored in pointers adresses.
 * @return FALSE: Data read is not valid.
 */
int processStripData(uint64_t *pan, uint32_t *add_data, uint32_t *disc_data);

/**
 * @brief Validates magnetic strip's track 2 data. It should end with
 * 		END_SENTINEL and all characters should have an odd quantity
 * 		of bits turned on.
 * @return 0: Data is invalid: it doesn't have FIELD SEPARATOR or END_SENTINEL,
 * 			  or at least one	character read has an even quantity of bits
 * 		      turned on, or string length = 0
 * @return N: N the length of the string read, doesn't takes into account
 * 	           END_SENTINEL
 */
uint8_t validateData(void);

// ToDo comment
void resetMagData(void);

#endif /* DRV_MAG_STRIP_H_ */
