// Copyright (c) 2022. Vetle Wegner Ingeberg/University of Oslo.
// All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/* -*- c++ -*-
 * Histogram2D.h
 *
 *  Created on: 10.03.2010
 *      Author: Alexander Bürger
 */

#ifndef HISTOGRAM2D_H_
#define HISTOGRAM2D_H_

#include <histogram/Histograms.h>
#include <vector>

#define USE_ROWS 1
//#define H2D_USE_BUFFER 1

//! A two-dimensional histogram.
class Histogram2D : public Named {
public:
  //! The type used to count in each bin.
  typedef size_t data_t;

  struct buf_t {
      Axis::bin_t x, y;
      data_t w;
      buf_t(Axis::bin_t xx, Axis::bin_t yy, data_t ww) : x(xx), y(yy), w(ww) { }

      template<typename V>
      buf_t(std::initializer_list<V> coord, data_t ww) : x( *( coord.begin() ) ), y( *(coord.begin() + 1) ), w( ww ){ }
  };
  typedef std::vector<buf_t> buffer_t;

  //! Construct a 2D histogram.
  Histogram2D( const std::string& name,   /*!< The name of the new histogram. */
               const std::string& title,  /*!< The title of teh new histogram. */
               Axis::index_t xchannels,   /*!< The number of regular bins on the x axis. */
               Axis::bin_t xleft,         /*!< The lower edge of the lowest bin on the x axis. */
               Axis::bin_t xright,        /*!< The upper edge of the highest bin on the x axis. */
               const std::string& xtitle, /*!< The title of the x axis. */
               Axis::index_t ychannels,   /*!< The number of regular bins on the y axis. */
               Axis::bin_t yleft,         /*!< The lower edge of the lowest bin on the y axis. */
               Axis::bin_t yright,        /*!< The upper edge of the highest bin on the y axis. */
               const std::string& ytitle, /*!< The title of the y axis. */
               const std::string& path="" /*!< Path if in directories within root file */);

  //! Deallocate memory.
  ~Histogram2D();

  /*!
     * Sum two histograms together.
     * Adds the counts of histogram `other` to the current
     * histogram weighted by `scale`.
     * Throws if the binning of the two are different.
     */
  void Add(const Histogram2Dp &other, data_t scale = 1.0);

  //! Increment a histogram bin.
  void Fill(Axis::bin_t x,  /*!< The x axis value. */
            Axis::bin_t y,  /*!< The y axis value. */
            data_t weight=1 /*!< How much to add to the corresponding bin content. */)
  {
#ifdef H2D_USE_BUFFER
    buffer.push_back(buf_t(x, y, weight)); if( buffer.size()>=buffer_max ) FlushBuffer();
#else
    FillDirect(x, y, weight);
#endif /* H2D_USE_BUFFER */
  }

  //! Get the contents of a bin.
  /*! \return The bin content.
   */
  data_t GetBinContent(Axis::index_t xbin /*!< The x bin to look at. */,
                       Axis::index_t ybin /*!< The y bin to look at. */);

  //! Set the contents of a bin.
  void SetBinContent(Axis::index_t xbin /*!< The x bin to look at.   */,
                     Axis::index_t ybin /*!< The y bin to look at.   */,
                     data_t c           /*!< The bin content.        */);

  //! Get the x axis of the histogram.
  /*! \return The histogram's x axis.
   */
  [[nodiscard]] const Axis& GetAxisX() const
  { return xaxis; }

  //! Get the y axis of the histogram.
  /*! \return The histogram's y axis.
   */
  [[nodiscard]] const Axis& GetAxisY() const
  { return yaxis; }

  //! Get the number of entries in the histogram.
  /*! \return The histogram's entry count.
   */
  [[nodiscard]] int GetEntries() const
  { return entries; }

  //! Clear all bins of the histogram.
  void Reset();

  //! Directly increment the histogram. Inlined for optimal performance.
  inline void FillDirect(const buf_t &element)
  {
      Axis::index_t xbin = xaxis.FindBin( element.x );
      Axis::index_t ybin = yaxis.FindBin( element.y );
#ifndef USE_ROWS
      data[xaxis.GetBinCountAll()*ybin + xbin] += element.w;
#else
      rows[ybin][xbin] += element.w;
#endif
      entries += 1;
  }

private:
  //! Increment a histogram bin directly, bypassing the buffer.
  void FillDirect(Axis::bin_t x,  /*!< The x axis value. */
                  Axis::bin_t y,  /*!< The y axis value. */
                  data_t weight=1 /*!< How much to add to the corresponding bin content. */);

#ifdef H2D_USE_BUFFER
  //! Flush the data buffer.
  void FlushBuffer();
#endif /* H2D_USE_BUFFER */

  //! The x axis of the histogram;
  const Axis xaxis;

  //! The y axis of the histogram;
  const Axis yaxis;

  //! The number of entries in the histogram.
  size_t entries;

#ifndef USE_ROWS
  //! The bin contents, including the overflow bins.
  data_t *data;
#else
  data_t **rows;
#endif

#ifdef H2D_USE_BUFFER
  buffer_t buffer;
  static const unsigned int buffer_max = 4096;
#endif /* H2D_USE_BUFFER */
};

#endif /* HISTOGRAM2D_H_ */