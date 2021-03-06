#include <cassert>
#include <type_traits>

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MainAudioProcessor::MainAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsSynth
                          .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                          .withOutput ("Output", AudioChannelSet::stereo(), true)
                       )
#endif
{

}

MainAudioProcessor::~MainAudioProcessor() {
}

const String MainAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool MainAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool MainAudioProcessor::producesMidi() const {
  return false;
}

bool MainAudioProcessor::isMidiEffect() const {
  return false;
}

double MainAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int MainAudioProcessor::getNumPrograms() {
    return 1;
}

int MainAudioProcessor::getCurrentProgram() {
    return 0;
}

void MainAudioProcessor::setCurrentProgram (int) { }

const String MainAudioProcessor::getProgramName (int) {
    return {};
}

void MainAudioProcessor::changeProgramName (int, const String&) { }

//==============================================================================
void MainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
  lufsMomentary.prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
  lufsShortTime.prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
  lufsLongTime.prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
  maxValue = 0;
}

void MainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MainAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
  if (toReset.load()) {
    resetData();
  }

  AudioBuffer<float> bufferCopy(buffer.getNumChannels(), buffer.getNumSamples());
  {
    for (int c = 0; c < buffer.getNumChannels(); c++) {
	  bufferCopy.copyFrom(c, 0, buffer, c, 0, buffer.getNumSamples());
    }
    lufsMomentary.processBlock(bufferCopy);
  }
  {
    for (int c = 0; c < buffer.getNumChannels(); c++) {
	  bufferCopy.copyFrom(c, 0, buffer, c, 0, buffer.getNumSamples());
    }
    lufsShortTime.processBlock(bufferCopy);
  }
  {
    for (int c = 0; c < buffer.getNumChannels(); c++) {
	  bufferCopy.copyFrom(c, 0, buffer, c, 0, buffer.getNumSamples());
    }
    lufsLongTime.processBlock(bufferCopy);
  }

  // calculate true max
  auto localMaxValue = maxValue.load();
  for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
    for (int i = 0; i < buffer.getNumSamples(); i++) {
      auto value = abs(buffer.getSample(channel, i));
      if (value > localMaxValue) {
        localMaxValue = value;
      }
    }
  }
  maxValue.store(localMaxValue);

  // FIXME: disable output currently for debug
  buffer.clear();
}

//==============================================================================
bool MainAudioProcessor::hasEditor() const {
    return true;
}

AudioProcessorEditor* MainAudioProcessor::createEditor() {
  return new MainComponent(*this);
}

//==============================================================================
void MainAudioProcessor::getStateInformation (MemoryBlock&) {
}

void MainAudioProcessor::setStateInformation (const void*, int) {
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MainAudioProcessor();
}


void LUFSMeter::processBlock(AudioBuffer<float> &buffer) {

  prefilter.processBlock(buffer);
  rlb.processBlock(buffer);

  std::vector<std::tuple<size_t, std::vector<double>>> semiResultsNew;
  auto n = buffer.getNumSamples();
  size_t newMeasurementStart;
  if (semiResults.empty()) {
    // global first
    newMeasurementStart = 0;
  } else {
    if (strideSize < std::get<0>(semiResults.back())) {
      assert(strideSize >= std::get<0>(semiResults.back()));
    }
    newMeasurementStart = strideSize - std::get<0>(semiResults.back());
  }
  for (auto &sr : semiResults) {
    auto lastCount = std::get<0>(sr);
    auto sums = std::get<1>(sr);

    for (int i = 0; ; i++) {
      if (lastCount + i >= measurementSize) {
        yieldLUFS(sums, globalOffset + i);
        break;
      } else if (i >= n) {
        assert(i == n);
        assert(i < strideSize);
        semiResultsNew.emplace_back(lastCount + n, sums);
        break;
      } else {
        for (int c = 0; c < buffer.getNumChannels(); c++) {
          auto value = buffer.getSample(c, i);
          sums[c] += value * value;
        }
      }
    }
  }

  for (auto offset = newMeasurementStart; offset < n; offset += strideSize) {
    std::vector<double> sums(buffer.getNumChannels(), 0);
    for (int i = 0; ; i++) {
      if (i >= measurementSize) {
        // done
        yieldLUFS(sums, globalOffset + i + offset);
        break;
      } else if (offset + i >= n) {
        // semi
        semiResultsNew.emplace_back(i, sums);
        break;
      } else {
        for (int c = 0; c < buffer.getNumChannels(); c++) {
          auto value = buffer.getSample(c, offset + i);
          sums[c] += value * value;
        }
      }
    }
  }
  semiResults = semiResultsNew;
  globalOffset += buffer.getNumSamples();
}
void LUFSMeter::yieldLUFS(const std::vector<double> &sums, size_t offset) {
  double lufs = -std::numeric_limits<double>::infinity();
  if (sums.size() > 0) {
    double average = std::accumulate(sums.begin(), sums.end(), 0.0) / measurementSize;
    // divided by 2 to do sqrt
    lufs = -0.691 + 10 * log10(average);
  }
  std::stringstream ss;
  ss<< "LUFS " << lufs;
  DBG(ss.str());
  lufs_ = lufs;
}
void LUFSMeter::prepareToPlay(double sampleRate, uint32_t maximumBlockSize, uint32_t numChannels) {

  // calculate paramters for current sample rate
  // we cannot resize the list since the filter type does not have copy constructor
  prefilter.prepareToPlay(sampleRate, numChannels);
  rlb.prepareToPlay(sampleRate, numChannels);
  // make sure strideSize is factor of measurementSize
  measurementSize = static_cast<int>(measurementSeconds * sampleRate);
  strideSize = static_cast<int>(strideSeconds * sampleRate);
  globalOffset = 0;
  semiResults.clear();
  lufs_ = -std::numeric_limits<float>::infinity();
}
