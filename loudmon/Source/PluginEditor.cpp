#include <chrono>

#include "PluginProcessor.h"
#include "PluginEditor.h"



MainComponent::MainComponent(MainAudioProcessor& p)
    : AudioProcessorEditor(p) {

//  debug_plot.set_value_range(0, 1, 0, 1, false, false);
//  debug_plot.add_new_values("1", {{0.1f, -0.1f}, {0.5f, 0.5f}, {0.9f, 1.1f}});
//  addAndMakeVisible(debug_plot);

  if (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::OperatingSystemType::Linux) {
    Desktop::getInstance().setGlobalScaleFactor(2);
  } else {
    Desktop::getInstance().setGlobalScaleFactor(1);
  }
  setResizable(true, true);
  setSize(800, 800);
  setResizeLimits(300, 300, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
}

MainComponent::~MainComponent() {
}

