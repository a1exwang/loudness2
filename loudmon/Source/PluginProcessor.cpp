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
  auto editor = dynamic_cast<MainComponent*>(getActiveEditor());
  auto input_channels = getTotalNumInputChannels();
  auto output_channels = getTotalNumOutputChannels();
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
  // calculate RMS
  double sum = 0;
  for (int i = 0; i < buffer.getNumChannels(); i++) {
    auto channelData = buffer.getArrayOfReadPointers()[i];
    for (int j = 0; j < buffer.getNumSamples(); j++) {
      sum += channelData[j] * channelData[j];
    }
  }
  auto totalSamples = buffer.getNumSamples() * buffer.getNumChannels();
  if (totalSamples != 0) {
    sum /= totalSamples;
  }
  auto rms = sqrt(sum);
  std::cout << "RMS " << rms << std::endl;
  // we don't generate sound
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
