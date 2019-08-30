/**********************************************************************************
*  MIT License                                                                    *
*                                                                                 *
*  Copyright (c) 2017 Vahan Aghajanyan <vahancho@gmail.com>                       *
*                                                                                 *
*  Permission is hereby granted, free of charge, to any person obtaining a copy   *
*  of this software and associated documentation files (the "Software"), to deal  *
*  in the Software without restriction, including without limitation the rights   *
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
*  copies of the Software, and to permit persons to whom the Software is          *
*  furnished to do so, subject to the following conditions:                       *
*                                                                                 *
*  The above copyright notice and this permission notice shall be included in all *
*  copies or substantial portions of the Software.                                *
*                                                                                 *
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
*  SOFTWARE.                                                                      *
***********************************************************************************/

#include <tuple>
#include <cinttypes>
#include <assert.h>
#include <cmath>


#include "polylineencoder.h"

static const double s_presision   = 100000.0f;
static const int    s_chunkSize   = 5;
static const int    s_asciiOffset = 63;
static const int    s_5bitMask    = 0x1f; // 0b11111 = 31
static const int    s_6bitMask    = 0x20; // 0b100000 = 32

void PolylineEncoder::addPoint(double latitude, double longitude)
{
    assert(latitude <= 90.0f && latitude >= -90.0f);
    assert(longitude <= 180.0f && longitude >= -180.0f);
    
    printf("added Point (%f,%f)\n",latitude, longitude);
    m_polyline.emplace_back(latitude, longitude);
}

std::string PolylineEncoder::encode() const
{
    return encode(m_polyline);
}

int PolylineEncoder::encode(double value, char *result)
{
    printf("encode initial value: %f \n", value);
    int32_t e5 = round(value * s_presision); // (2)

    printf("encode step 2: e5=%i \n",e5);
    e5 <<= 1;                                     // (4)

    printf("encode step 4: e5=%i \n",e5);
    if (value < 0) {
        e5 = ~e5;                                 // (5)
    }

    printf("encode step 5: e5=%i \n",e5);

    bool hasNextChunk = false;

    int r_len = 0; // length of result string so far

    // Split the value into 5-bit chunks and convert each of them to integer
    do {
        int32_t nextChunk = (e5 >> s_chunkSize); // (6), (7) - start from the left 5 bits.
        hasNextChunk = nextChunk > 0;

        int charVar = e5 & s_5bitMask;           // 5-bit mask (0b11111 == 31). Extract the left 5 bits.
        if (hasNextChunk) {
            charVar |= s_6bitMask;               // (8)
        }
        charVar += s_asciiOffset;                // (10)

        result[r_len++] = (char)charVar;         // (11)

        // Todo: check length!

        e5 = nextChunk;

    } while (hasNextChunk);

    result[r_len] = 0; // zero terminate string

    printf("encode result: %s \n", result);

    return r_len;
}

std::string PolylineEncoder::encode(const PolylineEncoder::Polyline &polyline)
{
    std::string result;

    // The first segment: offset from (.0, .0)
    double latPrev = .0;
    double lonPrev = .0;

    char c_polyline[POLYLINE_MAX_LENGTH+1];
    char c_result[POLYLINE_POINT_MAX_LENGTH+1];



    for (const auto &tuple : polyline)
    {
      const double lat = std::get<0>(tuple);
      const double lon = std::get<1>(tuple);

      // Offset from the previous point
      encode(lat - latPrev,c_result);      
      result.append(c_result);

      encode(lon - lonPrev,c_result);      
      result.append(c_result);

      latPrev = lat;
      lonPrev = lon;
    }

    return result;
}

float PolylineEncoder::decode(const std::string &coords, size_t &i)
{
    assert(i < coords.size());

    int32_t result = 0;
    int shift = 0;
    char c = 0;
    do {
        c = coords.at(i++);
        c -= s_asciiOffset;      // (10)
        result |= (c & s_5bitMask) << shift;
        shift += s_chunkSize;    // (7)
    } while (c >= s_6bitMask);

    printf("decode before step 5: result=%i \n",result);
    if (result & 1) {
        result = ~result;        // (5)
    }

    printf("decode before step 4: result=%i \n",result);
    result >>= 1;                // (4)

    printf("decode result=%f \n",result / s_presision);
    // Convert to decimal value.
    return result / s_presision; // (2)
}

PolylineEncoder::Polyline PolylineEncoder::decode(const std::string &coords)
{
    PolylineEncoder::Polyline polyline;

    size_t i = 0;
    while (i < coords.size())
    {
        auto lat = decode(coords, i);
        auto lon = decode(coords, i);

        if (!polyline.empty()) {
            const auto &prevPoint = polyline.back();
            lat += std::get<0>(prevPoint);
            lon += std::get<1>(prevPoint);
        }
        polyline.emplace_back(lat, lon);
    }

    return polyline;
}

const PolylineEncoder::Polyline &PolylineEncoder::polyline() const
{
    return m_polyline;
}

void PolylineEncoder::clear()
{
    m_polyline.clear();
}
