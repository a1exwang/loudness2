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
void Graph::resized() {
  int newSize = getWidth() / columnWidth;
  // make ringBuffer.size() == newSize, resize from front
  while (ringBuffer.size() > newSize) {
    ringBuffer.pop_front();
  }
  while (ringBuffer.size() < newSize) {
    ringBuffer.push_front(minLUFS);
  }
}
void Graph::addValue(double value) {
  ringBuffer.push_back(value);
  ringBuffer.pop_front();
}
