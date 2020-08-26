#pragma once

#include <JuceHeader.h>

class Graph : public juce::Component {
 public:
  void resized() override;
  void reset() {
    ringBuffer.clear();
  }

  void paint(Graphics &g) override;

  void addValue(std::chrono::high_resolution_clock::time_point time, double value, double _sampleRate);

  std::tuple<double, double> logicalToPixel(std::tuple<double, double> pos) {
    auto [x, y] = pos;
    // -maxDuration ~ 0
    int innerWidth = getWidth() - 2 * padWidth - (yAxisTextMarginWidth + yAxisTextWidth);
    int innerHeight = getHeight() - 2 * padHeight;
    return std::make_tuple(
      (x - (-maxDuration)) / maxDuration * innerWidth + padWidth + yAxisTextMarginWidth + yAxisTextWidth,
      (maxLUFS - y) / (maxLUFS - minLUFS) * innerHeight + padHeight
    );
  }

 private:
  int padWidth = 10;
  int padHeight = 36;
  int textHeight = 18;
  int yAxisTextWidth = 24;
  int yAxisTextMarginWidth = 5;
  int xAxisTextWidth = 40;
  double maxLUFS = 10, minLUFS = -60;
  double maxDuration = 10;
  double sampleRate = 48000;
  int columnWidth = 1;
  using TP = std::chrono::high_resolution_clock::time_point;
  Colour backgroundColor = Colour(0);
  Colour graphColour = Colour::fromHSV(0.3f, 0.7f, 0.52f, 0.5f);
  float graphLineWidth = 2;
  Colour lineColour = Colour::fromHSV(0.3, 0.9, 0.1f, 0.7f);
  std::list<std::tuple<TP, double>> ringBuffer;
};
