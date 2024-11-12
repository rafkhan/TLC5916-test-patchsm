#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
dsy_gpio dataGpio;
dsy_gpio clockGpio;
dsy_gpio latchGpio;

Metro tick;
int clkState = 0;
int clkCount = 0;
int tickCount = 0;

int ledTempState[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int ledState[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
  hw.ProcessAllControls();
  for (size_t i = 0; i < size; i++)
  {
    OUT_L[i] = IN_L[i];
    OUT_R[i] = IN_R[i];

    uint8_t isTick = tick.Process();
    if (isTick)
    {
      ledTempState[tickCount] = !ledTempState[tickCount];
      tickCount = (tickCount + 1) % 8;
    }
  }

  clkState = !clkState;
  dsy_gpio_write(&clockGpio, clkState);

  if (clkState)
  {
    dsy_gpio_write(&dataGpio, ledState[clkCount]);
    
    if (clkCount == 7)
    {
      dsy_gpio_write(&latchGpio, 1);
      memcpy(ledTempState, ledState, sizeof(ledState));
    }
    else
    {
      dsy_gpio_write(&latchGpio, 0);
    }
    clkCount = (clkCount + 1) % 8;
  }
  else
  {
    dsy_gpio_write(&dataGpio, 0);
  }

}

int main(void)
{
  hw.Init();
  float sampleRate = hw.AudioSampleRate();
  tick.Init(0.1, sampleRate);

  dataGpio.pin = (DaisyPatchSM::C2);
  dataGpio.mode = DSY_GPIO_MODE_OUTPUT_PP;
  dataGpio.pull = DSY_GPIO_NOPULL;

  clockGpio.pin = (DaisyPatchSM::C2);
  clockGpio.mode = DSY_GPIO_MODE_OUTPUT_PP;
  clockGpio.pull = DSY_GPIO_NOPULL;

  latchGpio.pin = (DaisyPatchSM::C2);
  latchGpio.mode = DSY_GPIO_MODE_OUTPUT_PP;
  latchGpio.pull = DSY_GPIO_NOPULL;

  hw.SetAudioBlockSize(4); // number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  hw.StartAudio(AudioCallback);

  while (1)
  {
  }
}
