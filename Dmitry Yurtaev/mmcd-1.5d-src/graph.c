/*
 *  $Id: graph.c,v 1.1.1.1 2004/07/18 20:56:36 yurtaev Exp $
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

#include <PalmOS.h>
#include "mmcd.h"
#include "graph.h"
#include "format.h"
#include "panel.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

extern UInt32 screenWidth, screenHeight, screenDepth;

#define VSIZE 64

UInt8 graphColor[32] = {
	  0,   8,   16, (23),  32,  40,   48,  56,
	 64,  72,   80,   88,  96, 104,  112, 0x50,
//	 64,  72,   80,   88,  96, 104,  (46), 0x50,
	128, 136, 0xdb,  152, 160, 168,  176, 184,
	192, 200,  208,  216, 220, 232,  240, 248
};

/*
 *  draw graph segment from 'from' to 'to-1' in screen coordinates
 */

void _GrfDrawGraphIntervalAny(const GraphType *graph, UInt16 from, UInt16 to, UInt8 index)
{
	Int16 x;
	UInt8 y0 = 0, y1;
	UInt32 dataMask = 1L << index;
	Boolean dataAtLeft = from && (graph->_buffer[from - 1].dataPresent & dataMask);
	Int16 vsize = VSIZE * screenDensity;

	WinSetForeColor(graphColor[index & 31]);

	for(x = from; x <= to; x++) {
		if((graph->_buffer[x].dataPresent & dataMask) == 0) {
			from = x + 1;
			dataAtLeft = false;
			continue;
		}
		if(x == from)
			y0 = vsize - 1 - graph->_buffer[dataAtLeft ? x - 1 : x].data[index] / (256 / vsize);
		y1 = vsize - 1 - graph->_buffer[x].data[index] / (256 / vsize);
		WinDrawLine(x - 1, y0, x, y1);
		y0 = y1;
	}
}

void _GrfDrawGraphInterval1bpp(const GraphType *graph, UInt16 from, UInt16 to, UInt8 index)
{
	register UInt16 x;
	register Int8 dy, s;
	UInt8 y0 = 0, y1;
	register UInt8 y, m = 0, *p = NULL;
	const Int16 modulo = graph->width / 8;
	UInt32 dataMask = 1L << index;
	Boolean dataAtLeft = from && (graph->_buffer[from - 1].dataPresent & dataMask);

	for(x = from; x <= to; x++) {
		// if there's no sample at current position...
		if((graph->_buffer[x].dataPresent & dataMask) == 0) {
			// .. consider next point a beginning of interval 
			from = x + 1;
			// .. set flag that's no data at left
			dataAtLeft = false;
			// .. and start over
			continue;
		}

		// if we are at the beginning of interval...
		if(x == from) {
			// from point may be current or previous
			// sample, if it presents
			y0 = VSIZE - 1 - graph->_buffer[dataAtLeft ? x - 1 : x].data[index] / (256 / VSIZE);
			// calculate a pointer to byte containig first point
			p = graph->_offscreenBits + y0 * modulo + x / 8;
			// position of 'x' in a byte pointed by 'p'
			m = 0x80 >> (x & 7);
		}

		y1 = VSIZE - 1 - graph->_buffer[x].data[index] / (256 / VSIZE);

		// horizontal line
		if(y1 == y0) {
			// do nothing if 'x' == 'from'
			if(from < x) {
				*p |= m;
				(m >>= 1) || (p++, m = 0x80);
			}
			if(x < to) *p |= m;
		} else {
		// slanted line
			if(y1 > y0) {
				dy = y1 - y0;
				s = modulo;
			} else {
				dy = y0 - y1;
				s = -modulo;
			}
			if(from >= x)
				p += s * ((dy + 1) / 2);
			else {
				for(y = (dy + 1) / 2; y; --y) {
					*p |= m;
					p += s;
				}
				(m >>= 1) || (p++, m = 0x80);
			}
			if(x < to) {
				*p |= m;
				for(y = dy / 2; y; --y) {
					p += s;
					*p |= m;
				}
			}
		}
		y0 = y1;
	}
}

