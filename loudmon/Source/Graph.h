#pragma once

#include <JuceHeader.h>

class Graph : public juce::Component {
 public:
  void resized() override;

  void paint(Graphics &g) override;

  void addValue(std::chrono::high_resolution_clock::time_point time, double value, double sampleRate);

  std::tuple<double, double> logicalToPixel(std::tuple<double, double> pos) {
    auto [x, y] = pos;
    // -maxDuration ~ 0
    int innerWidth = getWidth() - 2 * padWidth;
    int innerHeight = getHeight() - 2 * padHeight;
    return std::make_tuple(
      (x - (-maxDuration)) / maxDuration * innerWidth + padWidth,
      (maxLUFS - y) / (maxLUFS - minLUFS) * innerHeight + padHeight
    );
  }

 private:
  int padWidth = 20;
  int padHeight = 36;
  int fontHeight = 16;
  double maxLUFS = 10, minLUFS = -60;
  double maxDuration = 10;
  double sampleRate = 48000;
  int columnWidth = 1;
  using TP = std::chrono::high_resolution_clock::time_point;
  Colour backgroundColor = Colour(0), foregroundColor = Colour(0xff4488ccu);
  std::list<std::tuple<TP, double>> ringBuffer;
};
