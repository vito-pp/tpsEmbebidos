/*
 * map.h
 *
 *  Created on: 12 jul. 2026
 *      Author: Usuario
 */

#ifndef MAP_H_
#define MAP_H_

/**
 * @brief Updates the LED matrix to display the current occupancy map.
 *
 * Reads the occupancy of each floor, assigns a unique color to each person,
 * generates an 8x8 color map, and sends it to the LED matrix.
 *
 * Floor distribution:
 *   - Columns 0-1 : Floor 1
 *   - Columns 2-3 : Floor 2
 *   - Columns 4-5 : Floor 3
 *   - Columns 6-7 : Reserved
 *
 * Person colors:
 *   - 1st person : Red
 *   - 2nd person : Blue
 *   - 3rd person : Green
 *   - 4th person : Purple
 *
 * @note Each person is represented by a 2x2 block of LEDs.
 *       Occupancy grows from top to bottom.
 */
void loadMap(void);

#endif /* MAP_H_ */