void _GrfDrawGraphInterval8bpp(const GraphType *graph, UInt16 from, UInt16 to, UInt8 index)
{
	UInt16 x;
	Int16 s;
	Int8 d;
	UInt8 y0 = 0, y1, y, color, *p = NULL;
	const Int16 modulo = (graph->width + 1) & 0xfffe;
	UInt32 dataMask = 1L << index;
	Boolean dataAtLeft = from && (graph->_buffer[from - 1].dataPresent & dataMask);

	color = graphColor[index & 31];

	for(x = from; x <= to; x++) {
		if((graph->_buffer[x].dataPresent & dataMask) == 0) {
			from = x + 1;
			dataAtLeft = false;
			continue;
		}

		if(from == x) {
			y0 = VSIZE - 1 - graph->_buffer[dataAtLeft ? x - 1 : x].data[index] / (256 / VSIZE);
			// pointer to byte containig first point
			p = graph->_offscreenBits + y0 * modulo + x;
		}

		y1 = VSIZE - 1 - graph->_buffer[x].data[index] / (256 / VSIZE);

		if(y1 == y0) {
			if(x > from) *p++ = color;
			if(x < to) *p = color;
		} else {
			if(y1 > y0) {
				d = y1 - y0;
				s = modulo;
			} else {
				d = y0 - y1;
				s = -modulo;
			}

			// 
			if(x <= from)
				p += s * ((d + 1) / 2);
			else {
				for(y = (d + 1) / 2; y > 0; --y) {
					*p = color;
					p += s;
				}
				p++;
			}

			if(x < to) {
				*p = color;
				for(y = d / 2; y > 0; --y) {
					p += s;
					*p = color;
				}
			}
		}
		y0 = y1;
	}
}

void _GrfDrawTimeScale8bpp(const GraphType *graph)
{
	UInt16 x;
	UInt8 *p = NULL;
	const Int16 modulo = (graph->width + 1) & 0xfffe;
	UInt32 seconds;

	p = graph->_offscreenBits;
	seconds = graph->_buffer[0].time;
	for(x = 1; x < graph->width; x++) {
		if(seconds != graph->_buffer[x].time) {
			seconds = graph->_buffer[x].time;
			*p = p[modulo] = 128;
		}
		p++;
	}
}

// check if current sensor is active
void _GrfCheckSensor(GraphType *graph)
{
	if(graph->_numericMode && graph->numericMask)
		while(((1L << graph->_numericSensorIdx) & graph->numericMask) == 0) {
			graph->_numericSensorIdx = (graph->_numericSensorIdx + 1) % SENSOR_COUNT;
		}
}

/*
 *
 */

void GrfCreateGraph(GraphType *graph, UInt16 id, Coord x, Coord y, Coord width, Coord height, GraphDataReader dataReader)
{
	Err err;

	ErrFatalDisplayIf(graph == NULL, "GrfCreateGraph: graph == NULL");
	ErrFatalDisplayIf(width < 1, "GrfCreateGraph: width < 1");
	ErrFatalDisplayIf(height != VSIZE, "GrfCreateGraph: height != VSIZE");

	// open file, returns file length in samples
	graph->length = dataReader(graph->id, 0, NULL, 0);

	graph->bounds.topLeft.x = x;
	graph->bounds.topLeft.y = y;
	graph->bounds.extent.x = width;
	graph->bounds.extent.y = height;
	graph->attr.usable = 0;
	graph->attr.visible = 0;
	graph->attr.hilighted = 0;
	graph->attr.shown = 0;
	graph->attr.cursorVisible = 1;
	graph->position = 0;
	graph->cursor = 0;                       
	graph->width = width * screenDensity;
	graph->dataMask = 0;

	graph->_numericMode = true;
	graph->_numericSensorIdx = 0;

	graph->_offscreen = WinCreateOffscreenWindow(width, height, nativeFormat, &err);

	switch(screenDensity == 1 ? screenDepth : 0) {
	case 1:	graph->_offscreenDrawFunction = _GrfDrawGraphInterval1bpp; break;
	case 8:	graph->_offscreenDrawFunction = _GrfDrawGraphInterval8bpp; break;
	default: graph->_offscreenDrawFunction = _GrfDrawGraphIntervalAny; break;
	}

	graph->_offscreenBits = BmpGetBits(WinGetBitmap(graph->_offscreen));
	graph->_buffer = MemPtrNew((graph->width + 1) * sizeof(GraphSample));
	graph->_dataReader = dataReader;
}

