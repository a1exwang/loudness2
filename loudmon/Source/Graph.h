#pragma once

#include <JuceHeader.h>

class Graph : public juce::Component {
 public:
  void resized() override;

  void paint(Graphics &g) override;

  void addValue(double value);

 private:
  double maxLUFS = 10, minLUFS = -50;
  int columnWidth = 1;
  Colour backgroundColor = Colour(0), foregroundColor = Colour(0xff4488ccu);
  std::list<double> ringBuffer = std::list<double>(50, minLUFS);
};
