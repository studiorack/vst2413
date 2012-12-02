#include "SynthDriver.h"
#include "emu2413.h"

namespace {
    const unsigned int kMsxClock = 3579540;
    
    namespace OPLLC {
        void KeyOn(OPLL *opll, int channel, int noteNumber) {
            static const int fnums[] = { 181, 192, 204, 216, 229, 242, 257, 272, 288, 305, 323, 343 };
            int fnum = fnums[(noteNumber - 37) % 12];
            int block = (noteNumber - 37) / 12;
            block = block < 0 ? 0 : (block > 7) ? 7 : block;
            OPLL_writeReg(opll, 0x10 + channel, fnum & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, 0x30 + (fnum >> 8) + (block << 1));
        }
        
        void KeyOff(OPLL *opll, int channel) {
            OPLL_writeReg(opll, 0x20 + channel, 0);;
        }
        
    }
}

SynthDriver::SynthDriver(unsigned int sampleRate)
:   sampleRate_(sampleRate),
    opll_(0)
{
    // Initialize the emulator.
    OPLL_init(kMsxClock, sampleRate_);
    opll_ = OPLL_new();
    // Set default program.
    for (int i = 0; i < 9; i++) {
        OPLL_writeReg(opll_, 0x30 + i, 0x30);
    }
}

SynthDriver::~SynthDriver() {
    OPLL_delete(opll_);
    OPLL_close();
}

void SynthDriver::SetSampleRate(unsigned int sampleRate) {
    sampleRate_ = sampleRate;
    OPLL_setClock(kMsxClock, sampleRate_);
}

void SynthDriver::KeyOn(int noteNumber, int velocity) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        if (!note.active_) {
            OPLLC::KeyOn(opll_, i, noteNumber);
            note.noteNumber_ = noteNumber;
            note.velocity_ = velocity;
            note.active_ = true;
            break;
        }
    }
}

void SynthDriver::KeyOff(int noteNumber) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        if (note.active_ && note.noteNumber_ == noteNumber) {
            OPLLC::KeyOff(opll_, i);
            note.active_ = false;
            break;
        }
    }
}

void SynthDriver::KeyOffAll() {
    for (int i = 0; i < 9; i++) {
        notes_[i].active_ = false;
    }
}

float SynthDriver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
