/*==============================================================================
Gain DSP Plugin Example
Copyright (c), Firelight Technologies Pty, Ltd 2004-2015.

This example shows how to create a simple gain DSP effect.
==============================================================================*/

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "fmod.hpp"
#include <random>
#include <ctime>

#define FMOD_GAIN_USEPROCESSCALLBACK            /* FMOD plugins have 2 methods of processing data.  
1. via a 'read' callback which is compatible with FMOD Ex but limited in functionality, or
2. via a 'process' callback which exposes more functionality, like masks and query before process early out logic. */

extern "C" {
	F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION* F_STDCALL FMODGetDSPDescription();
}

const float FMOD_GAIN_PARAM_GAIN_MIN = -80.0f;
const float FMOD_GAIN_PARAM_GAIN_MAX = 10.0f;
const float FMOD_GAIN_PARAM_GAIN_DEFAULT = 0.0f;
const float FMOD_GLITCH_PARAM_MOD_MIN = 0.0f;
const float FMOD_GLITCH_PARAM_MOD_MAX = 1.0f;
const float FMOD_GLITCH_PARAM_MOD_DEFAULT = 0.5f;
#define FMOD_GAIN_RAMPCOUNT 256

enum
{
	FMOD_GAIN_PARAM_GAIN = 0,
	FMOD_GAIN_PARAM_GLITCH = 1,
	FMOD_GAIN_PARAM_INVERT,
	FMOD_GAIN_NUM_PARAMETERS
};

#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= FMOD_GAIN_PARAM_GAIN_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? FMOD_GAIN_PARAM_GAIN_MIN : 20.0f * log10f((float)__linval__))