void GrfEraseGraph(GraphType *graph)
{
	WinHandle oldWin;

	ErrFatalDisplayIf(graph == NULL, "GrfDestroyGraph: graph == NULL");

	MemSet(graph->_buffer, (graph->width + 1) * sizeof(GraphSample), 0);
	WinEraseRectangle(&graph->bounds, 0);
	oldWin = WinSetDrawWindow(graph->_offscreen);
	WinEraseWindow();
	WinSetDrawWindow(oldWin);

	graph->attr.shown = 0;
}

void GrfSetMode(GraphType *graph, Boolean numeric)
{
	graph->_numericMode = numeric;
//	_GrfCheckSensor(graph);
}

void GrfAppendSample(GraphType *graph, GraphSample *newSample)
{
	static struct GraphSample lastSample;

	ErrFatalDisplayIf(graph == NULL, "GrfDestroyGraph: graph == NULL");

	if(graph->_numericMode) {
		static UInt32 lastUpdateTime;

		if(newSample) {
			lastSample = graph->_buffer[graph->width] = *newSample;
			// scroll in-memory sample data 1 sample left
			MemMove(graph->_buffer, graph->_buffer + 1, graph->width * sizeof(GraphSample));
		} else {
			newSample = &lastSample;
			lastUpdateTime = 0;	// force refresh
		}

		// update display 4 times per second
		if(TimGetTicks() > lastUpdateTime + ticksPerSecond / 4) {
			FontID oldFont;
			UInt16 strWidth, strLen;
			Char strbuf[16] = "\0              \0";
			Char *str = strbuf;
			RectangleType rect = { { 0, 0 }, graph->bounds.extent };
			WinHandle oldWin = WinSetDrawWindow(graph->_offscreen);
	
			WinEraseWindow();
			lastUpdateTime = TimGetTicks();

			// draw value
			if(newSample && (newSample->dataPresent & 1L << graph->_numericSensorIdx)) {
				oldFont = FntSetFont(fntAppFontCustomBase);
				_pnlSensor[graph->_numericSensorIdx].format(newSample->data[graph->_numericSensorIdx], str);

				if((strLen = StrLen(str))) {
					strWidth = FntCharsWidth(str, strLen);
					WinDrawChars(str, strLen,
						rect.topLeft.x + (rect.extent.x - strWidth) / 2,
						rect.topLeft.y + ((Int16)rect.extent.y - FntCharHeight()) / 2
					);
				}
				FntSetFont(oldFont);
			}

			// draw label
			oldFont = FntSetFont(boldFont);
			str = (Char*)_pnlSensor[graph->_numericSensorIdx].slug;
			if((strLen = StrLen(str))) {
				strWidth = FntCharsWidth(str, strLen);

				WinDrawChars(str, strLen,
					rect.topLeft.x + (rect.extent.x - strWidth) / 2,
					rect.topLeft.y + rect.extent.y - FntCharHeight() - 1
				);
			}
			FntSetFont(oldFont);

			// blit offscreen window to display
			WinCopyRectangle(graph->_offscreen, oldWin, &rect, graph->bounds.topLeft.x, graph->bounds.topLeft.y, winPaint);
			WinSetDrawWindow(oldWin);
		}
	} else {
		if(newSample) {
			// scroll offscreen window 1 pixel left
			UInt16 oldCoordSys = screenDensity > 1 ? WinSetCoordinateSystem(kCoordinatesDouble) : 0;
			RectangleType scrollRect = { { 0, 0 }, { graph->bounds.extent.x * screenDensity, graph->bounds.extent.y * screenDensity } }, r;
			WinHandle oldWin = WinSetDrawWindow(graph->_offscreen);
			WinScrollRectangle(&scrollRect, winLeft, 1, &r);
			WinSetDrawWindow(oldWin);
			if(screenDensity > 1) WinSetCoordinateSystem(oldCoordSys);

			// place new sample at the end of the buffer
			graph->_buffer[graph->width] = *newSample;
			// scroll in-memory sample data delta samples left
			MemMove(graph->_buffer, graph->_buffer + 1, graph->width * sizeof(GraphSample));
			GrfDrawGraph(graph, graph->width - 2, graph->width);
		} else {
			// no new data, refresh requested
			GrfDrawGraph(graph, 0, graph->width);
		}
	}
}

void GrfDestroyGraph(GraphType *graph)
{
	ErrFatalDisplayIf(graph == NULL, "GrfDestroyGraph: graph == NULL");

	WinDeleteWindow(graph->_offscreen, false);
	MemPtrFree(graph->_buffer);
	WinEraseRectangle(&graph->bounds, 0);

	graph->attr.usable = 0;
	graph->attr.visible = 0;
}


