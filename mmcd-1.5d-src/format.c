/* 
 *  $Id: format.c,v 1.1.1.1 2004/07/18 20:56:34 yurtaev Exp $
 *
 *  Copyright (c) 2003, Dmitry Yurtaev <dm1try@umail.ru>
 *
 *  This is free software; you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */

#include <StringMgr.h>
#include "format.h"
#include "panel.h"

#define DEG "\260"	// degree symbol

extern UInt8 metricUnit;

/*

MMC Galant E3x, MMC Eclipse 1G, ...

*/


// just decimal value 0..255
Int16 f_DEC(UInt8 d, Char *s) {
	return StrPrintF(s, "%u", (UInt16)d);
}

// hexadecimal value
Int16 f_HEX(UInt8 d, Char *s) {
	return StrPrintF(s, "%x", d);
}

// Air Conditioner clutch
Int16 f_FLG0(UInt8 d, Char *s) {
	return StrPrintF(s, "%s", d & 0x20 ? "-" : "A");
}

// ...
Int16 f_FLG2(UInt8 d, Char *s) {
	return StrPrintF(s, "%s%s%s%s%s", 
		d & 0x04 ? "-" : "T",
		d & 0x08 ? "S" : "-",
		d & 0x10 ? "-" : "A",
		d & 0x20 ? "-" : "N",
		d & 0x80 ? "I" : "-"
	);
}


// Air Temperature, degrees C
// interp - 60
Int16 f_AIRT(UInt8 d, Char *s) {
	static const UInt8 interp[17] = {
		0xF4, 0xB0, 0x91, 0x80, 0x74, 0x6A, 0x62, 0x5A,
		0x53, 0x4C, 0x45, 0x3E, 0x35, 0x2B, 0x1D, 0x01,
		0x01
	};

	UInt8 v1 = interp[d / 16], v2 = interp[d / 16 + 1];

	if(metricUnit == PREFS_UNIT_METRIC) {
		Int32 v = v1 * 65536L - (d % 16) * (v1 - v2) * 4096L - 60 * 65536L;
		return StrPrintF(s, "%ld.%ld" DEG "C", v >> 16, (v & 0xffff) * 10 >> 16);
	} else if(metricUnit == PREFS_UNIT_ENGLISH) {
		Int32 v = v1 * 65536L * 9/5 - (d % 16) * (v1 - v2) * 4096L  * 9/5 - 60 * 65536L * 9/5 + 32 * 65536L;
		return StrPrintF(s, "%ld.%ld" DEG "F", v >> 16, (v & 0xffff) * 10 >> 16);
	} else {
		return f_DEC(d, s);
	}
}

// Coolant temperature, degrees C
// interp - 80
Int16 f_COOL(UInt8 d, Char *s) {
	static const UInt8 interp[17] = {
		0xEE, 0xBE, 0xA0, 0x90, 0x84, 0x7B, 0x73, 0x6C,
		0x65, 0x5F, 0x58, 0x51, 0x49, 0x40, 0x33, 0x15,
		0x15
	};

	UInt8 v1 = interp[d / 16], v2 = interp[d / 16 + 1];
	
        if(metricUnit == PREFS_UNIT_METRIC) {
		Int32 v = v1 * 65536L - (d % 16) * (v1 - v2) * 4096L - 80 * 65536L;
		return StrPrintF(s, "%ld.%ld" DEG "C", v >> 16, (v & 0xffff) * 10 >> 16);
	} else if(metricUnit == PREFS_UNIT_ENGLISH) {
		Int32 v = v1 * 65536L * 9/5 - (d % 16) * (v1 - v2) * 4096L  * 9/5 - 80 * 65536L * 9/5 + 32 * 65536L;
		return StrPrintF(s, "%ld.%ld" DEG "F", v >> 16, (v & 0xffff) * 10 >> 16);
	} else {
		return f_DEC(d, s);
	}
}

// EGRT Temperature, degrees C
// -1.5x + 314.2(7)
// F: -2.7x + 597.7
Int16 f_EGRT(UInt8 d, Char *s) {
	if(metricUnit == PREFS_UNIT_METRIC) {
		Int32 v = -98304L * d + 20596508L;
		return StrPrintF(s, "%ld.%ld" DEG "C", v >> 16, (v & 0xffff) * 10 >> 16);
	} else if(metricUnit == PREFS_UNIT_ENGLISH) {
		Int32 v = -176947L * d + 39170867L;
		return StrPrintF(s, "%ld.%ld" DEG "F", v >> 16, (v & 0xffff) * 10 >> 16);
	} else {
		return f_DEC(d, s);
	}
}

// Battery voltage, 0.0..18.7 volts
// 0.0733 * x
Int16 f_BATT(UInt8 d, Char *s) {
	Int32 v = 4804L * d;
	return StrPrintF(s, "%ld.%ldV", v >> 16, (v & 0xffff) * 10 >> 16);
}

// Engine speed, 0..8000 rpm
// 31.25 * x
Int16 f_ERPM(UInt8 d, Char *s) {
	Int32 v = 2048000L * d;
	return StrPrintF(s, "%ldrpm", v >> 16);
}

// Injector pulse duty period, 0.0..65.0 ms
// 0.256 * x
Int16 f_INJP(UInt8 d, Char *s) {
	Int32 v = 16777L * d;
	return StrPrintF(s, "%ld.%02ldms", v >> 16, (v & 0xffff) * 100 >> 16);
}

// Barometric pressure, 0.00..1.24 bar
// 0.00486 * x
// 1 bar = 14.50326 psi
Int16 f_BARO(UInt8 d, Char *s) {
	if(metricUnit == PREFS_UNIT_METRIC) {
		Int32 v = 319L * d;
		return StrPrintF(s, "%ld.%02ldbar", v >> 16, (v & 0xffff) * 100 >> 16);
	} else if(metricUnit == PREFS_UNIT_ENGLISH) {
		Int32 v = 4619L * d;
		return StrPrintF(s, "%ld.%02ldpsi", v >> 16, (v & 0xffff) * 100 >> 16);
	} else {
		return f_DEC(d, s);
	}
}

// Mass air flow sensor, 0.0..1600.0 Hz
// 6,29 * x
Int16 f_AIRF(UInt8 d, Char *s) {
	Int32 v = 412221L * d;
	return StrPrintF(s, "%ld.%ldHz", v >> 16, (v & 0xffff) * 10 >> 16);
}

// Throttle position, 0..100%
Int16 f_THRL(UInt8 d, Char *s) {
	Int32 v = 25600L * d;
	return StrPrintF(s, "%ld.%ld%%", v >> 16, (v & 0xffff) * 10 >> 16);
}

// Fuel trim, 0..200%
// f: 0.7843 * x
Int16 f_FTxx(UInt8 d, Char *s) {
	Int32 v = 51200L * d;
	return StrPrintF(s, "%ld.%ld%%", v >> 16, (v & 0xffff) * 10 >> 16);
}

// Oxygen sensor voltage, 0.00..5.00 volts
// f: 0.0196 * x
Int16 f_OXYG(UInt8 d, Char *s) {
	Int32 v = 1280L * d;
	return StrPrintF(s, "%ld.%02ldV", v >> 16, (v & 0xffff) * 100 >> 16);
}

// Timing advance, -10 .. 245 deg
// f: x - 10
Int16 f_TIMA(UInt8 d, Char *s) {
	return StrPrintF(s, "%ld" DEG, (Int32)d - 10);
}
