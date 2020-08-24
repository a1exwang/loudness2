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
  lufsMeter.prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
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
  AudioBuffer<float> b2 = buffer;
  MidiBuffer m2 = midiMessages;

  lufsMeter.processBlock(buffer);
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
    assert(strideSize > std::get<0>(semiResults.back()));
    newMeasurementStart = strideSize - std::get<0>(semiResults.back());
  }
  for (auto &sr : semiResults) {
    auto lastCount = std::get<0>(sr);
    auto sums = std::get<1>(sr);

    for (int i = 0; ; i++) {
      if (i >= n) {
        semiResultsNew.emplace_back(lastCount + n, sums);
        break;
      } else if (lastCount + i >= measurementSize) {
        yield_lufs(sums, i + globalOffset);
        break;
      } else {
        for (int c = 0; c < buffer.getNumChannels(); c++) {
          auto value = buffer.getSample(c, i);
          sums[c] = value * value;
        }
      }
    }
  }

  for (auto offset = newMeasurementStart; offset < n; offset += strideSize) {
    std::vector<double> sums(buffer.getNumChannels(), 0);
    for (int i = 0; ; i++) {
      if (offset + i >= n) {
        // semi
        semiResultsNew.emplace_back(i, sums);
        break;
      } else if (i >= measurementSize) {
        // done
        yield_lufs(sums, i + offset);
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
void LUFSMeter::yield_lufs(const std::vector<double> &sums, size_t offset) {
  double lufs = -0.691 + 10 * log10(std::accumulate(sums.begin(), sums.end(), 0.0));
  std::cout << "LUFS@" << offset << " = " << std::setprecision(3) << lufs << "dB" << std::endl;
  lufs_ = lufs;
}
void LUFSMeter::prepareToPlay(double sampleRate, uint32_t maximumBlockSize, uint32_t numChannels) {

  // calculate paramters for current sample rate
  // we cannot resize the list since the filter type does not have copy constructor
  prefilter.prepareToPlay(sampleRate, numChannels);
  rlb.prepareToPlay(sampleRate, numChannels);
  this->sampleRate = sampleRate;
  // make sure strideSize is factor of measurementSize
  measurementSize = static_cast<int>(0.4 * sampleRate) / 4 * 4;
  strideSize = measurementSize / 4;
  globalOffset = 0;
  semiResults.clear();
}
