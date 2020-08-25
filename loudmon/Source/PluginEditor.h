#pragma once

#include <list>
#include <string>
#include <map>
#include <utility>

#include <JuceHeader.h>
#include "Graph.h"

class MainComponent : public juce::AudioProcessorEditor {
 public:
  explicit MainComponent(MainAudioProcessor& p);
  ~MainComponent() override;

  void onUpdateTimer() {
    auto lufs = p.getLUFSMomentary();
    auto now = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "LUFS Momentary " << std::setprecision(3) << p.getLUFSMomentary() << "dB" << std::endl
       << "LUFS Short Time " << std::setprecision(3) << p.getLUFSShortTime() << "dB" << std::endl
       << "LUFS Long Time " << std::setprecision(3) << p.getLUFSLongTerm() << "dB" << std::endl;
    label.setText(ss.str(), dontSendNotification);
    graph.addValue(now, lufs, p.getSampleRate());
  }
  void resized() override {
    graph.setBounds(getLocalBounds());
    label.setBounds(getLocalBounds());
  }
 protected:

  Label label;
  Graph graph;
  std::unique_ptr<Timer> timer;
 private:
  MainAudioProcessor &p;
  //==============================================================================
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

