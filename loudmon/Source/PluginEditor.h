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

  void onUpdateTimer();
  void resized() override {
    setMyBounds();
  }

  void setMyBounds();

  void paint(Graphics &g) override {
    AudioProcessorEditor::paint(g);

    frameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    if (now - lastFPSTime > maxFPSInterval) {
      double seconds = std::chrono::duration<double>(now - lastFPSTime).count();
      if (seconds != 0) {
        fps = frameCount / seconds;
      }
      lastFPSTime = now;
      frameCount = 0;
    }

  }
 protected:
  // styles
  float controlAreaHeight = 50;
  float buttonResetWidth = 60;
  float buttonResetPadHeight = 6;
  float buttonResetPadWidth = 3;

  float labelAreaWidth = 120;
  float labelTextHeight = 14;
  float labelIntervalHeight = 8;
  float labelAreaPadHeight = 30 + 60;
  float labelValueIntervalHeight = 2;
  float valueTextHeight = 32;
  float labelAreaLabelHeight = labelTextHeight + 2;
  float labelAreaValueHeight = valueTextHeight + 4;
  Colour momentaryTextColour = Colour::fromHSV(0.3f, 0.7f, 0.52f, 1);
  Colour shortTimeTextColour = Colour::fromHSV(0.3f, 0.7f, 0.52f, 1);
  Colour longTimeTextColour = Colour::fromHSV(0.3f, 0.7f, 0.52f, 1);
  Colour trueMaxTextColour = Colour::fromHSV(0.3f, 0.7f, 0.52f, 1);

  std::chrono::high_resolution_clock::time_point lastFPSTime = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds maxFPSInterval = std::chrono::milliseconds(300);
  size_t frameCount = 0;
  double fps = 0;

  TextButton buttonReset;

  Label labelFPS;

  Label momentaryLabel;
  Label momentaryValueLabel;
  Label shortTimeLabel;
  Label shortTimeValueLabel;
  Label longTimeLabel;
  Label longTimeValueLabel;
  Label trueMaxLabel;
  Label trueMaxValueLabel;

  Graph graph;
  std::unique_ptr<Timer> timer;
 private:
  MainAudioProcessor &p;
  //==============================================================================
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

