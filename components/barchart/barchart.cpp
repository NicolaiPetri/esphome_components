#include "barchart.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/color.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <algorithm>
#include <sstream>
#include <iostream> // std::cout, std::fixed
#include <iomanip>
namespace esphome
{
  namespace barchart
  {

    using namespace display;

    static const char *const TAG = "barchart";
    static const char *const TAGL = "barchartlegend";

    static std::vector<float> parseDataString(std::string s)
    {
      auto del = ',';
      std::vector<float> result;
      std::stringstream ss(s);
      std::string word;
      while (!ss.eof())
      {
        getline(ss, word, del);
        result.insert(result.begin(), std::stof(word));
        //            result.push_back( std::stof(word));
        //            std::cout << word << std::endl;
      }
      return result;
    }

    void BarChartSeries::poll_data()
    {
      if (this->text_sensor_)
      {
        data_ = parseDataString(text_sensor_->get_state());
      }
    }
    void BarChartSeries::init(BarChart *g)
    {
      ESP_LOGI(TAG, "Init trace for sensor %s", this->get_name().c_str());
      /*  this->data_.init(g->get_width());
        if (sensor_) {
          sensor_->add_on_state_callback([this](float state) { this->data_.take_sample(state); });
        }
        this->data_.set_update_time_ms(g->get_duration() * 1000 / g->get_width());
        */
    }
    void BarChart::poll_data()
    {
      for (auto entry : traces_)
      {
        entry->poll_data();
      }
    }
    void BarChart::draw(DisplayBuffer *buff, uint16_t x_offset, uint16_t y_offset, Color color)
    {
      poll_data();
      innerX_ = 0;
      innerY_ = 0;
      this->innerWidth_ = this->width_;
      this->innerHeight_ = this->height_;
      //        ESP_LOGI(TAGL, "Drawing BarChart: %s", this->Name);
      /// Plot border
      if (this->border_ || true)
      {
        ESP_LOGI(TAGL, "drawing barchart border: %d, %d, %d, %d", x_offset, y_offset, this->width_, this->height_);
        buff->horizontal_line(x_offset, y_offset, this->width_, color);
        buff->horizontal_line(x_offset, y_offset + this->height_ - 1, this->width_, color);
        buff->vertical_line(x_offset, y_offset, this->height_, color);
        buff->vertical_line(x_offset + this->width_ - 1, y_offset, this->height_, color);
        innerX_++;
        innerWidth_ -= 2;
        innerY_++;
        innerHeight_ -= 2;
      }
#ifdef TESTING343
      if (this->font_label_)
      {
        buff->printf(x_offset + 10, y_offset + 10, this->font_label_, color, TextAlign::TOP_LEFT, "%s", "You are a legend");
        //        int fw, int fy,
        int fw, fxo, fbaseline, fheight;
        this->font_label_->measure("ABCDEFG", &fw, &fxo, &fbaseline, &fheight);
        ESP_LOGI(TAGL, " Font measure: %d, %d, %d, %d", fw, fxo, fbaseline, fheight);
        buff->printf(0, 0, this->font_label_, color, TextAlign::TOP_LEFT, "%s", "ABCDEFG");
      }
#endif // TESTING343
      ESP_LOGI(TAGL, "drawing %d traces", traces_.size());

      int YGridLines = 5;
      ESP_LOGI(TAGL, "OuterSizes = %d, %d", width_, height_);
      ESP_LOGI(TAGL, "InnerSizes = %d, %d", innerWidth_, innerHeight_);

      for (auto series : traces_)
      {
        if (series->data_.size() < 1)
        {
          ESP_LOGI(TAGL, "BarChart abort - no data in series");
          continue;
        }
        int bars = series->data_.size();
        int barSpacing = 2;
        int barWidth = ((innerWidth_ - barSpacing * 2) / bars) - barSpacing;
        int barMaxHeight = innerHeight_;

        if (YGridLines > 0)
        {
          float YGridSpacing = innerHeight_ / ((float)YGridLines + 1);
          for (int gl = 0; gl < YGridLines; gl++)
          {
            buff->horizontal_line(x_offset + innerX_, y_offset + (YGridSpacing * (gl + 1)) - 1, innerWidth_, color);
            //            SDL_RenderDrawLine(display, x_pos + innerX, y_pos + (YGridSpacing  * (gl+1)), x_pos + innerX + innerWidth, y_pos + (YGridSpacing  * (gl+1)));
          }
        }
        float maxValue = max_value_;
        auto itMax = std::max_element(std::begin(series->data_), std::end(series->data_));
        auto itMin = std::min_element(std::begin(series->data_), std::end(series->data_));
        calc_max_ = maxValue = *itMax;
        calc_min_ = min_value_ = *itMin;
        if (std::isnan(maxValue))
        {
          // No maxValue given - use calculated max from data series
          maxValue = calc_max_;
        }

        // Calculate and truncate scaleFactor
        float scaleFactor = maxValue / ((float)innerHeight_);
        //      scaleFactor = std::truncf(scaleFactor);
        ESP_LOGI(TAGL, "Calculating scaleFactor %d / %f = %f", innerHeight_, maxValue, scaleFactor);

        // Render bars
        for (int b = 0; b < bars; b++)
        {
          float scaledValue = series->data_[b] / scaleFactor;
          ESP_LOGI(TAGL, "drawing bar %d: val=%f, scaledVal=%f", b, series->data_[b], scaledValue);
          /*        SDL_Rect box;
                  box.x = (barSpacing / 2) + x_offset + innerX + (barSpacing * b) + (barWidth * b);
                  box.y = y_pos + innerY + innerHeight - scaledValue;
                  box.w = barWidth;
                  box.h = scaledValue;
                  SDL_RenderFillRects(display, &box, 1);
                  */
          ESP_LOGI(TAGL, " - bar sizes %d : %d : %d : %f",
                   (barSpacing) + x_offset + innerX_ + (barSpacing * b) + (barWidth * b),
                   (int)((float)(y_offset + innerY_ + innerHeight_) - scaledValue),
                   barWidth,
                   (int)scaledValue);
          buff->filled_rectangle(
              (barSpacing) + x_offset + innerX_ + (barSpacing * b) + (barWidth * b),
              y_offset + innerY_ + innerHeight_ - scaledValue,
              barWidth,
              scaledValue);
        }
      }
      ESP_LOGI(TAGL, "BarChart drawing completed");
      return;
#ifdef OLDSFSDF
      /// Determine best y-axis scale and range
      float ymin = NAN;
      float ymax = NAN;
      for (auto *trace : traces_)
      {
        /*    float mx = trace->get_tracedata()->get_recent_max();
            float mn = trace->get_tracedata()->get_recent_min();
            */
        float mx = 100;
        float mn = 0;
        if (std::isnan(ymax) || (ymax < mx))
          ymax = mx;
        if (std::isnan(ymin) || (ymin > mn))
          ymin = mn;
      }
      // Adjust if manually overridden
      if (!std::isnan(this->min_value_))
        ymin = this->min_value_;
      if (!std::isnan(this->max_value_))
        ymax = this->max_value_;

      float yrange = ymax - ymin;
      if (yrange > this->max_range_)
      {
        // Look back in trace data to best-fit into local range
        float mx = NAN;
        float mn = NAN;
        for (uint32_t i = 0; i < this->width_; i++)
        {
          for (auto *trace : traces_)
          {
            //        float v = trace->get_tracedata()->get_value(i);
            float v = 50;
            if (!std::isnan(v))
            {
              if ((v - mn) > this->max_range_)
                break;
              if ((mx - v) > this->max_range_)
                break;
              if (std::isnan(mx) || (v > mx))
                mx = v;
              if (std::isnan(mn) || (v < mn))
                mn = v;
            }
          }
        }
        yrange = this->max_range_;
        if (!std::isnan(mn))
        {
          ymin = mn;
          ymax = ymin + this->max_range_;
        }
        ESP_LOGV(TAG, "BarCharting at max_range. Using local min %f, max %f", mn, mx);
      }

      float y_per_div = this->min_range_;
      if (!std::isnan(this->gridspacing_y_))
      {
        y_per_div = this->gridspacing_y_;
      }
      // Restrict drawing too many gridlines
      if (yrange > 10 * y_per_div)
      {
        while (yrange > 10 * y_per_div)
        {
          y_per_div *= 2;
        }
        ESP_LOGW(TAG, "BarCharting reducing y-scale to prevent too many gridlines");
      }

      // Adjust limits to nice y_per_div boundaries
      int yn = int(ymin / y_per_div);
      int ym = int(ymax / y_per_div) + int(1 * (fmodf(ymax, y_per_div) != 0));
      ymin = yn * y_per_div;
      ymax = ym * y_per_div;
      yrange = ymax - ymin;

      /// Draw grid
      if (!std::isnan(this->gridspacing_y_))
      {
        for (int y = yn; y <= ym; y++)
        {
          int16_t py = (int16_t)roundf((this->height_ - 1) * (1.0 - (float)(y - yn) / (ym - yn)));
          for (uint32_t x = 0; x < this->width_; x += 2)
          {
            buff->draw_pixel_at(x_offset + x, y_offset + py, color);
          }
        }
      }
      if (!std::isnan(this->gridspacing_x_) && (this->gridspacing_x_ > 0))
      {
        int n = this->duration_ / this->gridspacing_x_;
        // Restrict drawing too many gridlines
        if (n > 20)
        {
          while (n > 20)
          {
            n /= 2;
          }
          ESP_LOGW(TAG, "BarCharting reducing x-scale to prevent too many gridlines");
        }
        for (int i = 0; i <= n; i++)
        {
          for (uint32_t y = 0; y < this->height_; y += 2)
          {
            buff->draw_pixel_at(x_offset + i * (this->width_ - 1) / n, y_offset + y, color);
          }
        }
      }

      /// Draw traces
      ESP_LOGV(TAG, "Updating barchart. ymin %f, ymax %f", ymin, ymax);
      /*
        for (auto *trace : traces_) {
          Color c = trace->get_line_color();
          uint16_t thick = trace->get_line_thickness();
          for (uint32_t i = 0; i < this->width_; i++) {
            float v = (trace->get_tracedata()->get_value(i) - ymin) / yrange;
            // ESP_LOGI(TAGL, " got value %f  for idx %d ", v, i);

            if (!std::isnan(v) && (thick > 0)) {
              int16_t x = this->width_ - 1 - i;
              uint8_t b = (i % (thick * 4)) / thick;
              if (((uint8_t) trace->get_line_type() & (1 << b)) == (1 << b)) {
                int16_t y = (int16_t) roundf((this->height_ - 1) * (1.0 - v)) - thick / 2;
                for (uint16_t t = 0; t < thick; t++) {
                  buff->draw_pixel_at(x_offset + x, y_offset + y + t, c);
                }
              }
            }
          }
        }
      */
#endif
    }

