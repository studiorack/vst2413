#include "vst2413p.h"

namespace {
    typedef std::string String;
}

#pragma mark Creation and destruction

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Vst2413p(audioMaster);
}

Vst2413p::Vst2413p(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, SynthDriver::kPrograms, SynthDriver::kParameters),
    driver_(44100)
{
    if(audioMaster != NULL) {
        setNumInputs(0);
        setNumOutputs(1);
        setUniqueID(kUniqueId);
        canProcessReplacing();
        isSynth();
    }
    suspend();
}

#pragma mark
#pragma mark Processing functions

VstInt32 Vst2413p::processEvents(VstEvents* events) {
	for (VstInt32 i = 0; i < events->numEvents; i++) {
		if (events->events[i]->type != kVstMidiType) continue;

		const char* data = reinterpret_cast<VstMidiEvent*>(events->events[i])->midiData;
        
        switch (data[0] & 0xf0) {
            // key off
            case 0x80:
                driver_.KeyOff(data[1] & 0x7f);
                break;
            // key On
            case 0x90:
                driver_.KeyOn(data[1] & 0x7f, 1.0f / 128 * (data[2] & 0x7f));
                break;
            // all keys off
            case 0xb0:
                if (data[1] == 0x7e || data[1] == 0x7b) driver_.KeyOffAll();
                break;
            // pitch wheel
            case 0xe0: {
                int position = ((data[2] & 0x7f) << 7) + (data[1] & 0x7f);
                driver_.SetPitchWheel((1.0f / 0x2000) * (position - 0x2000));
                break;
            }
            default:
                break;
        }
	}
	return 1;
}

void Vst2413p::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
    for (VstInt32 i = 0; i < sampleFrames; i++) outputs[0][i] = driver_.Step();
}

#pragma mark
#pragma mark Program

void Vst2413p::setProgram(VstInt32 index) {
    driver_.SetProgram(static_cast<SynthDriver::ProgramID>(index));
}

void Vst2413p::setProgramName(char* name) {
    // not supported
}

void Vst2413p::getProgramName(char* name) {
    driver_.GetProgramName(driver_.GetProgram()).copy(name, kVstMaxProgNameLen);
}

bool Vst2413p::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text) {
    if (index < SynthDriver::kPrograms) {
        driver_.GetProgramName(static_cast<SynthDriver::ProgramID>(index)).copy(text, kVstMaxProgNameLen);
        return true;
    } else {
        return false;
    }
}

#pragma mark
#pragma mark Parameter

void Vst2413p::setParameter(VstInt32 index, float value) {
    driver_.SetParameter(static_cast<SynthDriver::ParameterID>(index), value);
}

float Vst2413p::getParameter(VstInt32 index) {
    return driver_.GetParameter(static_cast<SynthDriver::ParameterID>(index));
}

void Vst2413p::getParameterLabel(VstInt32 index, char* text) {
    driver_.GetParameterLabel(static_cast<SynthDriver::ParameterID>(index)).copy(text, kVstMaxParamStrLen);
}

void Vst2413p::getParameterDisplay(VstInt32 index, char* text) {
    driver_.GetParameterText(static_cast<SynthDriver::ParameterID>(index)).copy(text, kVstMaxParamStrLen);
}

void Vst2413p::getParameterName(VstInt32 index, char* text) {
    driver_.GetParameterName(static_cast<SynthDriver::ParameterID>(index)).copy(text, kVstMaxParamStrLen);
}

#pragma mark
#pragma mark Output settings

void Vst2413p::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    driver_.SetSampleRate(sampleRate);
}

bool Vst2413p::getOutputProperties(VstInt32 index, VstPinProperties* properties) {
    if (index == 0) {
        String("1 Out").copy(properties->label, kVstMaxLabelLen);
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

#pragma mark
#pragma mark Plug-in properties

bool Vst2413p::getEffectName(char* name) {
    String("VST2413P").copy(name, kVstMaxEffectNameLen);
    return true;
}

bool Vst2413p::getVendorString(char* text) {
    String("Radium Software").copy(text, kVstMaxVendorStrLen);
    return true;
}

bool Vst2413p::getProductString(char* text) {
    String("VST2413P").copy(text, kVstMaxProductStrLen);
    return true;
}

VstInt32 Vst2413p::getVendorVersion() {
    return 1000;
}

VstInt32 Vst2413p::canDo(char* text) {
    String str = text;
    if (str == "receiveVstEvents") return 1;
    if (str == "receiveVstMidiEvent") return 1;
    return 0;
}

#pragma mark
#pragma mark MIDI channels I/O

VstInt32 Vst2413p::getNumMidiInputChannels() {
    return 1;
}

VstInt32 Vst2413p::getNumMidiOutputChannels() {
    return 0;
}