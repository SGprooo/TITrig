//
// Created by Decimation on 4/27/2018.
//

#include "Triangle.h"
#include "debug.h"
#include "Trigonometry.h"
#include "DynamicTriangle/DynamicTriangle.h"

static labelpoint_t g_nXAngles[3] = {
		{{30,       119 - 10}, "A"},   // AAA
		{{10 + 155, 119 - 10}, "B"},   // BBB
		{{135,      27},       "C"}   // CCC
};

static labelpoint_t g_nXSides[3] = {
		{{10 + 180, 60},  "a"},  // aaa
		{{60,       55},  "b"},  // bbb
		{{140 - 45, 130}, "c"}   // ccc
};


#define n_angle_A g_nXAngles[0]
#define n_angle_B g_nXAngles[1]
#define n_angle_C g_nXAngles[2]

#define n_side_a g_nXSides[0]
#define n_side_b g_nXSides[1]
#define n_side_c g_nXSides[2]

static const char lbl_AngleMode[] = "ANGLE MODE";
static const char lbl_SideMode[]  = "SIDE MODE";
static const char lbl_SSS[]       = "SSS";
static const char lbl_SSA[]       = "SSA (!)";
static const char lbl_SAS[]       = "SAS";
static const char lbl_AAS[]       = "AAS";
static const char lbl_ASA[]       = "ASA";
static const char lbl_Unknown[]   = "?";

static triangle_t   triangle;
static trigstatus_t trigstatus;

/**
 * When true : display area and perimeter
 * When false: display a, b, c, A, B, C
 */
static bool ui_bDispMeasurements;

static labelpoint_t       ui_Mode      = {{230, 10}, "ANGLE MODE"};
static labelpoint_t       ui_Type      = {{230, 20}, "..."};
static const labelpoint_t ui_btn_Mode  = {280, 230, "Mode"};
static const labelpoint_t ui_btn_Clear = {215, 230, "Clear"};
static const labelpoint_t ui_btn_Data  = {145, 230, "Data"};
static const labelpoint_t ui_btn_Pref  = {70, 230, "Pref"};

static labelpoint_t xmeasureData[2] = {
		{10, 155, "A = "},
		{10, 175, "P = "},
};

static labelpoint_t xanglesData[3] = {
		{10, 155, "A = "},
		{10, 175, "B = "},
		{10, 195, "C = "}
};

static labelpoint_t xsidesData[3] = {
		{130, 155, "a = "},
		{130, 175, "b = "},
		{130, 195, "c = "}
};

static labelpoint_t data_X_ex = {10, 215, "*X = "};

static void trig_Redraw()
{
	int index = 0;
	for (index = 0; index < 3; index++) {
		lp_Clear(&g_nXAngles[index]);
		lp_Clear(&g_nXSides[index]);

		lp_Print(&g_nXAngles[index]);
		lp_Print(&g_nXSides[index]);

		dbg_sprintf(dbgout, "[%s, %s]\n", g_nXAngles[index].label, g_nXSides[index].label);
	}
}

static void dbg_printTriangle()
{
	char buf[20];
	os_RealToStr(buf, &triangle.A, 0, 0, -1);
	dbg_sprintf(dbgout, "Angle A: %s | ", buf);
	os_RealToStr(buf, &triangle.a, 0, 0, -1);
	dbg_sprintf(dbgout, "Side a: %s\n", buf);

	os_RealToStr(buf, &triangle.B, 0, 0, -1);
	dbg_sprintf(dbgout, "Angle B: %s | ", buf);
	os_RealToStr(buf, &triangle.b, 0, 0, -1);
	dbg_sprintf(dbgout, "Side b: %s\n", buf);

	os_RealToStr(buf, &triangle.C, 0, 0, -1);
	dbg_sprintf(dbgout, "Angle C: %s | ", buf);
	os_RealToStr(buf, &triangle.c, 0, 0, -1);
	dbg_sprintf(dbgout, "Side c: %s\n", buf);

	dbg_sprintf(dbgout, "Angle availability: [%s, %s, %s]\n", trigstatus.A ? "1" : "0", trigstatus.B ? "1" : "0",
				trigstatus.C ? "1" : "0");
	dbg_sprintf(dbgout, "Side availability: [%s, %s, %s]\n", trigstatus.a ? "1" : "0", trigstatus.b ? "1" : "0",
				trigstatus.c ? "1" : "0");

	os_RealToStr(buf, &triangle.area, 0, 0, -1);
	dbg_sprintf(dbgout, "Area: %s\n", buf);
	os_RealToStr(buf, &triangle.perimeter, 0, 0, -1);
	dbg_sprintf(dbgout, "Perimeter: %s\n", buf);
}