void GrfGetGraph(const GraphType *graph, Int32 *length, Int32 *position, Int32 *cursor, UInt32* mask)
{
	ErrFatalDisplayIf(graph == NULL, "GrfDestroyGraph: graph == NULL");

	if(length) *length = graph->length;
	if(position) *position = graph->position;
	if(cursor) *cursor = graph->cursor;
	if(mask) *mask = graph->dataMask;
}


void GrfDrawGraph(const GraphType *graph, UInt16 from, UInt16 to)
{
	RectangleType fullRect, drawRect;
	WinHandle oldWin;
	UInt32 i;

	ErrFatalDisplayIf(graph == NULL, "GrfDrawGraph: graph == NULL");

	if(graph->_numericMode) return;

	// check bounds
	if(to > graph->width) to = graph->width;
	if(from >= to) return;

	// rectangle we are painting into
	drawRect.topLeft.x = from;
	drawRect.topLeft.y = 0;
	drawRect.extent.x = to - from;
	drawRect.extent.y = graph->bounds.extent.y * screenDensity;

	// full offscreen window bounds
	fullRect.topLeft.x = 0;
	fullRect.topLeft.y = 0;
	fullRect.extent.x = graph->bounds.extent.x * screenDensity;
	fullRect.extent.y = graph->bounds.extent.y * screenDensity;

	// paint to offscreen
	oldWin = WinSetDrawWindow(graph->_offscreen);
	WinPushDrawState();
	if(screenDensity > 1) WinSetCoordinateSystem(kCoordinatesDouble);

	// erase from..to range of graph
	WinEraseRectangle(&drawRect, 0);

	// draw graphs for active values
	for(i = 0; i < 32; ++i) {
		if(graph->dataMask & (1L << i))
			graph->_offscreenDrawFunction(graph, from, to, i);
	}

//	_GrfDrawTimeScale(graph);

	// draw cursor, if within from..to range
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	if(graph->attr.cursorVisible
		&& graph->cursor - graph->position >= from
		&& graph->cursor - graph->position < to
		&& graph->cursor < graph->length // no cursor if graph is empty
	) {
		WinDrawLine(graph->cursor - graph->position, 0, graph->cursor - graph->position, graph->bounds.extent.y * screenDensity - 1);
	}
	if(screenDensity > 1) WinSetCoordinateSystem(kCoordinatesStandard);

	// blit offscreen window to display
	WinCopyRectangle(graph->_offscreen, oldWin, &fullRect, graph->bounds.topLeft.x, graph->bounds.topLeft.y, winPaint);
	WinPopDrawState();
	WinSetDrawWindow(oldWin);
}

/*
 *
 */

void GrfSetMask(GraphType *graph, UInt32 mask)
{
	ErrFatalDisplayIf(graph == NULL, "GrfSetMask: graph == NULL");

	graph->dataMask = mask;
	GrfDrawGraph(graph, 0, graph->width);
}

void GrfSetNumericMask(GraphType *graph, UInt32 mask)
{
	ErrFatalDisplayIf(graph == NULL, "GrfSetNumericMask: graph == NULL");

	graph->numericMask = mask;
//	_GrfCheckSensor(graph);
}

void GrfShowCursor(GraphType *graph, Boolean visible)
{
	graph->attr.cursorVisible = !!visible;
}
/*
 *
 */

void GrfUpdateGraph(GraphType *graph)
{
	graph->length = graph->_dataReader(graph->id, 0, NULL, 0);
	graph->attr.shown = 0;
	if(graph->attr.visible)
		GrfSetGraph(graph, graph->length, graph->position, graph->cursor, graph->dataMask);
}

