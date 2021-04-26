/*
** Surge Synthesizer is Free and Open Source Software
**
** Surge is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html
**
** Copyright 2004-2021 by various individuals as described by the Git transaction log
**
** All source at: https://github.com/surge-synthesizer/surge.git
**
** Surge was a commercial product from 2004-2018, with Copyright and ownership
** in that period held by Claes Johanson at Vember Audio. Claes made Surge
** open source in September 2018.
*/

#include "SurgeSynthClient.h"
#include "SurgeSynthesizer.h"

SurgeSynthClient::SurgeSynthClient(SurgeSynthesizer *s) : synth(s) {}

void SurgeSynthClient::enqueuePatchLoad(const int &id)
{
    synth->patchid_queue = id;
    // Looks scary but remember this only runs if audio thread is off
    synth->processThreadunsafeOperations();
}

void SurgeSynthClient::enqueuePatchFileLoad(const std::string &file)
{
    strncpy(synth->patchid_file, file.c_str(), FILENAME_MAX);
    synth->has_patchid_file = true;
}

void SurgeSynthClient::refreshEditor(bool b)
{
    synth->refresh_editor = b;
}

const Tunings::KeyboardMapping &SurgeSynthClient::getCurrentMapping() const
{
    return synth->storage.currentMapping;
}

const Tunings::Scale &SurgeSynthClient::getCurrentScale() const
{
    return synth->storage.currentScale;
}

SurgeStorageInterface *SurgeSynthClient::getStorageInterface() const
{
    return &(synth->storage);
}

bool SurgeSynthClient::isStandardMapping() const
{
    return synth->storage.isStandardMapping;
}

bool SurgeSynthClient::isStandardTuning() const
{
    return synth->storage.isStandardTuning;
}

bool SurgeSynthClient::isStandardScale() const
{
    return synth->storage.isStandardScale;
}

bool SurgeSynthClient::hasEnqueuedPatch() const
{
    return synth->patchid_queue > 0;
}

float SurgeSynthClient::noteToPitch(float note) const
{
    return synth->storage.note_to_pitch(note);
}

int SurgeSynthClient::scaleConstantNote() const
{
    return synth->storage.scaleConstantNote();
}

std::string SurgeSynthClient::getControllerLabel(int c)
{
    return synth->storage.getPatch().CustomControllerLabel[c];
}

int SurgeSynthClient::getControllerMIDIMapping(int c)
{
    return synth->storage.controllers[c];
}

DAWExtraStateStorage &SurgeSynthClient::getDawExtraState()
{
    return synth->storage.getPatch().dawExtraState;
}

bool SurgeSynthClient::isEngineHalted()
{
    return synth->halt_engine;
}

SurgeSynthClient::ModRoutingLockGuard::ModRoutingLockGuard(SurgeSynthClient *c) : c(c)
{
    c->synth->storage.modRoutingMutex.lock();
}

SurgeSynthClient::ModRoutingLockGuard::~ModRoutingLockGuard()
{
    c->synth->storage.modRoutingMutex.unlock();
}