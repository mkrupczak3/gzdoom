/*
** music_midi_base.cpp
**
**---------------------------------------------------------------------------
** Copyright 1998-2010 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "i_midi_win32.h"


#include "i_musicinterns.h"
#include "c_dispatch.h"
#include "i_music.h"
#include "i_system.h"
#include "gameconfigfile.h"
#include "cmdlib.h"
#include "m_misc.h"
#include "s_sound.h"

#include "templates.h"
#include "v_text.h"
#include "menu/menu.h"

static uint32_t	nummididevices;
static bool		nummididevicesset;

#ifdef HAVE_FLUIDSYNTH
#define NUM_DEF_DEVICES 5
#else
#define NUM_DEF_DEVICES 4
#endif

static void AddDefaultMidiDevices(FOptionValues *opt)
{
	int p;
	FOptionValues::Pair *pair = &opt->mValues[opt->mValues.Reserve(NUM_DEF_DEVICES)];
#ifdef HAVE_FLUIDSYNTH
	pair[0].Text = "FluidSynth";
	pair[0].Value = -5.0;
	p = 1;
#else
	p = 0;
#endif
	pair[p].Text = "GUS";
	pair[p].Value = -4.0;
	pair[p+1].Text = "OPL Synth Emulation";
	pair[p+1].Value = -3.0;
	pair[p+2].Text = "TiMidity++";
	pair[p+2].Value = -2.0;
	pair[p+3].Text = "WildMidi";
	pair[p+3].Value = -6.0;

}

extern MusPlayingInfo mus_playing;

void MIDIDeviceChanged(int newdev, bool force)
{
	static int oldmididev = INT_MIN;

	// If a song is playing, move it to the new device.
	if (oldmididev != newdev || force)
	{
		if (currSong != NULL && currSong->IsMIDI())
		{
			MusInfo *song = currSong;
			if (song->m_Status == MusInfo::STATE_Playing)
			{
				if (song->GetDeviceType() == MDEV_FLUIDSYNTH && force)
				{
					// FluidSynth must reload the song to change the patch set.
					auto mi = mus_playing;
					S_StopMusic(true);
					S_ChangeMusic(mi.name, mi.baseorder, mi.loop);
				}
				else
				{
					song->Stop();
					song->Start(song->m_Looping);
				}
			}
		}
		else
		{
			S_MIDIDeviceChanged();
		}
	}
	// 'force' 
	if (!force) oldmididev = newdev;
}

#ifdef HAVE_FLUIDSYNTH 
#define DEF_MIDIDEV -5
#else
#define DEF_MIDIDEV -3
#endif

#ifdef _WIN32
unsigned mididevice;

CUSTOM_CVAR (Int, snd_mididevice, DEF_MIDIDEV, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
{
	if (!nummididevicesset)
		return;

	if ((self >= (signed)nummididevices) || (self < -6))
	{
		// Don't do repeated message spam if there is no valid device.
		if (self != 0)
		{
			Printf("ID out of range. Using default device.\n");
			self = DEF_MIDIDEV;
		}
		return;
	}
	else if (self == -1) self = DEF_MIDIDEV;
	mididevice = MAX<UINT>(0, self);
	MIDIDeviceChanged(self);
}

void I_InitMusicWin32 ()
{
	nummididevices = midiOutGetNumDevs ();
	nummididevicesset = true;
	snd_mididevice.Callback ();
}

void I_BuildMIDIMenuList (FOptionValues *opt)
{
	AddDefaultMidiDevices(opt);

	for (uint32_t id = 0; id < nummididevices; ++id)
	{
		MIDIOUTCAPS caps;
		MMRESULT res;

		res = midiOutGetDevCaps (id, &caps, sizeof(caps));
		assert(res == MMSYSERR_NOERROR);
		if (res == MMSYSERR_NOERROR)
		{
			FOptionValues::Pair *pair = &opt->mValues[opt->mValues.Reserve(1)];
			pair->Text = caps.szPname;
			pair->Value = (float)id;
		}
	}
}

static void PrintMidiDevice (int id, const char *name, uint16_t tech, uint32_t support)
{
	if (id == snd_mididevice)
	{
		Printf (TEXTCOLOR_BOLD);
	}
	Printf ("% 2d. %s : ", id, name);
	switch (tech)
	{
	case MIDIDEV_MIDIPORT:		Printf ("MIDIPORT");		break;
	case MIDIDEV_SYNTH:			Printf ("SYNTH");			break;
	case MIDIDEV_SQSYNTH:		Printf ("SQSYNTH");			break;
	case MIDIDEV_FMSYNTH:		Printf ("FMSYNTH");			break;
	case MIDIDEV_MAPPER:		Printf ("MAPPER");			break;
	case MIDIDEV_WAVETABLE:		Printf ("WAVETABLE");		break;
	case MIDIDEV_SWSYNTH:		Printf ("SWSYNTH");			break;
	}
	if (support & MIDICAPS_CACHE)
	{
		Printf (" CACHE");
	}
	if (support & MIDICAPS_LRVOLUME)
	{
		Printf (" LRVOLUME");
	}
	if (support & MIDICAPS_STREAM)
	{
		Printf (" STREAM");
	}
	if (support & MIDICAPS_VOLUME)
	{
		Printf (" VOLUME");
	}
	Printf (TEXTCOLOR_NORMAL "\n");
}

CCMD (snd_listmididevices)
{
	UINT id;
	MIDIOUTCAPS caps;
	MMRESULT res;

	PrintMidiDevice (-6, "WildMidi", MIDIDEV_SWSYNTH, 0);
#ifdef HAVE_FLUIDSYNTH
	PrintMidiDevice (-5, "FluidSynth", MIDIDEV_SWSYNTH, 0);
#endif
	PrintMidiDevice (-4, "Gravis Ultrasound Emulation", MIDIDEV_SWSYNTH, 0);
	PrintMidiDevice (-3, "Emulated OPL FM Synth", MIDIDEV_FMSYNTH, 0);
	PrintMidiDevice (-2, "TiMidity++", MIDIDEV_SWSYNTH, 0);
	if (nummididevices != 0)
	{
		for (id = 0; id < nummididevices; ++id)
		{
			res = midiOutGetDevCaps (id, &caps, sizeof(caps));
			if (res == MMSYSERR_NODRIVER)
				strcpy (caps.szPname, "<Driver not installed>");
			else if (res == MMSYSERR_NOMEM)
				strcpy (caps.szPname, "<No memory for description>");
			else if (res != MMSYSERR_NOERROR)
				continue;

			PrintMidiDevice (id, caps.szPname, caps.wTechnology, caps.dwSupport);
		}
	}
}

#else

// Everything but Windows uses this code.

CUSTOM_CVAR(Int, snd_mididevice, DEF_MIDIDEV, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
{
#ifdef __ANDROID__ //Limit to fluidsynth, gus, opl
	if (self < -5)
		self = -3;
	else if (self > -3)
		self = -5;
	else
		MIDIDeviceChanged(self);
#else
	if (self < -6)
		self = -6;
	else if (self > -2)
		self = -2;
	else
		MIDIDeviceChanged(self);
#endif
}

void I_BuildMIDIMenuList (FOptionValues *opt)
{
	AddDefaultMidiDevices(opt);
}

CCMD (snd_listmididevices)
{
	Printf("%s-6. WildMidi\n", -6 == snd_mididevice ? TEXTCOLOR_BOLD : "");
#ifdef HAVE_FLUIDSYNTH
	Printf("%s-5. FluidSynth\n", -5 == snd_mididevice ? TEXTCOLOR_BOLD : "");
#endif
	Printf("%s-4. Gravis Ultrasound Emulation\n", -4 == snd_mididevice ? TEXTCOLOR_BOLD : "");
	Printf("%s-3. Emulated OPL FM Synth\n", -3 == snd_mididevice ? TEXTCOLOR_BOLD : "");
	Printf("%s-2. TiMidity++\n", -2 == snd_mididevice ? TEXTCOLOR_BOLD : "");
}
#endif
