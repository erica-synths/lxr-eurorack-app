/*
 * MenuText.h
 *
 * Created: 04.11.2012 08:52:54
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef MENUTEXT_H_
#define MENUTEXT_H_

//-----------------------------------------------------------------
#define MENU_FILTER			1
#define MENU_WAVEFORM		2
#define MENU_AUDIO_OUT		3
#define MENU_ROLL_RATES		4
#define MENU_SYNC_RATES		5
#define MENU_LFO_WAVES		6
#define MENU_RETRIGGER		7
#define MENU_NEXT_PATTERN	8
#define MENU_SEQ_QUANT		9
#define MENU_TRANS			10
#define MENU_MIDI			11
#define MENU_MIDI_ROUTING	12
#define MENU_MIDI_FILTERING 13
#define MENU_PPQ_IN		 	14
#define MENU_MIX_FM		 	15
#define MENU_PPQ_OUT	 	16
#define MENU_FOLLOW		 	17
#define MENU_FX			 	18
#define MENU_DIST		 	19
#define MENU_RATIO		 	20
#define MENU_DELAY		 	21
#define MENU_REVERB		 	22
#define MENU_PAT_MODE	 	23
#define MENU_STEP_FLAM	 	24
#define MENU_STEP_SHIFT	 	25
#define MENU_CV_SRC			26
#define MENU_CV_SAVE		27
#define MENU_AUDIO_OUT_FX	28

//-----------------------------------------------------------------
// Shared texts. Reduce mem usage by pooling common text
// all of these need to be 3 chars
const char menuText_ok[] 	= 	"ok ";
const char menuText_off[] 	= "off";
const char menuText_on[] 	= 	"on ";
const char menuText_mix[] 	= "mix";
const char menuText_fm[] 	= 	"fm ";
const char menuText_dash[] 	="---";
const char menuText_blank[] ="   ";
const char menuText_any[]  	= "any";
//-----------------------------------------------------------------
const char cvSaveNames[][4] =
{
	{2},		//number of entries
	{"kit"},
	{"glo"},


	//{"+8"},

};
//-----------------------------------------------------------------
const char cvSrcNames[][4] =
{
	{17},		//number of entries
	{"cv1"},
	{"cv2"},
	{"cv3"},
	{"cv4"},
	{"cv5"},
	{"lf1"},
	{"lf2"},
	{"lf3"},
	{"lf4"},
	{"lf5"},
	{"lf6"},
	{"ac1"},
	{"ac2"},
	{"ac3"},
	{"ac4"},
	{"ac5"},
	{"ac6"}


	//{"+8"},

};
//-----------------------------------------------------------------
const char stepFlamNames[][4] =
{
	{6},		//number of entries
	{"off"},
	{"x2"},
	{"x3"},
	{"x4"},
	{"x6"},
	{"x8"}

	//{"+8"},

};
//-----------------------------------------------------------------
const char stepShiftNames[][4] =
{
	{15},		//number of entries
	{"-7"},
	{"-6"},
	{"-5"},
	{"-4"},
	{"-3"},
	{"-2"},
	{"-1"},
	{"0"},
	{"+1"},
	{"+2"},
	{"+3"},
	{"+4"},
	{"+5"},
	{"+6"},
	{"+7"},
	//{"+8"},

};
//-----------------------------------------------------------------
const char patModeNames[][4] =
{
	{2},		//number of entries
	{"off"},
	{"Aut"},

};
//-----------------------------------------------------------------
const char reverbNames[][4] =
{
	{1},		//number of entries
	{"Plt"},

};
//-----------------------------------------------------------------
const char delayNames[][4] =
{
	{2},		//number of entries
	{"Del"},
	{"Pp"},

};
//-----------------------------------------------------------------
const char ratioNames[][4] =
{
	{6},		//number of entries
	{"1:1"},
	{"2:1"},
	{"3:1"},
	{"4:1"},
	{"6:1"},
	{"8:1"}
};
//-----------------------------------------------------------------
const char fxNames[][4] =
{
	{5},		//number of entries
	{"off"},
	{"drv"},
	{"rng"},
	{"cmp"},
	{"del"},
	//{"rev"}
};
//-----------------------------------------------------------------
const char distNames[][4] =
{
	{3},		//number of entries
	{"tub"},
	{"fld"},
	{"clp"}
};
//-----------------------------------------------------------------
const char ppqInNames[][4] =
{
	{2},		//number of entries
	{"4"},
	//{"8"},
	//{"16"},
	{"24"}

};
//-----------------------------------------------------------------
const char followNames[][4] =
{
	{4},		//number of entries
	{"off"},
	{"bar"},
	{"pat"},
	{"p+b"}
};
//-----------------------------------------------------------------
const char ppqOutNames[][4] =
{
	{6},		//number of entries
	{"2"},
	{"4"},
	{"8"},
	{"16"},
	{"24"},
	{"48"},
};
//-----------------------------------------------------------------
const char midiModes[][4]  =
{
	{2},		//number of entries
	{"trg"},
	{"nte"},
};
//-----------------------------------------------------------------
const char quantisationNames[][4]  =
{
	{5},		//number of entries
	{"off"},
	{"8"},
	{"16"},
	{"32"},
	{"64"},
};
//-----------------------------------------------------------------
const char transientNames[][4]  =
{
	{14},		//number of entries
	{"Snp"},
	{"Ofs"},
	{"Clk"},
	{"Ck2"},
	{"Tik"},
	{"Kik"},
	{"Rim"},
	{"Drp"},
	{"Hat"},
	{"Clp"},
	{"Kk2"},
	{"Snr"},
	{"Tom"},
	{"Sp2"},
	
};


//-----------------------------------------------------------------
const char nextPatternNames[][4]  =
{
	{15},		//number of entries
	{"p1"},
	{"p2"},
	{"p3"},
	{"p4"},
	{"p5"},
	{"p6"},
	{"p7"},
	{"p8"},
	{"r2"},
	{"r3"},
	{"r4"},
	{"r5"},
	{"r6"},
	{"r7"},
	{"r8"},
};
//-----------------------------------------------------------------
const char retriggerNames[][4]  =
{
	{7},		//number of entries
	{"off"},
	{"v1"},
	{"v2"},
	{"v3"},
	{"v4"},
	{"v5"},
	{"v6"},
};
//-----------------------------------------------------------------
const char lfoWaveNames[][4]  =
{
	{8},		//number of entries
	{"sin"},
	{"tri"},
	{"sup"},
	{"sdn"},
	{"sqr"},
	{"rnd"},
	{"xup"},
	{"xdn"},
};
//-----------------------------------------------------------------
const char rollRateNames[][4]  =
{
	{6},		//number of entries
	{"One"},		
	{"1/1"},		
	{"1/2"},
	{"1/4"},		
	{"1/8"},		
	{"16"},		
	{"24"},		
	{"32"},		
	{"48"},		
	{"64"},		
	{"128"},
	{"1/3"},
	{"1/6"},
	{"12"},
};
//-----------------------------------------------------------------
const char syncRateNames[][4]  =
{
	{12},		//number of entries
	{"off"},
	{"4/1"},
	{"2/1"},
	{"1/1"},
	{"1/2"},
	{"1/3"},
	{"1/4"},
	{"1/6"},
	{"1/8"},
	{"12"},
	{"16"},
	{"32"},
};
//-----------------------------------------------------------------
const char waveformNames[][4]  =
{
	{6},		//number of entries
	{"Sin"}	,
	{"Tri"}	,
	{"Saw"}	,
	{"Rec"}	,
	{"Noi"}	,
	{"PWM"}	,

};
//-----------------------------------------------------------------
//output routing for FX processor, without FX target
const char outputNamesFx[][7]  =
{
	{6},		//number of entries
	{"St1"}	,	//Stereo Out DAC 1
	{"St2"}	,	//Stereo Out DAC 2
	{"L1"}	,	//Mono Out Left DAC 1
	{"R1"}	,	//Mono Out Right DAC 1
	{"L2"}	,	//Mono Out Left DAC 2
	{"R2"}	,	//Mono Out Right DAC 2

};
//-----------------------------------------------------------------
const char outputNames[][7]  =
{
	{7},		//number of entries
	{"St1"}	,	//Stereo Out DAC 1
	{"St2"}	,	//Stereo Out DAC 2
	{"L1"}	,	//Mono Out Left DAC 1
	{"R1"}	,	//Mono Out Right DAC 1
	{"L2"}	,	//Mono Out Left DAC 2
	{"R2"}	,	//Mono Out Right DAC 2
	{"FX"}	,	//to FX Bus
};
//-----------------------------------------------------------------
const char filterTypes[][8]  =
{
	{8},		//number of entries
	{"LP"}	,	//1 - 001
	{"HP"}	,	//2 - 010
	{"BP"}	,	//3 - 011
	{"UBP"}	,	//4 - 100
	{"Nch"}	,	//5 - 101
	{"Pek"}	,	//6 - 110
	{"LP2"}	,	//1 - 001
	{"off"}	,	//7 - 111
	//{"All"}	,	//7 - 111
};

//-----------------------------------------------------------------
const char midiRoutingNames[][6] =
{
		{6},	// number of entries
		{"off"},
		{"U2M"},
		{"M2M"},
		{"A2M"},
		{"M2U"},
		{"M2A"},
};
//-----------------------------------------------------------------
const char mixFmNames[][6] =
{
		{2},	// number of entries
		{"FM"},
		{"Mix"},

};
//-----------------------------------------------------------------
const char midiFilterNames[][16] =
{
	{16},	// number of entries
	{"off"},
	{"N"},
	{"R"},
	{"RN"},
	{"C"},
	{"CN"},
	{"CR"},
	{"CRN"},
	{"P"},
	{"PN"},
	{"PR"},
	{"PRN"},
	{"PC"},
	{"PCN"},
	{"PCR"},
	{"all"},
};
//-----------------------------------------------------------------
// these must correspond with shortNamesEnum
const char shortNames[][4]  =
{
	{""},
	{"coa"},
	{"fin"},
	{"atk"},
	{"dec"},
	{"eg2"},
	{"mod"},
	{"amt"},
	{"frq"},
	{"drv"},
	{"vol"},
	{"pan"},
	{"noi"},
	{"rpt"},
	{"mix"},	
	{"res"},
	{"typ"},
	{"f1"},
	{"f2"},
	{"g1"},
	{"g2"},
	{"wav"},
	{"dst"},
	{"snc"},
	{"rtg"},
	{"ofs"},
	{"voi"},
	{"slp"},
	{"d1"},		
	{"d2"},
	{"eqg"},
	{"eqf"},
#if USE_CODEC_EQ
	{"eq "},
	{"lim"},
	{"rel"},
	{"max"},
	{"min"},
#endif
	{"rol"},
	{"mrp"},
	{"nte"},
	{"prb"},
	{"stp"},
	{"len"},
	{"rot"},
	{"bpm"},
	{"ch"},
	{"out"},
	{"srt"},
	{"nxt"},
	{"mod"},
	{"vel"},
	{"fch"},
	{"flw"},
	{"qnt"},
	{"trk"},
	{"val"},
	{"shu"},
	{"ssv"},
	{"x"},
	{"y"},
	{"flx"},
	{"mid"},
	{"mrt"}, // midi routing
	{"mtx"}, // midi tx
	{"mrx"},  // midi rx
	{"cki"},  // trigger clock in ppq
	{"cko"},  // trigger clock out1 ppq
	{"co2"},  // trigger clock out2 ppq
	{"pcr"}, // pattern change resets bar counter
	{"pwm"},
	{"col"},
	{"d/w"},
	{"rat"},
	{"tre"},
	{"ton"},
	{"gai"},
	{"fbk"},
	{"rng"},
	{"tim"},
	{"pre"},
	{"ref"},
	{"p1"},
	{"p2"},
	{"p3"},
	{"p4"},
	{"kcm"},
	{"flm"},
	{"shf"},
	{"src"},
	{"v/o"},
	{"cv"},
	{"mtg"},

};
//-----------------------------------------------------------------
// These correspond with the catNamesEnum in menu.h
const char catNames[][16] =
{
	{""},
	//01234567
	{"Oscilltr"},
	{"Veloc EG"},
	{"Mod EG"},
	{"PitchMod"},
	{"FM"},
	{"Voice"},
	{"Noise"},
	{"Nois/Osc"},
	{"Filter"},
	{"Mod Osc"},
	{"LFO"},
	{"Transnt"},
	{"EQ"},
#if USE_CODEC_EQ		
	{"Limiter"},
#endif
	{"Pattern"},
	{"Sound"},
	{"Step"},
	{"Euklid"},		
	{"Global"},	
	{"Accent"},
	{"Parametr"},
	{"Sequencr"},
	{"Generatr"},
	{"MIDI"},
	{"Clock"},
	{"Fx"},
	{"Mod"},
};
//-----------------------------------------------------------------
// these must correspond to longNamesEnum
const char longNames[][16] =
{
	{""},
	//01234567
	{"Coarse"},
	{"Fine"},
	{"Attack"},
	{"Decay"},
	{"Amount"},
	{"Frequncy"},
	{"Overdriv"},
	{"Volume"},
	{"Panning"},
	{"Mix"},
	{"RepeatCt"},
	{"Resnance"},
	{"Type"},
	{"Gain"},
	{"Waveform"},
	{"DstParam"},
	{"ClockSnc"},
	{"Retriggr"},
	{"Offset"},		
	{"DstVoice"},
	{"Slope"},
	{"Dcy Clsd"},
	{"Dcy Open"},
#if USE_CODEC_EQ		
	{"Enable"},
	{"Release"},
	{"Threshold max"},
	{"Threshold min"},
#endif		
	{"RollRate"},
	{"Morph"},
	{"Note"},
	{"Prbablty"},
	{"Number"},
	{"Length"},
	{"Steps"},
	{"Rotation"},
	{"Tempo"},
	{"AudioOut"},
	{"Channel"},
	{"SampleRt"},
	{"NextPatt"},
	{"Phase"},
	{"Mode"},
	{"Vol mod"},
	{"Fetch"},	
	{"Follow"},	
	{"Quantize"},
	{"AutTrack"},
	{"Aut Dest"},
	{"AutValue"},
	{"Shuffle"},
	{"Screensv"},
	{"X Positn"},
	{"Y Positn"},
	{"Flux"},
	{"Velocity"},
	{"Freqcy 1"},
	{"Freqcy 2"},
	{"Gain 1"},
	{"Gain 2"},
	{"Routing"}, // midi routing
	{"Transmit"}, // midi tx filtering
	{"Receive"}, // midi rx filtering
	{"In PPQ"},
	{"Out1 PPQ"},
	{"Out2 PPQ"},
	{"GateMode"},
	{"PCReset" }, // reset bar counter on manual pattern change
	{"PulsWdth"},
	{"Color"},
	{"Dry/Wet"},
	{"Ratio"},
	{"Threshld"},
	{"Tone"},
	{"Feedback"},
	{"Range"},
	{"Time"},
	{"PreDelay"},
	{"Reflxion"},
	{"Param 1"},
	{"Param 2"},
	{"Param 3"},
	{"Param 4"},
	{"KitChMde"},
	{"Flam"},
	{"Shift"},
	{"Source"},
	{"1V/Oct"},
	{"CV Save"},
	{"MrphTrgt"},
};


#endif /* MENUTEXT_H_ */