void GrfSetGraph(GraphType *graph, Int32 length, Int32 position, Int32 cursor, UInt32 mask)
{
	Boolean redrawAll;
	Int32 scrollDelta;
	static RectangleType r;

	ErrFatalDisplayIf(graph == NULL, "GrfSetGraph: graph == NULL");

//	ErrFatalDisplayIf(length < 0, "GrfSetGraph: length < 0");
//	ErrFatalDisplayIf(position < 0 || (length > 0 && position >= length), "GrfSetGraph: position < 0 || position >= length");
//	ErrFatalDisplayIf(cursor < 0 || (length > 0 && cursor >= length), "GrfSetGraph: cursor < 0 || cursor >= length");

//	graph->length = length;

	// check bounds
	if(position > graph->length - graph->width) position = graph->length - graph->width;
	if(position < 0) position = 0;
	if(cursor >= graph->length) cursor = graph->length - 1;
	if(cursor < 0) cursor = 0;

	// if mask has changed or graph hasn't been drawn yet - redraw all
	redrawAll = (mask != graph->dataMask) || !graph->attr.shown;
	graph->dataMask = mask;

	if(cursor != graph->cursor) {
		Int32 oldCursor = graph->cursor;
		graph->cursor = cursor;

		if(!redrawAll && graph->attr.cursorVisible) {
			// is cursor visible?
			if(oldCursor >= graph->position && oldCursor < graph->position + graph->width) {
				// if so, erase it
				GrfDrawGraph(graph, oldCursor - graph->position, oldCursor - graph->position + 1);
			}
        		if(cursor >= graph->position && cursor < graph->position + graph->width) {
				// draw cursor at new position
				GrfDrawGraph(graph, cursor - graph->position, cursor - graph->position + 1);
			}
		}
	}

	scrollDelta = position - graph->position;
	graph->position = position;

	if(!graph->attr.shown || scrollDelta <= -graph->width + 10L || scrollDelta >= graph->width - 10L) {
		graph->_dataReader(graph->id,
			position,
			graph->_buffer,
			MIN(graph->length - position, graph->width + 1));
		if(graph->length - position < graph->width + 1) {
			MemSet(graph->_buffer + graph->length - position,
				(graph->width + 1 - graph->length + position) * sizeof(GraphSample), 0);
		}
		GrfDrawGraph(graph, 0, graph->width);
		graph->attr.shown = 1;
	} else if(scrollDelta > 0) {
		// scroll offscreen window delta pixels left
		UInt16 oldCoordSys = screenDensity > 1 ? WinSetCoordinateSystem(kCoordinatesDouble) : 0;
		RectangleType scrollRect = { { 0, 0 }, { graph->bounds.extent.x * screenDensity, graph->bounds.extent.y * screenDensity } };
		WinHandle oldWin = WinSetDrawWindow(graph->_offscreen);
		WinScrollRectangle(&scrollRect, winLeft, scrollDelta, &r);
		WinSetDrawWindow(oldWin);
		if(screenDensity > 1) WinSetCoordinateSystem(oldCoordSys);

		// scroll in-memory sample data delta samples left
		MemMove(graph->_buffer, graph->_buffer + scrollDelta,
			sizeof(GraphSample) * (graph->width + 1 - scrollDelta));

		// read sample range
		graph->_dataReader(graph->id,
			position + graph->width - scrollDelta + 1,
			graph->_buffer + graph->width + 1 - scrollDelta,
			scrollDelta);

		// draw sample range
		if(redrawAll)
			GrfDrawGraph(graph, 0, graph->width);
		else
			GrfDrawGraph(graph, graph->width - scrollDelta - 1, graph->width);

	} else if(scrollDelta < 0) {
		// scroll offscreen window delta pixels right
		UInt16 oldCoordSys = screenDensity > 1 ? WinSetCoordinateSystem(kCoordinatesDouble) : 0;
		RectangleType scrollRect = { { 0, 0 }, { graph->bounds.extent.x * screenDensity, graph->bounds.extent.y * screenDensity } };
		WinHandle oldWin = WinSetDrawWindow(graph->_offscreen);
		WinScrollRectangle(&scrollRect, winRight, -scrollDelta, &r);
		WinSetDrawWindow(oldWin);
		if(screenDensity > 1) WinSetCoordinateSystem(oldCoordSys);

		scrollDelta = -scrollDelta;

		// scroll in-memory sample data delta samples right
		MemMove(graph->_buffer + scrollDelta, graph->_buffer,
			sizeof(GraphSample) * (graph->width + 1 - scrollDelta));

		// read sample range
		graph->_dataReader(graph->id,
			position,
			graph->_buffer,
			scrollDelta);

		// draw sample range
		if(redrawAll)
			GrfDrawGraph(graph, 0, graph->width);
		else
			GrfDrawGraph(graph, 0, scrollDelta + 1);

	} else if(redrawAll) {
		GrfDrawGraph(graph, 0, graph->width);
	}

	graph->attr.usable = 1;
	graph->attr.visible = 1;
}