static void trig_Sync()
{
	dbg_sprintf(dbgout, "[Trig] Synchronizing...\n");
	__tri_Round(&triangle, g_round);
	dbg_sprintf(dbgout, "[Trig] Rounded\n");

	if (trigstatus.A) {
		os_RealToStr(n_angle_A.label, &triangle.A, 0, 0, 6);
	}
	if (trigstatus.B) {
		os_RealToStr(n_angle_B.label, &triangle.B, 0, 0, 6);
	}
	if (trigstatus.C) {
		os_RealToStr(n_angle_C.label, &triangle.C, 0, 0, 6);
	}
	if (trigstatus.a) {
		os_RealToStr(n_side_a.label, &triangle.a, 0, 0, 6);
	}
	if (trigstatus.b) {
		os_RealToStr(n_side_b.label, &triangle.b, 0, 0, 6);
	}
	if (trigstatus.c) {
		os_RealToStr(n_side_c.label, &triangle.c, 0, 0, 6);
	}

	ui_DispData();
	trig_TruncateLabels(g_digitThreshold);
	dbg_sprintf(dbgout, "[Trig] Truncated labels\n");
	trig_Redraw();
	dbg_printTriangle();
}

real_t trig_AreaBySin(real_t a, real_t b, real_t C)
{
	//Area =  1/2 ab sin C

	const real_t onehalf = os_FloatToReal(0.5f);
	real_t       buf;
	C   = os_RealSinDeg(C);
	buf = os_RealMul(&onehalf, &a);
	buf = os_RealMul(&buf, &b);
	return os_RealMul(&buf, &C);
}

static void trig_HeronsFormula()
{
	real_t       num, s, root;
	real_t       sma, smb, smc;
	const real_t real2 = os_Int24ToReal(2);

	num = os_RealAdd(&triangle.a, &triangle.b);
	num = os_RealAdd(&num, &triangle.c);

	// Half of perimeter
	s = os_RealDiv(&num, &real2);

	triangle.perimeter = os_RealMul(&s, &real2);
	dbg_sprintf(dbgout, "[Trig] Solved for perimeter\n");

	sma = os_RealSub(&s, &triangle.a);
	smb = os_RealSub(&s, &triangle.b);
	smc = os_RealSub(&s, &triangle.c);

	sma = os_RealMul(&s, &sma);

	root = os_RealMul(&sma, &smb);
	root = os_RealMul(&root, &smc);

	triangle.area = os_RealSqrt(&root);
	dbg_sprintf(dbgout, "[Trig] Solved for area\n");
}

static void trig_SolveSSS()
{
	real_t       buf;
	const real_t real180 = os_Int24ToReal(180);
	dbg_sprintf(dbgout, "[Trig] Solving SSS triangle\n");
	triangle.A   = loc_Angle_x(triangle.b, triangle.c, triangle.a);
	trigstatus.A = true;
	dbg_sprintf(dbgout, "[Trig] Solved for angle A\n");

	triangle.B   = loc_Angle_x(triangle.c, triangle.a, triangle.b);
	trigstatus.B = true;
	dbg_sprintf(dbgout, "[Trig] Solved for angle B\n");

	buf = os_RealAdd(&triangle.A, &triangle.B);
	triangle.C   = os_RealSub(&real180, &buf);
	trigstatus.C = true;
	dbg_sprintf(dbgout, "[Trig] Solved for angle C\n");

	trig_HeronsFormula();
	trig_Sync();
}

