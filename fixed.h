/*
 * fixed.h
 *
 *  Created on: 11 juli 2017
 *      Author: thene_000
 */

#ifndef FIXED_H_
#define FIXED_H_

#include "typedef.h"

typedef u16 fixed_t;
#define fixed(Dec, Frac) ((Dec << 8) | (Frac))
#define fix2i(Fixed) ((Fixed >> 8))
#define ffrac(Fixed) ((Fixed & 0xFF))

#endif /* FIXED_H_ */
