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
  Colour gridColour = Colour::fromRGB(0x44, 0x44, 0x44);
  Colour coordinateColour = Colour::fromRGB(0xcc, 0xcc, 0xcc);

  // draw grid
  auto now = std::chrono::high_resolution_clock::now();
  constexpr double displayValuesX[] = {-10, -7.5, -5, -2.5, 0};
  constexpr double displayValuesY[] = {-54, -45, -36, -27, -18, -9, -6, -3, 0, 3};

  auto [xMin, yMin] = logicalToPixel({-maxDuration, maxLUFS});
  auto [xMax, yMax] = logicalToPixel({0, minLUFS});

  g.setFont(Font(static_cast<float>(textHeight)));
  for (auto displayValueX : displayValuesX) {
    auto [x, y] = logicalToPixel({displayValueX, minLUFS});

    g.setColour(textColour);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << displayValueX;
    g.drawText(ss.str(), static_cast<int>(x - xAxisTextWidth/2.0),
               static_cast<int>(y + textHeight/2.0), xAxisTextWidth, textHeight, Justification::centred);

    g.setColour(gridColour);
    g.drawLine(x, yMin, x, yMax);
  }

  for (auto displayValueY : displayValuesY) {
    auto [x, y] = logicalToPixel({-maxDuration, displayValueY});

    g.setColour(textColour);
    g.drawText(std::to_string(int(displayValueY)),
               static_cast<int>(x - yAxisTextWidth - yAxisTextMarginWidth), static_cast<int>(y-textHeight/2.0),
               yAxisTextWidth, textHeight, Justification::centredRight);

    g.setColour(gridColour);
    g.drawLine(xMin, y, xMax, y);
  }

  // draw the loudness graph
  auto graphArea = Path();
  g.setColour(lineColour);
  float lastX = 0, lastY = 0;
  for (auto it = ringBuffer.begin(); it != ringBuffer.end(); it++) {
    auto [time, lufs] = *it;
    auto deltaT = std::chrono::duration<double>(time - now).count();
    auto [x, y] = logicalToPixel({clip(deltaT, -maxDuration, 0.0), clip(lufs, minLUFS, maxLUFS)});
    if (it == ringBuffer.begin()) {
      graphArea.startNewSubPath(x, yMax);
      g.drawLine(x, y, x, yMax, graphLineWidth);
    } else {
      g.drawLine(lastX, lastY, x, y, graphLineWidth);
    }

    lastX = x;
    lastY = y;

    graphArea.lineTo(x, y);
    if (it == std::prev(ringBuffer.end())) {
      graphArea.lineTo(x, yMax);
    }
  }
  graphArea.closeSubPath();
  FillType fillType;
  fillType.setColour(graphColour);
  g.setFillType(fillType);
  g.fillPath(graphArea);


  // draw edge grid. edge grid should be on top of the graph
  g.setColour(coordinateColour);
  g.drawLine(xMin, yMin, xMin, yMax);
  g.drawLine(xMax, yMin, xMax, yMax);
  g.drawLine(xMin, yMin, xMax, yMin);
  g.drawLine(xMin, yMax, xMax, yMax);
}

void Graph::resized() {}

void Graph::addValue(std::chrono::high_resolution_clock::time_point time, double value, double _sampleRate) {
  ringBuffer.emplace_back(time, value);
  while (!ringBuffer.empty() &&
      std::chrono::duration<double>(time - std::get<0>(ringBuffer.front())).count() > maxDuration) {
    ringBuffer.pop_front();
  }
  this->sampleRate = _sampleRate;
  repaint();
}
