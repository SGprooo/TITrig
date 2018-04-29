//
// Created by Decimation on 4/27/2018.
//

#include "GraphicsExt.h"
#include "Library.h"
#include "Triangle.h"
#include <tice.h>
#include <graphx.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <stdbool.h>



void gfx_RadicalFraction(gfx_point_t point, int24_t numOuter, int24_t numInner, int24_t denomOuter, int24_t denomInner)
{
	char numInnerBuf[10], numOuterBuf[10];
	char denomInnerBuf[10], denomOuterBuf[10];

	int numLength, denomLength;

	sprintf(numOuterBuf, "%d", numOuter);
	sprintf(numInnerBuf, "%d", numInner);

	sprintf(denomOuterBuf, "%d", denomOuter);
	sprintf(denomInnerBuf, "%d", denomInner);

	numLength   = (strlen(numOuterBuf) + strlen(numInnerBuf));
	denomLength = (strlen(denomOuterBuf) + strlen(denomInnerBuf));

	//todo: center the fractions in these two cases
	if (numLength > denomLength)
	{
		gfx_HorizLine(point.x - 1, point.y, gfx_GetStringWidth(numInnerBuf) + gfx_GetStringWidth(numOuterBuf) + 7);

		point.y -= 9;
		gfx_Sqrt(point, numOuter, numInner);

		point.y += 14;
		gfx_Sqrt(point, denomOuter, denomInner);
	}
	else if (numLength < denomLength)
	{
		gfx_HorizLine(point.x - 1, point.y, gfx_GetStringWidth(denomInnerBuf) + gfx_GetStringWidth(denomOuterBuf) + 7);

		point.y -= 9;
		gfx_Sqrt(point, numOuter, numInner);

		point.y += 14;
		gfx_Sqrt(point, denomOuter, denomInner);
	}
	else
	{
		gfx_HorizLine(point.x - 1, point.y, gfx_GetStringWidth(numInnerBuf) + gfx_GetStringWidth(numOuterBuf) + 7);

		point.y -= 9;
		gfx_Sqrt(point, numOuter, numInner);

		point.y += 14;
		gfx_Sqrt(point, denomOuter, denomInner);

	}
}

void gfx_Fraction(gfx_point_t point, real_t num, real_t denom)
{
	char numbuf[10];
	char denombuf[10];
	os_RealToStr(numbuf, &num, 0, 0, 6);
	os_RealToStr(denombuf, &denom, 0, 0, 6);
	gfx_FractionStr(point, numbuf, denombuf);
}

void gfx_FractionStr(gfx_point_t point, const char* num, const char* denom)
{

	if (strlen(num) > strlen(denom))
	{
		gfx_HorizLine(point.x - 1, point.y + 3, gfx_GetStringWidth(num) + 1);

		gfx_PrintStringXY(denom, point.x + ((gfx_GetStringWidth(denom) + 1) / 2), point.y + 5);
		gfx_PrintStringXY(num, point.x, point.y - 5);
	}
	else if (strlen(num) < strlen(denom))
	{
		gfx_HorizLine(point.x - 1, point.y + 3, gfx_GetStringWidth(denom) + 1);

		gfx_PrintStringXY(num, point.x + ((gfx_GetStringWidth(num) + 1) / 2), point.y - 5);
		gfx_PrintStringXY(denom, point.x, point.y + 5);
	}
	else
	{
		gfx_HorizLine(point.x - 1, point.y + 3, gfx_GetStringWidth(denom) + 1);

		gfx_PrintStringXY(num, point.x, point.y - 5);
		gfx_PrintStringXY(denom, point.x, point.y + 5);
	}
}

void gfx_Sqrt(gfx_point_t point, int24_t outer, int24_t inner)
{
	char buf[10];
	sprintf(buf, "%d", outer);

	//190, 155
	gfx_PrintStringXY(buf, point.x, point.y);

	//198, 158 -> 200, 160
	gfx_Line(point.x + 8, point.y + 3, point.x + 10, point.y + 7);

	//205, 155
	gfx_VertLine(point.x + 11, point.y - 3, 11);

	sprintf(buf, "%d", inner);
	gfx_PrintStringXY(buf, point.x + 14, point.y);

	gfx_HorizLine(point.x + 11, point.y - 3, gfx_GetStringWidth(buf) + 3);
}

void gfx_Clear(const superpoint_t* p)
{
	const int w = gfx_GetStringWidth(p->label);
	gfx_SetColor(gfx_white);
	gfx_FillRectangle(p->point.x, p->point.y, w, 9);
	gfx_SetColor(gfx_blue);
}

void gfx_Print(const superpoint_t* p)
{
	gfx_PrintStringXY(p->label, p->point.x, p->point.y);
}

void sp_SetLabel(superpoint_t* p, const char* s)
{
	gfx_Clear(p);
	lib_MemZero(p->label, 20);
	strncpy(p->label, s, strlen(s));
}

real_t io_gfx_ReadReal(superpoint_t* point)
{
	bool        isNeg    = false;
	uint8_t     key, i   = 0;
	real_t      rbuffer;
	static char lchars[] = "\0\0\0\0\0\0\0\0\0\0\"-RMH\0\0?[69LG\0\0.258KFC\0 147JEB\0\0XSNIDA\0\0\0\0\0\0\0\0";
	gfx_Clear(point);
	lib_MemZero(point->label, 20);


	lchars[33] = '0';
	lchars[18] = '3';

	while ((key = os_GetCSC()) != sk_Enter)
	{

		if (key == sk_Del)
		{
			gfx_Clear(point);
			point->label[--i] = '\0';
			gfx_Clear(point);
			gfx_Print(point);
		}

		if (key == 0x11) // todo: remove negative number support in GFX as our triangles can't have signed values
		{
			dbg_sprintf(dbgout, "Negative sign detected\n");
			point->label[i++] = char_Neg;
			gfx_Clear(point);
			gfx_Print(point);
			isNeg = true;
		}

		else if (lchars[key] && i + 1 <= gDigitThreshold)
		{
			point->label[i++] = lchars[key];
		}
		gfx_Print(point);
		gfx_HorizLine(point->point.x, point->point.y + 8, gfx_GetStringWidth(point->label));
	}

	rbuffer = os_StrToReal(point->label, NULL);
	if (isNeg) rbuffer = os_RealNeg(&rbuffer);

	/**/
	lib_StrReplace(point->label, char_Neg, '-');
	dbg_sprintf(dbgout, "[IO In] %s\n", point->label);

	return rbuffer;
}


bool PointEq(superpoint_t a, superpoint_t b)
{
	return a.point.x == b.point.x && a.point.y == b.point.y;
}

void gfx_ClearHighlight(superpoint_t* p)
{
	gfx_Clear(p);
	gfx_PrintColor(p, gfx_black);
}

void gfx_PrintColor(const superpoint_t* p, uint8_t color)
{
	gfx_SetTextFGColor(color);
	gfx_PrintStringXY(p->label, p->point.x, p->point.y);
	gfx_SetTextFGColor(gfx_black);
}

void gfx_HighlightPoint(superpoint_t* p)
{
	gfx_PrintColor(p, gfx_red);
}