UInt8 GrfSetSensor(GraphType *graph, UInt8 index)
{
	if(index < SENSOR_COUNT) {
		graph->_numericSensorIdx = index;
	} else {
		// next available in numericMask
		if(graph->_numericMode && graph->numericMask)
			do {
				graph->_numericSensorIdx = (graph->_numericSensorIdx + 1) % 32;
			} while(((1L << graph->_numericSensorIdx) & graph->numericMask) == 0);
	}

	// if in numeric. refresh display instantly
	if(graph->_numericMode) GrfAppendSample(graph, NULL);

	return graph->_numericSensorIdx;
}

Boolean	GrfHandleEvent(GraphType *graph, const EventType *e)
{
	GraphEventType newEvent;
	Int32 newCursor;
	Int32 newPosition;
	Int16 screenX = e->screenX;
	Int16 screenY = e->screenY;
	Boolean penDown;

	ErrFatalDisplayIf(graph == NULL, "GgfHandleEvent: graph == NULL");

	if(!graph->attr.usable || !graph->attr.visible) return false;
	newPosition = graph->position;

	if(screenDensity > 1) EvtGetPenNative(WinGetDisplayWindow(), &screenX, &screenY, &penDown);

	switch(e->eType) {

	case penDownEvent:
		if(graph->attr.cursorVisible && RctPtInRectangle(e->screenX, e->screenY, &graph->bounds)) {
//		if(RctPtInRectangle(e->screenX, e->screenY, &graph->bounds)) {
//			if(graph->_numericMode) {
//				SndPlaySystemSound(sndClick);
//				GrfNextSensor(graph);
//			} else 
			if(graph->attr.cursorVisible) {
				newCursor = graph->position + screenX - graph->bounds.topLeft.x * screenDensity;
				if(newCursor >= graph->length) newCursor = graph->length - 1;
				if(newCursor < 0) newCursor = 0;
				graph->attr.hilighted = 1;
				graph->attr.activeRegion = graphNone;
				goto sendRepeatEvent;
			}
		}
		break;

	case penUpEvent:
		if(graph->attr.hilighted) {
			graph->attr.hilighted = 0;
			return true;
		}
		break;

	case penMoveEvent:
		if(graph->attr.hilighted) {
			newCursor = graph->position + screenX - graph->bounds.topLeft.x * screenDensity;
			if(newCursor >= graph->length) newCursor = graph->length - 1;

			if(newCursor <= newPosition) {
				newPosition = MAX(0, newPosition - 4 * screenDensity);
				newCursor = newPosition;
				graph->attr.activeRegion = graphLeft;
			} else if(newCursor >= newPosition + graph->width - 1) {
				newPosition = MIN(graph->length - graph->width, newPosition + 4 * screenDensity);
				if(newPosition < 0) newPosition = 0;
				newCursor = newPosition + graph->width - 1;
				if(newCursor < 0) newCursor = 0;
				graph->attr.activeRegion = graphRight;
			} else {
				graph->attr.activeRegion = graphNone;
			}
sendRepeatEvent:
			if(newPosition != graph->position || newCursor != graph->cursor) {
				EvtCopyEvent(e, (EventType*)&newEvent);
				newEvent.eType = grfChangeEvent;
				newEvent.data.grfChange.positionChanged = (newPosition != graph->position);
				newEvent.data.grfChange.cursorChanged = (newCursor != graph->cursor);
				newEvent.data.grfChange.lengthChanged = 0;

				GrfSetGraph(graph, graph->length, newPosition, newCursor, graph->dataMask);

				newEvent.data.grfChange.graphID = graph->id;
				newEvent.data.grfChange.position = graph->position;
				newEvent.data.grfChange.cursor = graph->cursor;
				newEvent.data.grfChange.length = graph->length;
				EvtAddUniqueEventToQueue((EventType*)&newEvent, graph->id, false);
			}
			return true;
		}
		break;

	case grfChangeEvent:
		break;

	default:
		if(graph->attr.hilighted && graph->attr.activeRegion == graphLeft) {
			newPosition = MAX(0, graph->position - 4);
			newCursor = newPosition;
			goto sendRepeatEvent;
		}
		if(graph->attr.hilighted && graph->attr.activeRegion == graphRight) {
			newPosition = MIN(graph->length - graph->width, graph->position + 4);
			if(newPosition < 0) newPosition = 0;
			newCursor = newPosition + graph->width - 1;
			if(newCursor < 0) newCursor = 0;
			goto sendRepeatEvent;
		}
		break;
	}
	return false;
}
