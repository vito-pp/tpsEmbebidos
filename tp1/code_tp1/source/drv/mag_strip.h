/*
 * mag_strip.h
 *
 *  Created on: 8 sep. 2025
 *      Author: Usuario
 */

#ifndef DRV_MAG_STRIP_H_
#define DRV_MAG_STRIP_H_

int magStrip_Init(void);
int processStripData(void);
int getIsDataReady(void);

#endif /* DRV_MAG_STRIP_H_ */