    /*    void BarChart::set_data(std::string valuesString)
        {
          data_ = parseDataString(valuesString);
        }
    */
    /// Determine the best coordinates of drawing text + lines
    void BarChartLegend::init(BarChart *g)
    {
#ifdef NOT_IMPLEMENTED
      parent_ = g;

      // Determine maximum expected text and value width / height
      int txtw = 0, txth = 0;
      int valw = 0, valh = 0;
      int lt = 0;
      for (auto *trace : g->traces_)
      {
        std::string txtstr = trace->get_name();
        int fw, fos, fbl, fh;
        this->font_label_->measure(txtstr.c_str(), &fw, &fos, &fbl, &fh);
        if (fw > txtw)
          txtw = fw;
        if (fh > txth)
          txth = fh;
        if (trace->get_line_thickness() > lt)
          lt = trace->get_line_thickness();
        ESP_LOGI(TAGL, "  %s %d %d", txtstr.c_str(), fw, fh);

        if (this->values_ != VALUE_POSITION_TYPE_NONE)
        {
          std::stringstream ss;
          ss << std::fixed << std::setprecision(trace->sensor_->get_accuracy_decimals()) << trace->sensor_->get_state();
          std::string valstr = ss.str();
          if (this->units_ && trace->sensor_)
          {
            valstr += trace->sensor_->get_unit_of_measurement();
          }
          this->font_value_->measure(valstr.c_str(), &fw, &fos, &fbl, &fh);
          if (fw > valw)
            valw = fw;
          if (fh > valh)
            valh = fh;
          ESP_LOGI(TAGL, "    %s %d %d", valstr.c_str(), fw, fh);
        }
      }
      // Add extra margin
      txtw *= 1.2;
      valw *= 1.2;

      uint8_t n = g->traces_.size();
      uint16_t w = this->width_;
      uint16_t h = this->height_;
      DirectionType dir = this->direction_;
      ValuePositionType valpos = this->values_;
      if (!this->font_value_)
      {
        valpos = VALUE_POSITION_TYPE_NONE;
      }
      // Line sample always goes below text for compactness
      this->yl_ = txth + (txth / 4) + lt / 2;

      if (dir == DIRECTION_TYPE_AUTO)
      {
        dir = DIRECTION_TYPE_HORIZONTAL; // as default
        if (h > 0)
        {
          dir = DIRECTION_TYPE_VERTICAL;
        }
      }

      if (valpos == VALUE_POSITION_TYPE_AUTO)
      {
        // TODO: do something smarter?? - fit to w and h?
        valpos = VALUE_POSITION_TYPE_BELOW;
      }

      if (valpos == VALUE_POSITION_TYPE_BELOW)
      {
        this->yv_ = txth + (txth / 4);
        if (this->lines_)
          this->yv_ += txth / 4 + lt;
      }
      else if (valpos == VALUE_POSITION_TYPE_BESIDE)
      {
        this->xv_ = (txtw + valw) / 2;
      }

      // If width or height is specified we divide evenly within, else we do tight-fit
      if (w == 0)
      {
        this->x0_ = txtw / 2;
        this->xs_ = txtw;
        if (valpos == VALUE_POSITION_TYPE_BELOW)
        {
          this->xs_ = std::max(txtw, valw);
          ;
          this->x0_ = this->xs_ / 2;
        }
        else if (valpos == VALUE_POSITION_TYPE_BESIDE)
        {
          this->xs_ = txtw + valw;
        }
        if (dir == DIRECTION_TYPE_VERTICAL)
        {
          this->width_ = this->xs_;
        }
        else
        {
          this->width_ = this->xs_ * n;
        }
      }
      else
      {
        this->xs_ = w / n;
        this->x0_ = this->xs_ / 2;
      }

      if (h == 0)
      {
        this->ys_ = txth;
        if (valpos == VALUE_POSITION_TYPE_BELOW)
        {
          this->ys_ = txth + txth / 2 + valh;
          if (this->lines_)
          {
            this->ys_ += lt;
          }
        }
        else if (valpos == VALUE_POSITION_TYPE_BESIDE)
        {
          if (this->lines_)
          {
            this->ys_ = std::max(txth + txth / 4 + lt + txth / 4, valh + valh / 4);
          }
          else
          {
            this->ys_ = std::max(txth + txth / 4, valh + valh / 4);
          }
          this->height_ = this->ys_ * n;
        }
        if (dir == DIRECTION_TYPE_HORIZONTAL)
        {
          this->height_ = this->ys_;
        }
        else
        {
          this->height_ = this->ys_ * n;
        }
      }
      else
      {
        this->ys_ = h / n;
      }

      if (dir == DIRECTION_TYPE_HORIZONTAL)
      {
        this->ys_ = 0;
      }
      else
      {
        this->xs_ = 0;
      }
#endif // NOT_IMPLEMETED
    }

