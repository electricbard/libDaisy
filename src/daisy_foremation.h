#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H
#include "daisy_seed.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the electric bard foremation board. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Shahin Etemadzadeh
    @date Jan 2022
    @ingroup boards
*/
class Foremation
{
  public:
    /** Switches */
    enum Sw
    {
        SW_OCT_UP,    
        SW_OCT_DOWN,
        SW_WAVE_1_A,
        SW_WAVE_1_B,
        SW_WAVE_2_A,
        SW_WAVE_2_B,
        SW_FILT_VCA_A,
        SW_FILT_VCA_B,
        SW_LAST,        /**< Last enum item */
    };

    /** CV control pins*/
    enum Ctrl
    {
        CTRL_PITCH_1,
        CTRL_FORMANT,
        CTRL_DETUNE,
        CTRL_AIRE,
        CTRL_PITCH_2,
        CTRL_BARREL,
        CTRL_FINE,
        CTRL_FM,
        CTRL_LAST,
    };

    /** Pitch LEDs */
    enum PitchLed
    {
        LED_1,
        LED_2,
        LED_3,
        LED_4,
        LED_5,
        LED_LAST,
    }


    /** Constructor */
    foremation() {}
    /** Destructor */
    ~foremation() {}

    /** Initializes the daisy seed, and patch hardware.*/
    void Init(bool boost = false);

    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);


    /** Starts the callback
    \param cb multichannel callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio */
    void StopAudio();

    /** Set the sample rate for the audio */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Get sample rate */
    float AudioSampleRate();

    /** Audio Block size defaults to 48.
    Change it using this function before StartingAudio
    \param size Audio block size.
    */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion.*/
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();


    /** Call at same rate as reading controls for good reads. */
    void ProcessAnalogControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /**
       Get value for a particular control
       \param k Which control to get
     */
    float GetKnobValue(Ctrl k);

    /**  Process the digital controls */
    void ProcessDigitalControls();

     /** Turn all leds off */
    void ClearLeds();

    /**
       Set pitch LED
     */
    void SetLed(int l);


    /* These are exposed for the user to access and manipulate directly
       Helper functions above provide easier access to much of what they are capable of.
    */
    DaisySeed       seed;                             /**< Seed object */
    AnalogControl   controls[CTRL_LAST];              /**< Array of controls*/
    Switch          switches[SW_LAST];
    //Led             pitch_led[LED_LAST];


  private:
    void SetHidUpdateRates();
    void InitAudio();
    void InitControls();
    void InitSwitches();
    void InitLeds();
};

} // namespace daisy

#endif
