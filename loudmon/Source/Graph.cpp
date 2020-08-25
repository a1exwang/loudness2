#include "Graph.h"

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
void Graph::paint(Graphics &g) {
  // background
  g.fillAll(backgroundColor);
  Colour textColour = Colour::fromRGB(0xcc, 0xcc, 0xcc);

  // draw grid
  auto now = std::chrono::high_resolution_clock::now();
  int textWidth = 100;
  constexpr double displayValuesX[] = {-10, -7.5, -5, -2.5, 0};
  constexpr double displayValuesY[] = {-54, -45, -36, -27, -16, -9, -6, -3, 0, 3, 10};
  g.setFont(Font(fontHeight));
  for (auto displayValueX : displayValuesX) {
    auto [x, y] = logicalToPixel({displayValueX, minLUFS});

    g.setColour(textColour);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << displayValueX;
    g.drawText(ss.str(), x - textWidth/2, y + fontHeight/2, textWidth, fontHeight, Justification::centred);

    g.setColour(Colour::fromRGB(0x33,0x33,0x33));
    g.drawLine(x, 0, x, getHeight());
  }

  for (auto displayValueY : displayValuesY) {
    auto [x, y] = logicalToPixel({-maxDuration, displayValueY});

    g.setColour(textColour);
    g.drawText(std::to_string(int(displayValueY)), x - textWidth, y-fontHeight/2, textWidth, fontHeight, Justification::centredRight);

    g.setColour(Colour::fromRGB(0x33,0x33,0x33));
    g.drawLine(0, y, getWidth(), y);
  }

  int i = 0;
  double lastX = 0, lastY = 0;
  g.setColour(foregroundColor);
  for (auto it = ringBuffer.begin(); it != ringBuffer.end(); it++, i++) {
    auto [time, lufs] = *it;
    auto deltaT = std::chrono::duration<double>(time - now).count();
    auto [x, y] = logicalToPixel({clip(deltaT, -maxDuration, 0.0), clip(lufs, minLUFS, maxLUFS)});
    if (it != ringBuffer.begin()) {
      g.drawLine(lastX, lastY, x, y);
    }
    lastX = x;
    lastY = y;
  }
}
void Graph::resized() {}

void Graph::addValue(std::chrono::high_resolution_clock::time_point time, double value, double sampleRate) {
  ringBuffer.emplace_back(time, value);
  while (!ringBuffer.empty() && std::chrono::duration<double>(time - std::get<0>(ringBuffer.front())).count() > maxDuration) {
    ringBuffer.pop_front();
  }
  this->sampleRate = sampleRate;
  repaint();
}
