#include <chrono>

#include "PluginProcessor.h"
#include "PluginEditor.h"


class MainTimer :public juce::Timer {
 public:
  explicit MainTimer(MainComponent *main) :main(main) {}

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
  setSize(480, 480);
  setResizeLimits(300, 300, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

  addAndMakeVisible(graph);

  addAndMakeVisible(label);
  label.setFont(Font(32));

  setMyBounds();

  timer = std::make_unique<MainTimer>(this);
  timer->startTimer(1000/60);
}

MainComponent::~MainComponent() {
  timer->stopTimer();
  timer.reset();
}

