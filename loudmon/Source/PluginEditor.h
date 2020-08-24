#pragma once

#include <list>
#include <string>
#include <map>
#include <utility>

#include <JuceHeader.h>

template <typename T>
T clip(T value, T min, T max) {
  if (value > max) {
    return max;
  } else if (value < min) {
    return min;
  } else {
    return value;
  }
}

class Graph : public juce::Component {
 public:
  void resized() override {
    int newSize = getWidth() / columnWidth;
    // make ringBuffer.size() == newSize, resize from front
    while (ringBuffer.size() > newSize) {
      ringBuffer.pop_front();
    }
    while (ringBuffer.size() < newSize) {
      ringBuffer.push_front(minLUFS);
    }
  }

  void paint(Graphics &g) override {
    g.fillAll(backgroundColor);
    int columnHeight = getHeight();
    int i = 0;
    for (auto it = ringBuffer.begin(); it != ringBuffer.end(); it++, i++) {
      auto lufs = *it;
      double r = clip((lufs - minLUFS) / (maxLUFS - minLUFS), 0.0, 1.0);
      g.setColour(foregroundColor);
      g.fillRect(columnWidth * i, int(columnHeight * (1 - r)), columnWidth, columnHeight);
    }
  }

  void addValue(double value) {
    ringBuffer.push_back(value);
    ringBuffer.pop_front();
  }

  double maxLUFS = 10, minLUFS = -50;
  int columnWidth = 1;
  Colour backgroundColor = Colour(0), foregroundColor = Colour(0xff4488ccu);
  std::list<double> ringBuffer = std::list<double>(50, minLUFS);
};

class MainComponent : public juce::AudioProcessorEditor {
 public:
  explicit MainComponent(MainAudioProcessor& p);
  ~MainComponent() override;

  void onUpdateTimer() {
    auto lufs = p.getLUFS();
    std::stringstream ss;
    ss << "LUFS " << std::setprecision(3) << lufs << "dB";
    label.setText(ss.str(), dontSendNotification);
    graph.addValue(lufs);
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

