
#include <string.h>
#include <math.h>
#include <array>
#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
#include "SdramPool.h"
#include "ReverbEngine.h"
//#include "Equalizer.h"
//#include "Controller.h"

//For at kunne flushe denormals
#include "core_cm7.h"

using namespace daisy;
using namespace daisysp;

// Use hwPod or hwSeed (Realtime or testing)
#define USE_HWPOD 

#define SAMPLE_RATE 		48000 // Set to 48000
#define SAMPLE_BUFFER_SIZE 		256
#define SAMPLE_TIME_NS		(SAMPLE_BUFFER_SIZE/(float)SAMPLE_RATE*1000000000) // in ns
#define NUM_COLORS 				7


#define USE_SDRAM // Must be used if FFT_SIZE is larger than 4096 //Skal vi tjekke den ud?

static Color my_colors[NUM_COLORS];
static DaisyPod hwPod; // Used for realtime audio and controls
static uint32_t  start, end, dur;
static volatile uint32_t cb_overruns = 0;
static volatile uint32_t cb_peak_ns = 0;

static inline void ConfigureFpuForRealtimeAudio()
{
	// Flush denormals to zero to avoid expensive subnormal float handling on M7.
	FPU->FPDSCR |= (1UL << 24) | (1UL << 25); // FZ + DN
	uint32_t fpscr = __get_FPSCR();
	fpscr |= (1UL << 24) | (1UL << 25);        // FZ + DN for current context
	__set_FPSCR(fpscr);
}

ReverbEngine<float>* reverbEngine = nullptr; //Global oprettelse, initialiseres i main efter hardwawre-initialisering for adgang til SDRAM memory

//DSY_SDRAM_DATA static ReverbEngine<float> reverbEngine;
//static Equalizer equalizerLeft;
//static Equalizer equalizerRight;
//static Controller controller(&equalizerLeft, &equalizerRight, &hwPod);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	start = System::GetTick();

	for (size_t i = 0; i < size; i++)
	{
		std::array<float, 2> sample = {in[0][i],in[1][i]};
		
		sample = reverbEngine->process(sample);

		//float sample = equalizerLeft.Process(in[0][i]); 
		//out[0][i] = sample;
		//out[1][i] = equalizerRight.Process(in[1][i]);

		
		out[0][i] = sample[0];
		out[1][i] = sample[1];
		// Bypass
		//out[0][i] = in[0][i];
		//out[1][i] = in[1][i];
	}

	end = System::GetTick();
	dur = (end - start) * 5; // ns
	if (dur > cb_peak_ns)
	{
		cb_peak_ns = dur;
	}
	if (dur > static_cast<uint32_t>(SAMPLE_TIME_NS))
	{
		cb_overruns++;
	}
	//dur = (end - start) / 200; // us
}

#ifndef USE_HWPOD

static Oscillator osc; // Oscillator for testing
static DaisySeed hwSeed; // Used for testing and logging without hwPod

void algoTester(void)
{
	// Placeholder for non-realtime testing of algorithms
	float sample, res, sample_rate; 

    // Below code creates COM4 port and displays log messages
	// hwSeed and hwPod do not work well together
	hwSeed.Configure();
	hwSeed.Init();
	hwSeed.StartLog();
    System::Delay(5000); // Wait 5 second
	hwSeed.PrintLine("Daisy Pod IIR Filter Example");

    hwSeed.SetAudioBlockSize(SAMPLE_BUFFER_SIZE);
    sample_rate = hwSeed.AudioSampleRate();
    osc.Init(sample_rate);
	hwSeed.PrintLine("Sample rate %.6f", sample_rate);	

	// Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(785); // Notch frequency Hz
    osc.SetAmp(0.5);
	
	equalizerLeft.setBypass(false);
	while (1) {
		//sample = 0.5f;
		sample = osc.Process();
		start = System::GetTick();
		res = equalizerLeft.Process(sample);
		end = System::GetTick();
		dur = (end - start) * 5; // ps
		//dur = (end - start) / 200; // us
		//hwSeed.PrintLine("IIR filter duration = %u ns start = %u end = %u", dur, start, end);
		//hwSeed.PrintLine("Sample input %.4f and output %.4f", sample, res);
		hwSeed.PrintLine("%.6f,%.6f", sample, res);
		System::Delay(100); // Wait 100 mseconds
	}

}

