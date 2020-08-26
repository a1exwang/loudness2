#pragma once

#include <JuceHeader.h>
#include "SecondOrderIIRFilter.h"


class LUFSMeter {
 public:
  explicit LUFSMeter(double measurementSeconds = 0.4, double strideSeconds = 0.1)
      :measurementSeconds(measurementSeconds), strideSeconds(strideSeconds) { }

  void prepareToPlay(double sampleRate, uint32_t maximumBlockSize, uint32_t numChannels);

  std::vector<std::tuple<size_t, std::vector<double>>> semiResults;

  void processBlock(AudioBuffer<float> &buffer);

  void yieldLUFS(const std::vector<double> &sums, size_t offset);

  double getLUFS() const {
    return lufs_;
  }
 private:
  double measurementSeconds, strideSeconds;
  int strideSize = 1;
  int measurementSize = 1;
  size_t globalOffset = 0;
  std::atomic<double> lufs_ = 0;

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

  double getLUFSMomentary() const {
    return lufsMomentary.getLUFS();
  }

  double getLUFSShortTime() const {
    return lufsShortTime.getLUFS();
  }

  double getLUFSLongTime() const {
    return lufsLongTime.getLUFS();
  }

  float getMaxValue() const {
    return maxValue;
  }

  void enqueueReset() {
    toReset.store(true);
  }

 private:
  void resetData() {
    maxValue.store(0);
    toReset.store(false);
    lufsMomentary.prepareToPlay(getSampleRate(), getBlockSize(), getTotalNumOutputChannels());
    lufsShortTime.prepareToPlay(getSampleRate(), getBlockSize(), getTotalNumOutputChannels());
    lufsLongTime.prepareToPlay(getSampleRate(), getBlockSize(), getTotalNumOutputChannels());
  }

 private:
  std::atomic<float> maxValue = 0;
  std::atomic<bool> toReset = false;
  LUFSMeter lufsMomentary = LUFSMeter(0.4, 0.1);
  LUFSMeter lufsShortTime = LUFSMeter(2, 0.5);
  LUFSMeter lufsLongTime = LUFSMeter(10, 2.5);
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioProcessor)
};

