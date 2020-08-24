#include <chrono>

#include "PluginProcessor.h"
#include "PluginEditor.h"


class MainTimer :public juce::Timer {
 public:
  MainTimer(MainComponent *main) :main(main) {}

  void timerCallback() override {
    main->onUpdateTimer();
  }

 private:
  MainComponent *main;
};

MainComponent::MainComponent(MainAudioProcessor& p)
    : AudioProcessorEditor(p), p(p) {
  if (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::OperatingSystemType::Linux) {
    Desktop::getInstance().setGlobalScaleFactor(2);
  } else {
    Desktop::getInstance().setGlobalScaleFactor(1);
  }
  setResizable(true, true);
  setSize(400, 400);
  setResizeLimits(300, 300, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

  addAndMakeVisible(graph);
  graph.setBounds(getLocalBounds());

  addAndMakeVisible(label);
  label.setBounds(0, 0, getWidth(), getHeight());
  label.setFont(Font(64, Font::bold));

  timer = std::make_unique<MainTimer>(this);
  timer->startTimer(100);
}

MainComponent::~MainComponent() = default;