    void BarChart::draw_legend(display::DisplayBuffer *buff, uint16_t x_offset, uint16_t y_offset, Color color)
    {
      if (!legend_)
        return;

      /// Plot border
      if (this->border_)
      {
        int w = legend_->width_;
        int h = legend_->height_;
        buff->horizontal_line(x_offset, y_offset, w, color);
        buff->horizontal_line(x_offset, y_offset + h - 1, w, color);
        buff->vertical_line(x_offset, y_offset, h, color);
        buff->vertical_line(x_offset + w - 1, y_offset, h, color);
      }
#ifdef NOT_IMPLEMENTED
      int x = x_offset + legend_->x0_;
      int y = y_offset;
      for (auto *trace : traces_)
      {
        std::string txtstr = trace->get_name();
        ESP_LOGV(TAG, "  %s", txtstr.c_str());

        //    buff->printf(x, y, legend_->font_label_, trace->get_line_color(), TextAlign::TOP_CENTER, "%s", txtstr.c_str());

        if (legend_->lines_)
        {
          uint16_t thick = trace->get_line_thickness();
          for (int i = 0; i < legend_->x0_ * 4 / 3; i++)
          {
            uint8_t b = (i % (thick * 4)) / thick;
            /*        if (((uint8_t) trace->get_line_type() & (1 << b)) == (1 << b)) {
                      buff->vertical_line(x - legend_->x0_ * 2 / 3 + i, y + legend_->yl_ - thick / 2, thick,
                                          trace->get_line_color());
                    }
                    */
          }
        }
        if (legend_->values_ != VALUE_POSITION_TYPE_NONE)
        {
          int xv = x + legend_->xv_;
          int yv = y + legend_->yv_;
          std::stringstream ss;
          ss << std::fixed << std::setprecision(trace->sensor_->get_accuracy_decimals()) << trace->sensor_->get_state();
          std::string valstr = ss.str();
          if (legend_->units_)
          {
            valstr += trace->sensor_->get_unit_of_measurement();
          }
          //      buff->printf(xv, yv, legend_->font_value_, trace->get_line_color(), TextAlign::TOP_CENTER, "%s", valstr.c_str());
          ESP_LOGV(TAG, "    value: %s", valstr.c_str());
        }
        x += legend_->xs_;
        y += legend_->ys_;
      }
#endif
    }

    void BarChart::setup()
    {
      for (auto *trace : traces_)
      {
        trace->init(this);
      }
    }

    void BarChart::dump_config()
    {
      for (auto *trace : traces_)
      {
        ESP_LOGCONFIG(TAG, "BarChart for sensor %s", trace->get_name().c_str());
      }
    }

  } // namespace barchart
} // namespace esphome