#endif

int main(void)
{
	//equalizerLeft.Init(SAMPLE_RATE);
	//equalizerLeft.setBypass(true);
	//equalizerRight.Init(SAMPLE_RATE);
	//equalizerRight.setBypass(true);

#ifdef USE_HWPOD // Realtime audio with Daisy Pod and IIR Filter bypass toggle

	//int32_t  inc;
	//bool eqOn = false;
	//int counter = 0;
	//int band = 0;

	my_colors[0].Init(Color::PresetColor::RED);
	my_colors[1].Init(Color::PresetColor::GOLD);
    my_colors[2].Init(Color::PresetColor::GREEN);
    my_colors[3].Init(Color::PresetColor::BLUE);
    my_colors[4].Init(Color::PresetColor::PURPLE);
	my_colors[5].Init(Color::PresetColor::WHITE);
    my_colors[6].Init(Color::PresetColor::OFF);
	
	hwPod.Init();
	
	// For at flushe denormals (tjek ud hvad den gør. Processor-specifik funktion)
	//ConfigureFpuForRealtimeAudio();
	resetDelayBufferCounter(); //OBS: Skal kaldet være her eller senere?
	hwPod.seed.StartLog();
	hwPod.SetAudioBlockSize(SAMPLE_BUFFER_SIZE); // number of samples handled per callback

	reverbEngine = new ReverbEngine<float>();
	
	if (SAMPLE_RATE == 48000)
		hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	else if (SAMPLE_RATE == 96000)
		hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
	else
		hwPod.seed.PrintLine("Unsupported sample rate %d", SAMPLE_RATE);		

	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

	uint32_t monitor_counter = 0;

    while(1)
    {
    	hwPod.ProcessAllControls(); 
		monitor_counter++;
		if (monitor_counter >= 1000)
		{
			hwPod.seed.PrintLine("cb_peak_ns=%lu budget_ns=%lu overruns=%lu",
			                     static_cast<unsigned long>(cb_peak_ns),
			                     static_cast<unsigned long>(SAMPLE_TIME_NS),
			                     static_cast<unsigned long>(cb_overruns));
			monitor_counter = 0;
		}
	
		// Debounce the Encoder at a steady, fixed rate.
		//hwPod.encoder.Debounce();
		//inc = hwPod.encoder.Increment();
		/*	
		if (inc < 0) {
			controller.adjust(DEC_PARAM_VALUE);
		}
		else if (inc > 0) {
			controller.adjust(INC_PARAM_VALUE);
		}
		if (hwPod.encoder.RisingEdge()) {
    	// Encoder-knap til at vælge param (trigger én gang)
			controller.incParam();
		}
        // using button1 turn EQ on/off
        if (hwPod.button1.RisingEdge()) {
			eqOn = !eqOn;
			controller.setBypass(!eqOn);
		}

       // using button2 select eq band
 		if (hwPod.button2.RisingEdge()) {
			band = (int)controller.adjust(SEL_BAND);
			hwPod.seed.PrintLine("Equalizer band %d", band);
			counter = 1;
		}
		// 
		hwPod.ClearLeds();

		// Opdaterer led1 med param eller off
		PARAMETER param = controller.getParam();

		//int EqOn = eqOn ? 3 : 4; // LED White if on, Off if bypass
		int led1;
		if (eqOn && (param == PM_GAIN)){
			hwPod.led1.SetColor(my_colors[0]);
		}
		else if (eqOn && (param == PM_FREQ)){
			hwPod.led1.SetColor(my_colors[2]);
		}
		// off-case
		else{
			hwPod.led1.SetColor(my_colors[6]);
		}
		hwPod.led2.SetColor(my_colors[band]);
		hwPod.UpdateLeds();
		
		if (counter % 200000 == 0) { // Print every ~1 seconds
			//hwPod.seed.PrintLine("Daisy Pod IIR Filter Example");
			//hwPod.seed.PrintLine("Equalizer duration = %u ns start = %u end = %u", dur, start, end);
			
			//controller.printState(dur, SAMPLE_TIME_NS);
			controller.printParam();
		}
			*/
		//counter++;
		System::Delay(1); // Wait 1 ms
    }

#else // Non-realtime test with Daisy Seed testing and logging

	algoTester();

#endif

}
