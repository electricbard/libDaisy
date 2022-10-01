#include "daisy_foremation.h"

using namespace daisy;

// Hardware Definitions
#define PIN_ENC_CLICK 0
#define PIN_ENC_B 11
#define PIN_ENC_A 12

#define PIN_CTRL_PITCH_1    15
#define PIN_CTRL_FORMANT    16
#define PIN_CTRL_DETUNE     17
#define PIN_CTRL_AIRE       18
#define PIN_CTRL_PITCH_2    19
#define PIN_SIG_FM          20
#define PIN_CTRL_BARREL     21
#define PIN_CTRL_FINE       24
#define PIN_CTRL_FM         25

#define PIN_SW_OCT_UP       22
#define PIN_SW_OCT_DOWN     23
#define PIN_SW_WAVE_1_A     29
#define PIN_SW_WAVE_1_B     28
#define PIN_SW_WAVE_2_A     27
#define PIN_SW_WAVE_2_B     26
#define PIN_SW_FILT_VCA_A   0
#define PIN_SW_FILT_VCA_B   30

#define PIN_LED_1           1
#define PIN_LED_2           2
#define PIN_LED_3           3
#define PIN_LED_4           4
#define PIN_LED_5           5

uint8_t led_numbers[Foremation::LED_LAST] = {
        PIN_LED_1,
        PIN_LED_2,
        PIN_LED_3,
        PIN_LED_4,
        PIN_LED_5,
    };

uint8_t pin_numbers[Foremation::SW_LAST] = {
        PIN_SW_OCT_UP,
        PIN_SW_OCT_DOWN,
        PIN_SW_WAVE_1_A,
        PIN_SW_WAVE_1_B,
        PIN_SW_WAVE_2_A,
        PIN_SW_WAVE_2_B,
        PIN_SW_FILT_VCA_A,
        PIN_SW_FILT_VCA_B,
    };

void Foremation::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);
    InitAudio();
    InitControls();
    InitSwitches();
    InitLeds();
}

void Foremation::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void Foremation::SetHidUpdateRates()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }
}

void Foremation::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void Foremation::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void Foremation::StopAudio()
{
    seed.StopAudio();
}

void Foremation::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float Foremation::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void Foremation::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t Foremation::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float Foremation::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void Foremation::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void Foremation::StopAdc()
{
    seed.adc.Stop();
}


void Foremation::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}
float Foremation::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

void Foremation::ProcessDigitalControls()
{
    for(size_t i = 0; i < SW_LAST; i++)
    {
        switches[i].Debounce();
    }
}

void Foremation::ClearLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        //leds[i].Set(1.0f);
        dsy_gpio_write(&leds[i], true);
    }
}

void Foremation::SetLed(int l)
{
    ClearLeds();        //turn off all leds
    dsy_gpio_write(&leds[l], false);
}

// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void Foremation::InitAudio()
{
    // Handle Seed Audio as-is and then
    SaiHandle::Config sai_config;
    // Internal Codec
    if(seed.CheckBoardVersion() == DaisySeed::BoardVersion::DAISY_SEED_1_1)
    {
        sai_config.pin_config.sa = {DSY_GPIOE, 6};
        sai_config.pin_config.sb = {DSY_GPIOE, 3};
        sai_config.a_dir         = SaiHandle::Config::Direction::RECEIVE;
        sai_config.b_dir         = SaiHandle::Config::Direction::TRANSMIT;
    }
    else
    {
        sai_config.pin_config.sa = {DSY_GPIOE, 6};
        sai_config.pin_config.sb = {DSY_GPIOE, 3};
        sai_config.a_dir         = SaiHandle::Config::Direction::TRANSMIT;
        sai_config.b_dir         = SaiHandle::Config::Direction::RECEIVE;
    }
    sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config.pin_config.fs   = {DSY_GPIOE, 4};
    sai_config.pin_config.mclk = {DSY_GPIOE, 2};
    sai_config.pin_config.sck  = {DSY_GPIOE, 5};

    SaiHandle sai_handle;
    sai_handle.Init(sai_config);

    // Reinit Audio for _both_ codecs...
    AudioHandle::Config cfg;
    cfg.blocksize  = 48;
    cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    cfg.postgain   = 0.5f;
    seed.audio_handle.Init(cfg, sai_handle);
}

