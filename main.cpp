
#include <string.h>
#include <math.h>
#include <array>
#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
#include "SdramPool.h"
#include "ReverbEngine.h"
#include "Matrix_array_v0.0.1.h"
#include "Controller.h"

// In Git Bash compile by:
// make clean
// make
// make program-dfu

//For at kunne flushe denormals
#include "core_cm7.h"

using namespace daisy;
using namespace daisysp;

// Use hwPod or hwSeed (Realtime or testing)
#define USE_HWPOD 

// Print parameters or not
#define PRINT_USER_PARAMETERS

#define SAMPLE_RATE 		48000 // Set to 48000
#define SAMPLE_BUFFER_SIZE 		512
#define BUFFER_TIME_NS		(SAMPLE_BUFFER_SIZE/(float)SAMPLE_RATE*1000000000) // in ns
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


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	start = System::GetTick();

	for (size_t i = 0; i < size; i++)
	{
		std::array<float, 2> sample = {in[0][i],in[1][i]};
		
		sample = reverbEngine->process(sample);
		
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
	if (dur > static_cast<uint32_t>(BUFFER_TIME_NS))
	{
		cb_overruns++;
	}
	//dur = (end - start) / 200; // us
}

#ifndef USE_HWPOD

static Oscillator osc; // Oscillator for testing
static DaisySeed hwSeed; // Used for testing and logging without hwPod
#endif

// shuffleHad matrix container. Is manually allocated to DTCM ram
int main(void)
{


#ifdef USE_HWPOD // Realtime audio with Daisy Pod and IIR Filter bypass toggle

	my_colors[0].Init(Color::PresetColor::RED);
	my_colors[1].Init(Color::PresetColor::GOLD);
    my_colors[2].Init(Color::PresetColor::GREEN);
    my_colors[3].Init(Color::PresetColor::BLUE);
    my_colors[4].Init(Color::PresetColor::PURPLE);
	my_colors[5].Init(Color::PresetColor::WHITE);
    my_colors[6].Init(Color::PresetColor::OFF);
	
	hwPod.Init();


	// Manually assign shuffleHad values. Zero-initialized on compile. Allocated in DTCM ram.
	initShuffleHadMatrix();

	// Create hadamard matrix
	createHadamardMatrix(8);
	
	
	// For at flushe denormals (tjek ud hvad den gør. Processor-specifik funktion)
	ConfigureFpuForRealtimeAudio();
	resetDelayBufferCounter(); //OBS: Skal kaldet være her eller senere?
	hwPod.seed.StartLog();
	hwPod.SetAudioBlockSize(SAMPLE_BUFFER_SIZE); // number of samples handled per callback

	reverbEngine = new ReverbEngine<float>();

	controller contr(&hwPod, reverbEngine); // Instantiate controller
	
	if (SAMPLE_RATE == 48000)
		hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	else if (SAMPLE_RATE == 96000)
		hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
	else
		hwPod.seed.PrintLine("Unsupported sample rate %d", SAMPLE_RATE);		

	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

	uint32_t monitor_counter = 0;

	hwPod.seed.PrintLine("Program started\n");	
    while(1)
    {
    	hwPod.ProcessAllControls(); // Reads all buttons

		contr.update(); // Maps button values to reverb parameters


		monitor_counter++;
		if (monitor_counter >= 20000)
		{
			#ifdef PRINT_USER_PARAMETERS
			// Print user parameters
			hwPod.seed.PrintLine("===== User parameters =====\n");	
			hwPod.seed.PrintLine("State: %d\n", contr.getState());	
			// hwPod.seed.PrintLine("Armed: %s", contr.getArmed() ? "true" : "false");
			hwPod.seed.PrintLine("RT60: %.1f\n", contr.getRT60());	
			hwPod.seed.PrintLine("Mix: %.2f\n", contr.getMix());	
			hwPod.seed.PrintLine("Size: %d\n", contr.getSize());	
			hwPod.seed.PrintLine("Low decay: %.2f\n", contr.getLoDecay());	
			hwPod.seed.PrintLine("High decay: %.2f\n", contr.getHiDecay());
			hwPod.seed.PrintLine("Low frequency: %.2f\n", contr.getLoFreq());
			hwPod.seed.PrintLine("High frequency: %.2f\n", contr.getHiFreq());
			#endif
			// Print cycle time
			// hwPod.seed.PrintLine("cb_peak_ns=%lu budget_ns=%lu overruns=%lu",
			//                      static_cast<unsigned long>(cb_peak_ns),
			//                      static_cast<unsigned long>(BUFFER_TIME_NS),
			//                      static_cast<unsigned long>(cb_overruns));
			//hwPod.seed.PrintLine("Test printout ");
			monitor_counter = 0;
		}
		hwPod.UpdateLeds();

		//counter++;
		//System::Delay(1); // Wait 0.1 ms
    }

#endif

}