FMOD_RESULT F_CALLBACK FMOD_Gain_dspcreate(FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Gain_dsprelease(FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspreset(FMOD_DSP_STATE *dsp_state);
#ifdef FMOD_GAIN_USEPROCESSCALLBACK
FMOD_RESULT F_CALLBACK FMOD_Gain_dspprocess(FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
#else
FMOD_RESULT F_CALLBACK FMOD_Gain_dspread(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
#endif
FMOD_RESULT F_CALLBACK FMOD_Gain_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspsetparamint(FMOD_DSP_STATE *dsp_state, int index, int value);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspsetparambool(FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspsetparamdata(FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int length);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspgetparamint(FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspgetparambool(FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Gain_dspgetparamdata(FMOD_DSP_STATE *dsp_state, int index, void **value, unsigned int *length, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Gain_shouldiprocess(FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);
FMOD_RESULT F_CALLBACK FMOD_Gain_sys_register(FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Gain_sys_deregister(FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Gain_sys_mix(FMOD_DSP_STATE *dsp_state, int stage);

static bool                    FMOD_Gain_Running = false;
static FMOD_DSP_PARAMETER_DESC p_gain;
static FMOD_DSP_PARAMETER_DESC p_glitch;
static FMOD_DSP_PARAMETER_DESC p_invert;

FMOD_DSP_PARAMETER_DESC *FMOD_Gain_dspparam[FMOD_GAIN_NUM_PARAMETERS] =
{
	&p_gain,
	&p_glitch,
	&p_invert
};

FMOD_DSP_DESCRIPTION FMOD_Gain_Desc =
{
	FMOD_PLUGIN_SDK_VERSION,
	"CX2 Glitch",    // name
	0x00000001,     // plug-in version
	1,              // number of input buffers to process
	1,              // number of output buffers to process
	FMOD_Gain_dspcreate,
	FMOD_Gain_dsprelease,
	FMOD_Gain_dspreset,
#ifndef FMOD_GAIN_USEPROCESSCALLBACK
	FMOD_Gain_dspread,
#else
	0,
#endif
#ifdef FMOD_GAIN_USEPROCESSCALLBACK
	FMOD_Gain_dspprocess,
#else
	0,
#endif
	0,
	FMOD_GAIN_NUM_PARAMETERS,
	FMOD_Gain_dspparam,
	FMOD_Gain_dspsetparamfloat,
	0, // FMOD_Gain_dspsetparamint,
	FMOD_Gain_dspsetparambool,
	0, // FMOD_Gain_dspsetparamdata,
	FMOD_Gain_dspgetparamfloat,
	0, // FMOD_Gain_dspgetparamint,
	FMOD_Gain_dspgetparambool,
	0, // FMOD_Gain_dspgetparamdata,
	FMOD_Gain_shouldiprocess,
	0,                                      // userdata
	FMOD_Gain_sys_register,
	FMOD_Gain_sys_deregister,
	FMOD_Gain_sys_mix
};

extern "C"
{

	F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION* F_STDCALL FMODGetDSPDescription()
	{
		static float gain_mapping_values[] = { -80, -50, -30, -10, 10 };
		static float gain_mapping_scale[] = { 0, 2, 4, 7, 11 };

		FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING(p_gain, "Gain", "dB", "Gain in dB. -80 to 10. Default = 0", FMOD_GAIN_PARAM_GAIN_DEFAULT, gain_mapping_values, gain_mapping_scale);
		FMOD_DSP_INIT_PARAMDESC_FLOAT(p_glitch, "Glitch", "%", "Glitch percentage chance.", FMOD_GAIN_PARAM_GAIN_MIN, FMOD_GAIN_PARAM_GAIN_MAX, FMOD_GAIN_PARAM_GAIN_DEFAULT);
		FMOD_DSP_INIT_PARAMDESC_BOOL(p_invert, "Invert", "", "Invert signal. Default = off", false, 0);

		return &FMOD_Gain_Desc;
	}

}

class FMODGainState
{
public:
	FMODGainState();

	void read(float *inbuffer, float *outbuffer, unsigned int length, int channels);
	void reset();
	void setGain(float);
	void setGlitch(float);
	void setInvert(bool);
	float gain() const { return LINEAR_TO_DECIBELS(m_invert ? -m_target_gain : m_target_gain); }
	float glitch() const { return m_current_glitch; }
	FMOD_BOOL invert() const { return m_invert; }

private:
	float m_target_gain;
	float m_current_gain;
	float m_current_glitch;
	int   m_ramp_samples_left;
	bool  m_invert;
    float m_delay_buffer[2 * 441000];
    float m_max_delay_sec = 2;
    int currentDelayIndex = 441000;
};

FMODGainState::FMODGainState()
{
	srand(time(0));
    
	m_target_gain = DECIBELS_TO_LINEAR(FMOD_GAIN_PARAM_GAIN_DEFAULT);
	m_current_glitch = FMOD_GLITCH_PARAM_MOD_DEFAULT;
	m_invert = 0;
    currentDelayIndex = 0;
	reset();
}

void FMODGainState::read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    int delayTime = 441000;
    
    // Check delay index
    if (currentDelayIndex > (2 * 441000) - 1) {
        currentDelayIndex = 0;
    }
    
    unsigned int samples = length * channels;
    while (samples--) {
        if (currentDelayIndex > (2 * 441000) / 2){
            m_delay_buffer[currentDelayIndex - delayTime] += *inbuffer * .75f;
        } else {
            m_delay_buffer[currentDelayIndex + delayTime] += *inbuffer * .75f;
        }
        
        *outbuffer++ = *inbuffer++ + m_delay_buffer[currentDelayIndex++];
    }
    /*
	// Note: buffers are interleaved
	float gain = m_current_gain;

	if (m_ramp_samples_left)
	{
		float target = m_target_gain;
		float delta = (target - gain) / m_ramp_samples_left;
		while (length)
		{
			if (--m_ramp_samples_left)
			{
				gain += delta;
				for (int i = 0; i < channels; ++i)
				{
					*outbuffer++ = *inbuffer++ * gain;
				}
			}
			else
			{
				gain = target;
				break;
			}
			--length;
		}
	}

	unsigned int samples = length * channels;
	while (samples--)
	{
		if (m_current_glitch > ((float)rand() / (float)RAND_MAX)) {
			*outbuffer++ = *inbuffer++ * gain *  ((float)rand() / (float)RAND_MAX);
		}
		else {
			*outbuffer++ = *inbuffer++ * gain;
		}
	}

	m_current_gain = gain;
     */
}

void FMODGainState::reset()
{
	m_current_gain = m_target_gain;
	m_ramp_samples_left = 0;
}

void FMODGainState::setGain(float gain)
{
	m_target_gain = m_invert ? -DECIBELS_TO_LINEAR(gain) : DECIBELS_TO_LINEAR(gain);
	m_ramp_samples_left = FMOD_GAIN_RAMPCOUNT;
}

void FMODGainState::setGlitch(float glitch) {
	m_current_gain = glitch;
}

void FMODGainState::setInvert(bool invert)
{
	if (invert != m_invert)
	{
		m_target_gain = -m_target_gain;
		m_ramp_samples_left = FMOD_GAIN_RAMPCOUNT;
	}
	m_invert = invert;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_dspcreate(FMOD_DSP_STATE *dsp_state)
{
	dsp_state->plugindata = (FMODGainState *)FMOD_DSP_STATE_MEMALLOC(dsp_state, sizeof(FMODGainState), FMOD_MEMORY_NORMAL, "FMODGainState");
	if (!dsp_state->plugindata)
	{
		return FMOD_ERR_MEMORY;
	}
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_dsprelease(FMOD_DSP_STATE *dsp_state)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;
	FMOD_DSP_STATE_MEMFREE(dsp_state, state, FMOD_MEMORY_NORMAL, "FMODGainState");
	return FMOD_OK;
}

#ifdef FMOD_GAIN_USEPROCESSCALLBACK

FMOD_RESULT F_CALLBACK FMOD_Gain_dspprocess(FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL /*inputsidle*/, FMOD_DSP_PROCESS_OPERATION op)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

	if (op == FMOD_DSP_PROCESS_QUERY)
	{
		if (outbufferarray && inbufferarray)
		{
			outbufferarray[0].bufferchannelmask[0] = inbufferarray[0].bufferchannelmask[0];
			outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
			outbufferarray[0].speakermode = inbufferarray[0].speakermode;
		}
	}
	else
	{
		state->read(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, inbufferarray[0].buffernumchannels[0]); // input and output channels count match for this effect
	}

	return FMOD_OK;
}

#else

FMOD_RESULT F_CALLBACK FMOD_Gain_dspread(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int * /*outchannels*/)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;
	state->read(inbuffer, outbuffer, length, inchannels); // input and output channels count match for this effect
	return FMOD_OK;
}

#endif

FMOD_RESULT F_CALLBACK FMOD_Gain_dspreset(FMOD_DSP_STATE *dsp_state)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;
	state->reset();
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

	switch (index)
	{
	case FMOD_GAIN_PARAM_GAIN:
		state->setGain(value);
		return FMOD_OK;
	case FMOD_GAIN_PARAM_GLITCH:
		state->setGlitch(value);
		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

	switch (index)
	{
	case FMOD_GAIN_PARAM_GAIN:
		*value = state->gain();
		if (valuestr) sprintf(valuestr, "%.1f dB", state->gain());
		return FMOD_OK;
	case FMOD_GAIN_PARAM_GLITCH:
		*value = state->glitch();
		if (valuestr) sprintf(valuestr, "%.1f", state->glitch());
		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_dspsetparambool(FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

	switch (index)
	{
	case FMOD_GAIN_PARAM_INVERT:
		state->setInvert(value ? true : false);
		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_dspgetparambool(FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr)
{
	FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

	switch (index)
	{
	case FMOD_GAIN_PARAM_INVERT:
		*value = state->invert();
		if (valuestr) sprintf(valuestr, state->invert() ? "Inverted" : "Off");
		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_shouldiprocess(FMOD_DSP_STATE * /*dsp_state*/, FMOD_BOOL inputsidle, unsigned int /*length*/, FMOD_CHANNELMASK /*inmask*/, int /*inchannels*/, FMOD_SPEAKERMODE /*speakermode*/)
{
	if (inputsidle)
	{
		return FMOD_ERR_DSP_DONTPROCESS;
	}

	return FMOD_OK;
}


FMOD_RESULT F_CALLBACK FMOD_Gain_sys_register(FMOD_DSP_STATE * /*dsp_state*/)
{
	FMOD_Gain_Running = true;
	// called once for this type of dsp being loaded or registered (it is not per instance)
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_sys_deregister(FMOD_DSP_STATE * /*dsp_state*/)
{
	FMOD_Gain_Running = false;
	// called once for this type of dsp being unloaded or de-registered (it is not per instance)
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Gain_sys_mix(FMOD_DSP_STATE * /*dsp_state*/, int /*stage*/)
{
	// stage == 0 , before all dsps are processed/mixed, this callback is called once for this type.
	// stage == 1 , after all dsps are processed/mixed, this callback is called once for this type.
	return FMOD_OK;
}