void Foremation::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[CTRL_PITCH_1].InitSingle(seed.GetPin(PIN_CTRL_PITCH_1));
    cfg[CTRL_FORMANT].InitSingle(seed.GetPin(PIN_CTRL_FORMANT));
    cfg[CTRL_DETUNE].InitSingle(seed.GetPin(PIN_CTRL_DETUNE));
    cfg[CTRL_AIRE].InitSingle(seed.GetPin(PIN_CTRL_AIRE));
    cfg[CTRL_PITCH_2].InitSingle(seed.GetPin(PIN_CTRL_PITCH_2));
    cfg[CTRL_BARREL].InitSingle(seed.GetPin(PIN_CTRL_BARREL));
    cfg[CTRL_FINE].InitSingle(seed.GetPin(PIN_CTRL_FINE));
    cfg[CTRL_FM].InitSingle(seed.GetPin(PIN_CTRL_FM));
    cfg[SIG_FM].InitSingle(seed.GetPin(PIN_SIG_FM));

    // Initialize ADC
    seed.adc.Init(cfg, CTRL_LAST);

    // Initialize AnalogControls, with flip set to true
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate(), true);
    }
}

void Foremation::InitSwitches()
{
    //    // button1
    //    button1.Init(seed.GetPin(SW_1_PIN), callback_rate_);
    //    // button2
    //    button2.Init(seed.GetPin(SW_2_PIN), callback_rate_);
    //
    //    buttons[BUTTON_1] = &button1;
    //    buttons[BUTTON_2] = &button2;

    //switches[PIN_SW_OCT_UP].Init(seed.GetPin(pin_numbers[PIN_SW_OCT_UP]), AudioCallbackRate(), Switch::TYPE_MOMENTARY, Switch::POLARITY_INVERTED, Switch::PULL_UP);
    //switches[PIN_SW_OCT_DOWN].Init(seed.GetPin(pin_numbers[PIN_SW_OCT_DOWN]), AudioCallbackRate(), Switch::TYPE_MOMENTARY, Switch::POLARITY_INVERTED, Switch::PULL_UP);

    // for(size_t i = PIN_SW_WAVE_1_A; i < SW_LAST; i++)
    // {
    //     switches[i].Init(seed.GetPin(pin_numbers[i]), AudioCallbackRate(), Switch::TYPE_TOGGLE, Switch::POLARITY_INVERTED, Switch::PULL_UP);
    // }
    for(size_t i = 0; i < SW_LAST; i++)
    {
        switches[i].Init(seed.GetPin(pin_numbers[i]));
    }
}

void Foremation::InitLeds()
{   
    for (size_t i = 0; i < LED_LAST; i++)
    {
        //leds[i].Init(seed.GetPin(led_numbers[i]), false, AudioCallbackRate());
        /** @brief Explicity initialize all configuration for the GPIO 
     *  @param p Pin specifying the physical connection on the hardware
     *  @param m Mode specifying the behavior of the GPIO (input, output, etc.). Defaults to Mode::INPUT
     *  @param pu Pull up/down state for the GPIO. Defaults to Pull::NOPULL
     *  @param sp Speed setting for drive strength/slew rate. Defaults to Speed::Slow
    */
    /*leds[i].Init(seed.GetPin(led_numbers[i]),
              GPIO::Mode::OUTPUT_OD,
              GPIO::Pull::NOPULL,
              GPIO::Speed::LOW);*/

    leds[i].pin  = seed.GetPin(led_numbers[i]);
    leds[i].mode = DSY_GPIO_MODE_OUTPUT_OD;
    leds[i].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&leds[i]);
    }

    ClearLeds();
}