#pragma once

#include <JuceHeader.h>
#include "SecondOrderIIRFilter.h"


class LUFSMeter {
 public:
  void prepareToPlay(double sampleRate, uint32_t maximumBlockSize, uint32_t numChannels);

  std::vector<std::tuple<size_t, std::vector<double>>> semiResults;

  void processBlock(AudioBuffer<float> &buffer);

  void yield_lufs(const std::vector<double> &sums, size_t offset);

  double getLUFS() const {
    return lufs_;
  }
 private:
  int strideSize;
  int measurementSize;
  double sampleRate;
  size_t globalOffset;
  std::atomic<double> lufs_;

  SecondOrderIIRFilter prefilter = SecondOrderIIRFilter(
      1.53512485958697,
      -2.69169618940638,
      1.19839281085285,
      -1.69065929318241,
      0.73248077421585);
  SecondOrderIIRFilter rlb = SecondOrderIIRFilter(
      1.0,
      -2.0,
      1.0,
      -1.99004745483398,
      0.99007225036621);
};

class MainComponent;
class MainAudioProcessor  : public AudioProcessor {
 public:
  //==============================================================================
  MainAudioProcessor();
  ~MainAudioProcessor() override;

  //==============================================================================
  void prepareToPlay (double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

  void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

  AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram (int index) override;
  const String getProgramName (int index) override;
  void changeProgramName (int index, const String& newName) override;

  void getStateInformation (MemoryBlock& destData) override;
  void setStateInformation (const void* data, int sizeInBytes) override;

  double getLUFS() const {
    return lufsMeter.getLUFS();
  }

 private:
  LUFSMeter lufsMeter;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioProcessor)
};

