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

#ifndef SURGE_XT_SURGESYNTHCLIENT_H
#define SURGE_XT_SURGESYNTHCLIENT_H

#include <string>
#include "Tunings.h"
#include "SurgeStorage.h"

class SurgeSynthesizer;
class Parameter;

struct timedata
{
    double ppqPos, tempo;
    int timeSigNumerator = 4, timeSigDenominator = 4;
};

/*
 * SurgeSynthClient is a way to interact with a SurgeSynth
 * cleanly off the audio thread. If you are running co-threaded
 * with the synth (which the python api and test api do)
 * you can call SurgeSynth methods directly, but if calling form
 * a UI thread use this client.
 *
 * So clearly, SurgeGUIEditor.h references this class not SurgeSynth.
 *
 * This class does not expose things like playNote or process; those
 * are audio thread APIs which can be called from the audio thread directly
 * on SurgeSynth. It exposes things needed to render or modify the parameter
 * set.
 */

class SurgeSynthClient
{
  public:
    class ParameterProxy
    {
        friend class SurgeSynthClient;
      private:
        explicit ParameterProxy(int i);
      public:
        int getMidiController(); // paramptr[i]->midictrl
        const char* getFullName(); // ptr->get_full_name()C
    };
    explicit SurgeSynthClient(SurgeSynthesizer *s);

    bool isEngineHalted();



    void enqueuePatchLoad(const int &id);
    void enqueuePatchFileLoad(const std::string &id);
    bool hasEnqueuedPatch() const;
    int getPatchId() const; // synth->patchid
    int getPatchCategoryId() const;
    std::string getPatchName() const; // synth->storage.getPatch().name
    std::string getPatchCategory() const;
    std::string getPatchAuthor() const;
    void selectRandomPatch();

    void refreshEditor(bool = true);
    bool getRefreshEditor() const; // just return synth->refresh_edtiro

    bool isStandardTuning() const;
    bool isStandardScale() const;
    bool isStandardMapping() const;
    const Tunings::Scale &getCurrentScale() const;
    const Tunings::KeyboardMapping &getCurrentMapping() const;
    bool hasODDSoundMTSClient() const; // synth->storage.oddsound_mts_client
    bool isODDSoundMTSActive() const; // synth->storage.oddsound_mts_active

    bool isMpeEnabled() const; // return synth->mpeEnabled;
    void setMpeEnabled(bool toThis);

    const timedata& getTimeData() const;

    float getVuPeak(int meter) const; // synth->vu_peak[meter];

    float noteToPitch(float note) const;
    int scaleConstantNote() const;

    int getLearnParam() const; // synth->learn_param
    void setLearnParam(int p);
    int getLearnCustom() const; // synth->learn_custom
    void setLearnCustom( int p);

    bool isActiveModulation(long ptag, modsources i) const;
    bool isBipolarModulation(modsources i) const;
    bool isModsourceUsed(modsources i) const;
    bool isModDestUsed(long i) const ;
    float getModulation(long, modsources) const;
    float getModDepth(long, modsources) const; // what's the difference?
    void setModulation(long, modsources);
    void clearModulation(long ptag, modsources i);

    bool hasActiveFxAt(int slot) const; // synht->fx[slot] != nullptr
    float getFxVuLevel(int fxslot, int channel) const; // synth->fx[current_fx]->vu[(i << 1) + channel]
    int getFxVuType(int fxslot, int index) const; // synth->fx[slot]->vu_type(i);
    int getFxVuPos(int fxslot, int index) const; // synth->fx[slot]->vu_pos(i);
    const char* getFxGroupLabel(int fxslot, int index) const; // synth->fx[slot]->group_label(i);
    int getFxGroupLabelYPos(int fxslot, int index) const; // synth->fx[slot]->group_label_ypos(i);

    SurgeStorageInterface *getStorageInterface() const;

    /*
     * This API is potentially unsafe. Please use it sparingly and thoughtfully.
     */
    SurgeStorage *getStorageUnsafe();

    /*
     * When we have threaded properly this will probably go away
     */
    struct ModRoutingLockGuard {
        explicit ModRoutingLockGuard(SurgeSynthClient *c);
        ~ModRoutingLockGuard();
        SurgeSynthClient *c;
    };

    /*
     * dawExtraState. For now I am not going to tread this in a thread-safe way really
     * but rather just assume we coordinate the reads and writes. But obviously that assumption
     * shoudl get fixed.
     */
    DAWExtraStateStorage &getDawExtraState();

    /*
     * Params and Controllers
     */
    ParameterProxy *getParamProxy(int i);


    /*
     * Clipboard
     */
    int getClipboardType() const; // synht->storage.getcbt
    void clipboardPaste(int, int, int);
    void clipboardCopy(int, int, int);

    /*
     * The client has a *const* view of the SurgePatch. You don't want to use this for direct
     * value access, really just ID access, but I'm too lazy to strictly enforce this in the API
     */
    const SurgePatch &getPatch();
    SurgePatch &getPatchUnsafe();

    std::string getControllerLabel(int c) const;
    void setControllerLabel(int c, const std::string &s);
    int getControllerMIDIMapping(int c) const;
    void setControllerMIDIMapping(int c, int midicc); // synth->storage.controllers[c] = midicc;

    /*
     * The synth when it recieves automation events can force a reset of a parameter or control
     * and that happens through this api
     */
    int getRefreshControlQueueSize() const { return 8; }
    int getRefreshControlQueue(int i ) const; // synth->refresh_ctrl_queue[i]
    float getRefreshControlQueueValue(int i) const; // synht->refresh_ctrl_queue_value[i]
    void clearRefreshControlQueue(int i); // synth->refresh_ctrl_queue[i] = -1;

    bool getRefreshParamOverflow() const; // synth->refresh_overflow
    void clearRefreshParamOverflow(); // synth->refresh_overflow = false;
    int getRefreshParamQueueSize() const { return 8; }
    int getRefreshParamQueue(int i) const; // synth->refresh_overflow
    void clearRefreshParamQueue(int i); // synth->refresh_overflow = false;

    /*
     * For now, I keep the synth id stuff alive
     */
    bool isValidSynthSideID(int j) const { return true; }
    void getParameterName(int i, char *pname) const; // FIXME that signatur tho
    void getParameterDisplay(int i, char *pname) const;
    float getParameter01(int i) const;
    int idForParameter(const Parameter *p) const; // return p->id
    void setParameter01(int id, float, bool, bool );

    /*
     * Patch maangement
     */
    void incrementPatch(bool, bool); // just passthrough to synth
    void incrementCategory(bool);

    const std::string &getHostProgram() const; // synth->hostProgram
    const std::string &getJuceWrapperType() const;

    void setHardclipMode(SurgeStorage::HardClipMode m, int scene = -1); // global at -1
    SurgeStorage::HardClipMode getHardclipMode(int scene = -1) const;

  private:
    SurgeSynthesizer *synth;
};

#endif // SURGE_XT_SURGESYNTHCLIENT_H