static void trig_SolveMissingAngle()
{
	const real_t real180 = os_Int24ToReal(180);
	real_t       buf;

	if (trigstatus.A && trigstatus.B) {
		buf = os_RealAdd(&triangle.A, &triangle.B);
		triangle.C   = os_RealSub(&real180, &buf);
		trigstatus.C = true;
	}
	else if (trigstatus.A && trigstatus.C) {
		buf = os_RealAdd(&triangle.A, &triangle.C);
		triangle.B   = os_RealSub(&real180, &buf);
		trigstatus.B = true;
	}
	else if (trigstatus.B && trigstatus.C) {
		buf = os_RealAdd(&triangle.B, &triangle.C);
		triangle.A   = os_RealSub(&real180, &buf);
		trigstatus.A = true;
	}
}

static void trig_TruncateLabels(int len)
{
	int i = 0;
	for (; i < 3; i++) {
		sys_StrCut(g_nXAngles[i].label, len, 20 - len);
		sys_StrCut(g_nXSides[i].label, len, 20 - len);
	}
}

static void trig_CheckSolvability()
{
	real_t       rbuf;
	char         cbuf[20];
	char         cbuf2[20];
	const real_t real180 = os_Int24ToReal(180);

	dbg_sprintf(dbgout, "[Trig] Checking solvability...\n");
	ui_DispData();

	// Automatically solve for the third angle when possible
	trig_SolveMissingAngle();
	trig_Sync();


	if (trigstatus.complete) {
		return;
	}


	// SSS
	if (trigstatus.a && trigstatus.b && trigstatus.c) {
		dbg_sprintf(dbgout, "SSS detected [%s, %s, %s]\n", n_side_a.label, n_side_b.label, n_side_c.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SSS);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveSSS();
		trigstatus.complete = true;
		return;
	}

	// AAS
	// "AAS" is when we know two angles and one side (which is not between the angles).
	if (trigstatus.A && trigstatus.C && trigstatus.c) {
		dbg_sprintf(dbgout, "AAS_1 detected [%s, %s, %s]\n", n_angle_A.label, n_angle_C.label, n_side_c.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_AAS);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveMissingAngle();
		triangle.a   = los_Side_x(triangle.A, triangle.c, triangle.C);
		trigstatus.a = true;
		triangle.b   = los_Side_x(triangle.B, triangle.a, triangle.A);
		trigstatus.b = true;

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.B && trigstatus.C && trigstatus.b) {
		dbg_sprintf(dbgout, "AAS_2 detected [%s, %s, %s]\n", n_angle_B.label, n_angle_C.label, n_side_b.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_AAS);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveMissingAngle();

		triangle.c   = los_Side_x(triangle.C, triangle.b, triangle.B);
		trigstatus.c = true;
		triangle.a   = los_Side_x(triangle.A, triangle.c, triangle.C);
		trigstatus.a = true;

		//dbg_printTriangle();
		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.A && trigstatus.B && trigstatus.b) {
		dbg_sprintf(dbgout, "AAS_3 detected [%s, %s, %s]\n", n_angle_A.label, n_angle_B.label, n_side_b.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_AAS);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveMissingAngle();
		triangle.c   = los_Side_x(triangle.C, triangle.b, triangle.B);
		trigstatus.c = true;
		triangle.a   = los_Side_x(triangle.A, triangle.b, triangle.B);
		trigstatus.a = true;

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}

	// ASA
	// "ASA" is when we know two angles and a side between the angles.
	if (trigstatus.A && trigstatus.c && trigstatus.B) {
		dbg_sprintf(dbgout, "ASA_1 detected [%s, %s, %s]\n", n_angle_A.label, n_side_c.label, n_angle_B.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_ASA);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveMissingAngle();
		triangle.a   = los_Side_x(triangle.A, triangle.c, triangle.C);
		trigstatus.a = true;
		triangle.b   = los_Side_x(triangle.B, triangle.c, triangle.C);
		trigstatus.b = true;

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.A && trigstatus.b && trigstatus.C) //todo: verify
	{
		dbg_sprintf(dbgout, "ASA_2 detected [%s, %s, %s]\n", n_angle_A.label, n_side_b.label, n_angle_C.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_ASA);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveMissingAngle();
		triangle.a   = los_Side_x(triangle.A, triangle.b, triangle.B);
		trigstatus.a = true;
		triangle.c   = los_Side_x(triangle.C, triangle.b, triangle.B);
		trigstatus.c = true;

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.B && trigstatus.a && trigstatus.C) //todo: verify
	{
		dbg_sprintf(dbgout, "ASA_3 detected [%s, %s, %s]\n", n_angle_B.label, n_side_a.label, n_angle_C.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_ASA);
		lp_PrintColor(&ui_Type, gfx_green);

		trig_SolveMissingAngle();
		triangle.b   = los_Side_x(triangle.B, triangle.a, triangle.A);
		trigstatus.b = true;
		triangle.c   = los_Side_x(triangle.C, triangle.a, triangle.A);
		trigstatus.c = true;

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}

	// SAS
	// "SAS" is when we know two sides and the angle between them.
	if (trigstatus.b && trigstatus.A && trigstatus.c) {
		dbg_sprintf(dbgout, "SAS_1 detected [%s, %s, %s]\n", n_side_b.label, n_angle_A.label, n_side_c.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SAS);
		lp_PrintColor(&ui_Type, gfx_green);

		triangle.a   = loc_Side_x(triangle.b, triangle.c, triangle.A);
		trigstatus.a = true;

		// todo: need to find the smaller of the two angles
		triangle.B   = los_Angle_x(triangle.b, triangle.A, triangle.a);
		trigstatus.B = true;

		trig_SolveMissingAngle();

		trigstatus.complete = true;
		trig_Sync();
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.c && trigstatus.B && trigstatus.a) //todo: verify
	{
		dbg_sprintf(dbgout, "SAS_2 detected [%s, %s, %s]\n", n_side_c.label, n_angle_B.label, n_side_a.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SAS);
		lp_PrintColor(&ui_Type, gfx_green);

		triangle.b   = loc_Side_x(triangle.a, triangle.c, triangle.B);
		trigstatus.b = true;

		// todo: need to find the smaller of the two angles
		triangle.A   = los_Angle_x(triangle.a, triangle.B, triangle.b);
		trigstatus.A = true;

		trig_SolveMissingAngle();
		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.b && trigstatus.C && trigstatus.a) {
		dbg_sprintf(dbgout, "SAS_3 detected [%s, %s, %s]\n", n_side_b.label, n_angle_C.label, n_side_a.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SAS);
		lp_PrintColor(&ui_Type, gfx_green);

		triangle.c   = loc_Side_x(triangle.a, triangle.b, triangle.C);
		trigstatus.c = true;

		// todo: need to find the smaller of the two angles
		triangle.A   = los_Angle_x(triangle.a, triangle.C, triangle.c);
		trigstatus.A = true;

		trig_SolveMissingAngle();
		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}

	// SSA
	// "SSA" is when we know two sides and an angle that is not the angle between the sides.
	// todo: check if it has multiple answers
	if (trigstatus.b && trigstatus.c && trigstatus.B) {
		dbg_sprintf(dbgout, "SSA_1 detected [%s, %s, %s]\n", n_side_b.label, n_side_c.label, n_angle_B.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SSA);
		lp_PrintColor(&ui_Type, gfx_green);
		trigstatus.isSSA = true;

		triangle.C   = los_Angle_x(triangle.c, triangle.B, triangle.b);
		trigstatus.C = true;

		trig_SolveMissingAngle();

		triangle.a   = los_Side_x(triangle.A, triangle.c, triangle.C);
		trigstatus.a = true;

		// other possible answer for C
		rbuf = os_RealSub(&real180, &triangle.C);
		os_RealToStr(cbuf, &rbuf, 0, 0, 6);

		sprintf(cbuf2, "*C = %s", cbuf);
		lp_SetLabel(&data_X_ex, cbuf2);
		lp_Clear(&data_X_ex);
		lp_Print(&data_X_ex);
		dbg_sprintf(dbgout, "data_X_ex = %s\n", data_X_ex.label);
		dbg_sprintf(dbgout, "Other possible solution for C = %s\n", cbuf);

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.b && trigstatus.a && trigstatus.B) //todo: verify
	{
		dbg_sprintf(dbgout, "SSA_2 detected [%s, %s, %s]\n", n_side_b.label, n_side_a.label, n_angle_B.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SSA);
		lp_PrintColor(&ui_Type, gfx_green);
		trigstatus.isSSA = true;

		triangle.C   = los_Angle_x(triangle.c, triangle.B, triangle.b);
		trigstatus.C = true;

		trig_SolveMissingAngle();

		triangle.c   = los_Side_x(triangle.C, triangle.b, triangle.B);
		trigstatus.c = true;

		// other possible answer for C
		rbuf = os_RealSub(&real180, &triangle.C);
		os_RealToStr(cbuf, &rbuf, 0, 0, 6);

		sprintf(cbuf2, "*C = %s", cbuf);
		lp_SetLabel(&data_X_ex, cbuf2);
		lp_Clear(&data_X_ex);
		lp_Print(&data_X_ex);
		dbg_sprintf(dbgout, "data_X_ex = %s\n", data_X_ex.label);
		dbg_sprintf(dbgout, "Other possible solution for C = %s\n", cbuf);

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
	else if (trigstatus.a && trigstatus.c && trigstatus.C) //todo: verify
	{
		dbg_sprintf(dbgout, "SSA_3 detected [%s, %s, %s]\n", n_side_a.label, n_side_c.label, n_angle_C.label);
		lp_Clear(&ui_Type);
		lp_SetLabel(&ui_Type, lbl_SSA);
		lp_PrintColor(&ui_Type, gfx_green);
		trigstatus.isSSA = true;

		triangle.B   = los_Angle_x(triangle.a, triangle.C, triangle.c);
		trigstatus.B = true;

		trig_SolveMissingAngle();

		triangle.b   = los_Side_x(triangle.B, triangle.c, triangle.C);
		trigstatus.b = true;

		// other possible answer for B
		rbuf = os_RealSub(&real180, &triangle.B);
		os_RealToStr(cbuf, &rbuf, 0, 0, 6);

		sprintf(cbuf2, "*B = %s", cbuf);
		lp_SetLabel(&data_X_ex, cbuf2);
		lp_Clear(&data_X_ex);
		lp_Print(&data_X_ex);
		dbg_sprintf(dbgout, "data_X_ex = %s\n", data_X_ex.label);
		dbg_sprintf(dbgout, "Other possible solution for B = %s\n", cbuf);

		trig_Sync();
		trigstatus.complete = true;
		trig_HeronsFormula();
		ui_DispData();
		return;
	}
}

static void ui_ClearMeasurements()
{
	int i = 0;
	for (; i < 2; i++) {
		lp_Clear(&xmeasureData[i]);
	}
	dbg_sprintf(dbgout, "[UI] Cleared measurements\n");
}

static void ui_ClearAngleSideData()
{
	int i = 0;
	for (; i < 3; i++) {
		lp_Clear(&xsidesData[i]);
		lp_Clear(&xanglesData[i]);
	}
	dbg_sprintf(dbgout, "[UI] Cleared angle and side data\n");
}

static void trig_SelectSide()
{
	uint8_t key;
	labelpoint_t* currentSelection = &n_side_b; // start at b
	lp_HighlightPoint(&n_side_b);
	RECURSE:
	while ((key = os_GetCSC()) != sk_Enter) {
		if (key == sk_Clear) {
			return;
		}

		if (key == sk_Zoom) {
			if (ui_bDispMeasurements) {
				ui_bDispMeasurements = false;
				ui_ClearMeasurements();
			}
			else {
				ui_bDispMeasurements = true;
				ui_ClearAngleSideData();
			}
			ui_DispData();
		}

		if (key == sk_Trace) {
			trig_Reset();
			currentSelection = &n_side_b;
			lp_HighlightPoint(&n_side_b);
		}

		if (key == sk_Graph) {
			lp_Clear(&ui_Mode);
			lp_SetLabel(&ui_Mode, lbl_AngleMode);
			lp_Print(&ui_Mode);
			lp_ClearHighlight(currentSelection);
			trig_SelectAngle();
			return;
		}

		/* bbb -> aaa */
		if (key == sk_Right && lp_Equals(*currentSelection, n_side_b)) {
			lp_SetFocus(&currentSelection, &n_side_b, &n_side_a);
		}

		/* bbb -> ccc */
		if (key == sk_Down && lp_Equals(*currentSelection, n_side_b)) {
			lp_SetFocus(&currentSelection, &n_side_b, &n_side_c);
		}

		/* ccc -> bbb */
		if ((key == sk_Up || key == sk_Left) && lp_Equals(*currentSelection, n_side_c)) {
			lp_SetFocus(&currentSelection, &n_side_c, &n_side_b);
		}

		/* ccc -> aaa */
		if (key == sk_Right && lp_Equals(*currentSelection, n_side_c)) {
			lp_SetFocus(&currentSelection, &n_side_c, &n_side_a);
		}

		/* aaa -> bbb */
		if (key == sk_Left && lp_Equals(*currentSelection, n_side_a)) {
			lp_SetFocus(&currentSelection, &n_side_a, &n_side_b);
		}

		/* aaa -> ccc */
		if (key == sk_Down && lp_Equals(*currentSelection, n_side_a)) {
			lp_SetFocus(&currentSelection, &n_side_a, &n_side_c);
		}
	}

	if (lp_Equals(*currentSelection, n_side_a)) {
		dbg_sprintf(dbgout, "[Trig] User selected side %s\n", n_side_a.label);
		triangle.a   = io_gfx_ReadReal(&n_side_a);
		trigstatus.a = true;
	}

	if (lp_Equals(*currentSelection, n_side_b)) {
		dbg_sprintf(dbgout, "[Trig] User selected side %s\n", n_side_b.label);
		triangle.b   = io_gfx_ReadReal(&n_side_b);
		trigstatus.b = true;
	}

	if (lp_Equals(*currentSelection, n_side_c)) {
		dbg_sprintf(dbgout, "[Trig] User selected side %s\n", n_side_c.label);
		triangle.c   = io_gfx_ReadReal(&n_side_c);
		trigstatus.c = true;
	}

	trig_CheckSolvability();
	lp_HighlightPoint(currentSelection);
	goto RECURSE;

}

static void trig_Quit()
{
	gfx_ZeroScreen();
	gfx_End();
	dbg_sprintf(dbgout, "[Trig] Zeroed screen and ended GFX\n");
}

static void trig_Reset()
{
	int i = 0;
	bool  * bptr = (bool*) &trigstatus;
	real_t* rptr = (real_t*) &triangle;
	for (; i < sizeof(trigstatus) / sizeof(bool); i++) {
		bptr[i] = false;
	}
	for (i = 0; i < sizeof(triangle) / sizeof(real_t); i++) {
		rptr[i] = os_Int24ToReal(0);
	}
	for (i = 0; i < 3; i++) {
		lp_Clear(&xanglesData[i]);
		sys_MemZero(xanglesData[i].label + kLabelOffset, 16);

		lp_Clear(&xsidesData[i]);
		sys_MemZero(xsidesData[i].label + kLabelOffset, 16);
	}
	lp_Clear(&ui_Type);
	lp_Clear(&data_X_ex);
	sys_MemZero(data_X_ex.label + kLabelOffset, 16);

	lp_Clear(&xmeasureData[0]);
	lp_Clear(&xmeasureData[1]);
	sys_MemZero(xmeasureData[0].label + kLabelOffset, 20 - kLabelOffset);
	sys_MemZero(xmeasureData[1].label + kLabelOffset, 20 - kLabelOffset);

	lp_SetLabel(&n_angle_A, "A");
	lp_SetLabel(&n_angle_B, "B");
	lp_SetLabel(&n_angle_C, "C");
	lp_SetLabel(&n_side_a, "a");
	lp_SetLabel(&n_side_b, "b");
	lp_SetLabel(&n_side_c, "c");
	trig_Redraw();
	trig_DrawTriangleSides();
	trig_Sync();
}

static void trig_DrawTriangleSides()
{
	int index = 0;

	/* Triangle coordinates */
	const int verts[6] = {
			140, 10,   /* (x0, y0) */
			10, 119,  /* (x1, y1) */
			209, 119, /* (x2, y2) */
	};

	/* Leg a */
	gfx_Line(verts[0], verts[1], verts[2], verts[3]);

	/* Leg b */
	gfx_Line(verts[2], verts[3], verts[4], verts[5]);

	/* Hypotenuse */
	gfx_Line(verts[4], verts[5], verts[0], verts[1]);


	// Angles A, B, C
	trig_Redraw();

	// Leg a, b, c
	for (index = 0; index < 3; index++) {
		lp_Print(&g_nXSides[index]);
	}
}

static void ui_DispData()
{
	if (ui_bDispMeasurements) {
		ui_DispMeasurements();
	}
	else if (!ui_bDispMeasurements) {
		ui_DispAngleSideData();
	}
}

static void trig_SelectAngle()
{
	uint8_t key;
	labelpoint_t* currentSelection = &n_angle_A;
	lp_HighlightPoint(&n_angle_A);
	RECURSE:
	while ((key = os_GetCSC()) != sk_Enter) {
		if (key == sk_Clear) {
			return;
		}

		if (key == sk_Zoom) {
			if (ui_bDispMeasurements) {
				ui_bDispMeasurements = false;
				ui_ClearMeasurements();
			}
			else {
				ui_bDispMeasurements = true;
				ui_ClearAngleSideData();
			}
			ui_DispData();
		}

		if (key == sk_Trace) {
			trig_Reset();
			currentSelection = &n_angle_A;
			lp_HighlightPoint(&n_angle_A);
		}

		if (key == sk_Graph) {
			lp_Clear(&ui_Mode);
			lp_SetLabel(&ui_Mode, lbl_SideMode);
			lp_Print(&ui_Mode);
			lp_ClearHighlight(currentSelection);
			trig_SelectSide();
			return;
		}

		/* AAA -> BBB */
		if (key == sk_Right && lp_Equals(*currentSelection, n_angle_A)) {
			lp_SetFocus(&currentSelection, &n_angle_A, &n_angle_B);
		}

		/* AAA -> CCC */
		if (key == sk_Up && lp_Equals(*currentSelection, n_angle_A)) {
			lp_SetFocus(&currentSelection, &n_angle_A, &n_angle_C);
		}

		/* BBB -> AAA */
		if (key == sk_Left && lp_Equals(*currentSelection, n_angle_B)) {
			lp_SetFocus(&currentSelection, &n_angle_B, &n_angle_A);
		}

		/* BBB -> CCC */
		if (key == sk_Up && lp_Equals(*currentSelection, n_angle_B)) {
			lp_SetFocus(&currentSelection, &n_angle_B, &n_angle_C);
		}

		/* CCC -> AAA */
		if (key == sk_Left && lp_Equals(*currentSelection, n_angle_C)) {
			lp_SetFocus(&currentSelection, &n_angle_C, &n_angle_A);
		}

		/* CCC -> BBB */
		if (key == sk_Down && lp_Equals(*currentSelection, n_angle_C)) {
			lp_SetFocus(&currentSelection, &n_angle_C, &n_angle_B);
		}
	}

	if (lp_Equals(*currentSelection, n_angle_A)) {
		dbg_sprintf(dbgout, "[Trig] User selected angle %s\n", n_angle_A.label);
		triangle.A   = io_gfx_ReadReal(&n_angle_A);
		trigstatus.A = true;
	}

	if (lp_Equals(*currentSelection, n_angle_B)) {
		dbg_sprintf(dbgout, "[Trig] User selected angle %s\n", n_angle_B.label);
		triangle.B   = io_gfx_ReadReal(&n_angle_B);
		trigstatus.B = true;
	}

	if (lp_Equals(*currentSelection, n_angle_C)) {
		dbg_sprintf(dbgout, "[Trig] User selected angle %s\n", n_angle_C.label);
		triangle.C   = io_gfx_ReadReal(&n_angle_C);
		trigstatus.C = true;
	}

	trig_CheckSolvability();
	lp_HighlightPoint(currentSelection);
	goto RECURSE;
}

static void ui_DispMeasurements()
{
	char         buffer[20];
	const real_t real0 = os_Int24ToReal(0);
	int          i     = 0;

	for (; i < 2; i++) {
		lp_Clear(&xmeasureData[i]);
	}

	if (os_RealCompare(&triangle.area, &real0) == 0) {
		strcpy(xmeasureData[0].label + kLabelOffset, lbl_Unknown);
	}
	else {
		os_RealToStr(buffer, &triangle.area, 0, 0, 6);
		strncpy(xmeasureData[0].label + kLabelOffset, buffer, 20 - kLabelOffset);
	}

	if (os_RealCompare(&triangle.perimeter, &real0) == 0) {
		strcpy(xmeasureData[1].label + kLabelOffset, lbl_Unknown);
	}
	else {
		os_RealToStr(buffer, &triangle.perimeter, 0, 0, 6);
		strncpy(xmeasureData[1].label + kLabelOffset, buffer, 20 - kLabelOffset);
	}


	for (i = 0; i < 2; i++) {
		lp_Clear(&xmeasureData[i]);
		lp_Print(&xmeasureData[i]);
	}
}

static void ui_DispAngleSideData()
{
	int i = 0;
	for (; i < 3; i++) {
		lp_Clear(&xsidesData[i]);
		lp_Clear(&xanglesData[i]);

		if (*g_nXSides[i].label == 'a' || *g_nXSides[i].label == 'b' || *g_nXSides[i].label == 'c')
			strcpy(xsidesData[i].label + kLabelOffset, lbl_Unknown);
		else strcpy(xsidesData[i].label + kLabelOffset, g_nXSides[i].label);

		if (*g_nXAngles[i].label == 'A' || *g_nXAngles[i].label == 'B' || *g_nXAngles[i].label == 'C')
			strcpy(xanglesData[i].label + kLabelOffset, lbl_Unknown);
		else strcpy(xanglesData[i].label + kLabelOffset, g_nXAngles[i].label);

		lp_Clear(&xanglesData[i]);
		lp_Print(&xanglesData[i]);

		lp_Clear(&xsidesData[i]);
		lp_Print(&xsidesData[i]);
	}
	if (trigstatus.isSSA) {
		lp_Clear(&data_X_ex);
		lp_Print(&data_X_ex);
	}
	else if (!trigstatus.isSSA) {
		lp_Clear(&data_X_ex);
	}
}

void trig_SolveTriangle()
{
	os_ClrHome();

	ui_bDispMeasurements = false;

	gfx_Begin();
	gfx_SetColor(gfx_blue);
	gfx_SetTextFGColor(gfx_black);

	lp_Print(&ui_Mode);
	lp_Print(&ui_btn_Mode);
	lp_Print(&ui_btn_Clear);
	lp_Print(&ui_btn_Data);

	trig_Reset();

	trig_SelectAngle();

	trig_Quit();
}
