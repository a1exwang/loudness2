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
  if (showGraph) {
    setSize(minimumWidth + defaultGraphWidth, minimumHeight);
  } else {
    setSize(minimumWidth, minimumHeight);
  }
  setResizeLimits(minimumWidth, minimumHeight, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

  titleLabel.setText("a1ex's LM", dontSendNotification);
  addAndMakeVisible(titleLabel);

  addChildComponent(graph);
  graph.setVisible(showGraph);

  addAndMakeVisible(buttonReset);
  buttonReset.setButtonText("Reset");
  buttonReset.onClick = [this]() {
    this->p.enqueueReset();
    this->graph.reset();
    this->repaint();
  };

  addAndMakeVisible(buttonToggleGraph);
  buttonToggleGraph.setButtonText("Toggle Graph");
  buttonToggleGraph.onClick = [this]() {
    this->toggleGraph();
  };

  addAndMakeVisible(labelFPS, 0);
  labelFPS.setFont(Font(16));
  labelFPS.setJustificationType(Justification::topRight);

  addAndMakeVisible(momentaryLabel);
  momentaryLabel.setFont(Font(labelTextHeight));
  momentaryLabel.setJustificationType(Justification::centred);
  momentaryLabel.setText("Momentary(400ms)", dontSendNotification);

  addAndMakeVisible(momentaryValueLabel);
  momentaryValueLabel.setFont(Font(valueTextHeight, Font::bold));
  momentaryValueLabel.setJustificationType(Justification::centred);
  momentaryValueLabel.setColour(juce::Label::textColourId, momentaryTextColour);

  addAndMakeVisible(shortTimeLabel);
  shortTimeLabel.setFont(Font(labelTextHeight));
  shortTimeLabel.setJustificationType(Justification::centred);
  shortTimeLabel.setText("Short Time(2s)", dontSendNotification);

  addAndMakeVisible(shortTimeValueLabel);
  shortTimeValueLabel.setFont(Font(valueTextHeight, Font::bold));
  shortTimeValueLabel.setJustificationType(Justification::centred);
  shortTimeValueLabel.setColour(juce::Label::textColourId, shortTimeTextColour);

  addAndMakeVisible(longTimeLabel);
  longTimeLabel.setFont(Font(labelTextHeight));
  longTimeLabel.setJustificationType(Justification::centred);
  longTimeLabel.setText("Long Time(10s)", dontSendNotification);

  addAndMakeVisible(longTimeValueLabel);
  longTimeValueLabel.setFont(Font(valueTextHeight, Font::bold));
  longTimeValueLabel.setJustificationType(Justification::centred);
  longTimeValueLabel.setColour(juce::Label::textColourId, longTimeTextColour);

  addAndMakeVisible(trueMaxLabel);
  trueMaxLabel.setFont(Font(labelTextHeight));
  trueMaxLabel.setJustificationType(Justification::centred);
  trueMaxLabel.setText("True Max", dontSendNotification);

  addAndMakeVisible(trueMaxValueLabel);
  trueMaxValueLabel.setFont(Font(valueTextHeight, Font::bold));
  trueMaxValueLabel.setJustificationType(Justification::centred);
  trueMaxValueLabel.setColour(juce::Label::textColourId, trueMaxTextColour);

  setMyBounds();

  timer = std::make_unique<MainTimer>(this);
  timer->startTimer(1000 / 40);
}

MainComponent::~MainComponent() {
  timer->stopTimer();
  timer.reset();
}

void MainComponent::onUpdateTimer() {
  auto lufs = p.getLUFSMomentary();
  auto now = std::chrono::high_resolution_clock::now();
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::setfill('0') << p.getLUFSMomentary();
    momentaryValueLabel.setText(ss.str(), dontSendNotification);
  }
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::setfill('0') << p.getLUFSShortTime();
    shortTimeValueLabel.setText(ss.str(), dontSendNotification);
  }
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::setfill('0') << p.getLUFSLongTime();
    longTimeValueLabel.setText(ss.str(), dontSendNotification);
  }
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::setfill('0') << (10 * log10f(p.getMaxValue()));
    trueMaxValueLabel.setText(ss.str(), dontSendNotification);
  }
  graph.addValue(now, lufs, p.getSampleRate());

  labelFPS.setText(std::to_string(static_cast<int>(fps)), dontSendNotification);
}

template <typename T>
Rectangle<T> pad(Rectangle<T> rect, T size) {
  rect.removeFromTop(size);
  rect.removeFromBottom(size);
  rect.removeFromLeft(size);
  rect.removeFromRight(size);
  return rect;
}

template <typename T>
Rectangle<T> padVertical(Rectangle<T> rect, T size) {
  rect.removeFromTop(size);
  rect.removeFromBottom(size);
  return rect;
}

template <typename T>
Rectangle<T> padHorizontal(Rectangle<T> rect, T size) {
  rect.removeFromLeft(size);
  rect.removeFromRight(size);
  return rect;
}


void MainComponent::setMyBounds() {
  auto area = getLocalBounds();

  auto titleRowArea = area.removeFromTop(titleAreaHeight);
  auto fpsArea = titleRowArea.removeFromRight(fpsAreaWidth);
  titleLabel.setBounds(titleRowArea);
  labelFPS.setBounds(fpsArea);

  auto controlArea = area.removeFromTop(controlAreaHeight);

  auto buttonResetArea = controlArea.removeFromLeft(buttonWidth);
  buttonReset.setBounds(padVertical<int>(padHorizontal<int>(buttonResetArea, buttonPadWidth), buttonPadHeight));

  auto buttonToggleGraphArea = controlArea.removeFromLeft(buttonWidth);
  buttonToggleGraph.setBounds(padVertical<int>(padHorizontal<int>(buttonToggleGraphArea, buttonPadWidth), buttonPadHeight));

  auto labelArea = area.removeFromLeft(labelAreaWidth);
  labelArea.removeFromTop(labelAreaPadHeight);
  momentaryLabel.setBounds(labelArea.removeFromTop(labelAreaLabelHeight));
  labelArea.removeFromTop(labelValueIntervalHeight);
  momentaryValueLabel.setBounds(labelArea.removeFromTop(labelAreaValueHeight));
  labelArea.removeFromTop(labelIntervalHeight);

  shortTimeLabel.setBounds(labelArea.removeFromTop(labelAreaLabelHeight));
  labelArea.removeFromTop(labelValueIntervalHeight);
  shortTimeValueLabel.setBounds(labelArea.removeFromTop(labelAreaValueHeight));
  labelArea.removeFromTop(labelIntervalHeight);

  longTimeLabel.setBounds(labelArea.removeFromTop(labelAreaLabelHeight));
  labelArea.removeFromTop(labelValueIntervalHeight);
  longTimeValueLabel.setBounds(labelArea.removeFromTop(labelAreaValueHeight));
  labelArea.removeFromTop(labelIntervalHeight);

  trueMaxLabel.setBounds(labelArea.removeFromTop(labelAreaLabelHeight));
  labelArea.removeFromTop(labelValueIntervalHeight);
  trueMaxValueLabel.setBounds(labelArea.removeFromTop(labelAreaValueHeight));
  labelArea.removeFromTop(labelIntervalHeight);

  if (showGraph) {
    graph.setBounds(area);
  }
}

