#pragma once
#include <cstdint>
#include <utility>
#include <vector>
//#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/color.h"
#include "esphome/core/component.h"

namespace esphome {

// forward declare DisplayBuffer
namespace display {
class DisplayBuffer;
class Font;
}  // namespace display

namespace barchart {

class BarChart;

// const Color COLOR_ON(255, 255, 255, 255);

enum DirectionType {
  DIRECTION_TYPE_AUTO,
  DIRECTION_TYPE_HORIZONTAL,
  DIRECTION_TYPE_VERTICAL,
};

enum ValuePositionType {
  VALUE_POSITION_TYPE_NONE,
  VALUE_POSITION_TYPE_AUTO,
  VALUE_POSITION_TYPE_BESIDE,
  VALUE_POSITION_TYPE_BELOW
};

class BarChartLegend {
 public:
  void init(BarChart *g);
  void set_name_font(display::Font *font) { this->font_label_ = font; }
  void set_value_font(display::Font *font) { this->font_value_ = font; }
  void set_width(uint32_t width) { this->width_ = width; }
  void set_height(uint32_t height) { this->height_ = height; }
  void set_border(bool val) { this->border_ = val; }
  void set_lines(bool val) { this->lines_ = val; }
  void set_values(ValuePositionType val) { this->values_ = val; }
  void set_units(bool val) { this->units_ = val; }
  void set_direction(DirectionType val) { this->direction_ = val; }

 protected:
  uint32_t width_{0};
  uint32_t height_{0};
  bool border_{true};
  bool lines_{true};
  ValuePositionType values_{VALUE_POSITION_TYPE_AUTO};
  bool units_{true};
  DirectionType direction_{DIRECTION_TYPE_AUTO};
  display::Font *font_label_{nullptr};
  display::Font *font_value_{nullptr};
  // Calculated values
  BarChart *parent_{nullptr};
  //                      (x0)          (xs,ys)         (xs,ys)
  // <x_offset,y_offset> ------> LABEL1 -------> LABEL2 -------> ...
  //                                | \(xv,yv)        \ .
  //                                |  \               \-> VALUE1+units
  //                          (0,yl)|   \-> VALUE1+units
  //                                v     (top_center)
  //                            LINE_SAMPLE
  int x0_{0};  // X-offset to centre of label text
  int xs_{0};  // X spacing between labels
  int ys_{0};  // Y spacing between labels
  int yl_{0};  // Y spacing from label to line sample
  int xv_{0};  // X distance between label to value text
  int yv_{0};  // Y distance between label to value text
  friend BarChart;
};

class BarChartSeries;


class BarChartSeries {
 public:
  void init(BarChart *g);
  void set_name(std::string name) { name_ = std::move(name); }
//  void set_sensor(sensor::Sensor *sensor) { sensor_ = sensor; }
  void set_sensor(text_sensor::TextSensor *sensor) { text_sensor_ = sensor; }
  uint8_t get_line_thickness() { return this->line_thickness_; }
  void set_line_thickness(uint8_t val) { this->line_thickness_ = val; }
  std::string get_name() { return name_; }
  void poll_data();
 protected:
  std::vector<float> data_;
//  sensor::Sensor *sensor_{nullptr};
  text_sensor::TextSensor *text_sensor_{nullptr};
  std::string name_{""};
  uint8_t line_thickness_{3};

  friend BarChart;
  friend BarChartLegend;
};

class BarChart : public Component {
 public:
  void draw(display::DisplayBuffer *buff, uint16_t x_offset, uint16_t y_offset, Color color);
  void draw_legend(display::DisplayBuffer *buff, uint16_t x_offset, uint16_t y_offset, Color color);

  void setup() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }
  void dump_config() override;

//  void set_data(std::string valuesString);
  void set_duration(uint32_t duration) { duration_ = duration; }
  void set_width(uint32_t width) { width_ = width; }
  void set_height(uint32_t height) { height_ = height; }
  void set_min_value(float val) { this->min_value_ = val; }
  void set_max_value(float val) { this->max_value_ = val; }
  void set_min_range(float val) { this->min_range_ = val; }
  void set_max_range(float val) { this->max_range_ = val; }
  void set_grid_x(float val) { this->gridspacing_x_ = val; }
  void set_grid_y(float val) { this->gridspacing_y_ = val; }
  void set_border(bool val) { this->border_ = val; }
  void add_trace(BarChartSeries *trace) { traces_.push_back(trace); }
  void set_name_font(display::Font *font) { this->font_label_ = font; }
  void set_value_font(display::Font *font) { this->font_value_ = font; }
  void poll_data();
//  std::vector<BarChartSeries*>& get_series() { return traces_}
  BarChartSeries* get_trace(int idx) { return traces_[idx]; }
  void add_legend(BarChartLegend *legend) {
    this->legend_ = legend;
    legend->init(this);
  }
  uint32_t get_duration() { return duration_; }
  uint32_t get_width() { return width_; }
  uint32_t get_height() { return height_; }
  float get_calculated_max() { return calc_max_; }
  float get_calculated_min() { return calc_min_; }

 protected:
  uint32_t duration_;  /// in seconds
  uint32_t width_;     /// in pixels
  uint32_t height_;    /// in pixels
  uint32_t innerHeight_;
  uint32_t innerWidth_;
  uint32_t innerX_;
  uint32_t innerY_;
  float min_value_{NAN};
  float max_value_{NAN};
  float calc_max_{NAN};
  float calc_min_{NAN};
  float min_range_{1.0};
  float max_range_{NAN};
  float gridspacing_x_{NAN};
  float gridspacing_y_{NAN};
  bool border_{true};
//  std::vector<float> data_;
  std::vector<BarChartSeries *> traces_;
  BarChartLegend *legend_{nullptr};
  display::Font *font_label_{nullptr};
  display::Font *font_value_{nullptr};

  friend BarChartLegend;
};

}  // namespace barchart
}  // namespace esphome
