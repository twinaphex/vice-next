#ifndef __AUDIO_QUAD_RESAMPLER_HPP
#define __AUDIO_QUAD_RESAMPLER_HPP

#include "resampler.hpp"
#include <deque>
#include <stdint.h>

namespace Audio {

class QuadraticResampler : public Resampler
{
   public:
      template <class T>
      QuadraticResampler(T& drain_obj, double in_ratio, unsigned in_channels) : Resampler(drain_obj), ratio(in_ratio), 
         channels(in_channels), sum_output_samples(0), sum_input_samples(0) {}

      ssize_t pull(float *out, size_t samples);
   private:
      unsigned channels;
      uint64_t sum_output_samples;
      uint64_t sum_input_samples;
      double ratio;
      std::deque<float> data;

      inline size_t required_samples(size_t samples);
      inline void poly_create_3(float *poly, float *y);
      size_t process(size_t samples, float *out_data);
};

}

#endif
