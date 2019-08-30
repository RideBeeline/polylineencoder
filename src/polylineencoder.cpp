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


int encodePoint( double lat, double lon, char *result)
{
    int r_len = 0; // length of result string so far

    double *p_value = &lat;

    for (int deg = 0; deg <2; ++deg)
    {
        printf("encode initial value: %f \n", *p_value);
        int32_t e5 = round(*p_value * s_presision); // (2)

        printf("encode step 2: e5=%i \n",e5);
        e5 <<= 1;                                     // (4)

        printf("encode step 4: e5=%i \n",e5);
        if (*p_value < 0) {
            e5 = ~e5;                                 // (5)
        }

        printf("encode step 5: e5=%i \n",e5);

        bool hasNextChunk = false;      

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

            if(r_len == POLYLINE_POINT_MAX_LENGTH) 
            {
                // Error: Point too long. (This should never happen)
                return 0;
            }

            e5 = nextChunk;

        } while (hasNextChunk);

        *p_value = lon;
    }
    result[r_len] = 0; // zero terminate string

    return r_len;
}

int encodeAll(Pointf *p_points, const size_t num_points, char *result, const size_t size_result)
{
    // The first segment: offset from (.0, .0)
    double latPrev = .0;
    double lonPrev = .0;

    result[0] = 0; // make string size 0
    size_t res_len = 0; // length of result string at the moment

    int ret = 0; // number of points

    // buffer to store point result in
    char c_point[POLYLINE_POINT_MAX_LENGTH+1];

    for(size_t pt = 0; pt<num_points; pt++)
    {
        int len = encodePoint(p_points[pt].lat - latPrev, p_points[pt].lon-lonPrev, c_point);  
        if( len ) 
        {
            if(res_len + len >= size_result )
            {   
                //Error: ran out of space
                printf("Error ran out of space\n");
                return ret;
            }

            printf("result 2points: %s\n",c_point);

            strcat(result, c_point);
            res_len += len;
            ret++;

        }
        latPrev = p_points[pt].lat;
        lonPrev = p_points[pt].lon;
    }

    return ret;
}

int PolylineEncoder::encode(double value, char *result)
{
    
    return 0;
}

std::string PolylineEncoder::encode(const PolylineEncoder::Polyline &polyline)
{
       

    char c_polyline[POLYLINE_MAX_LENGTH+1];

    
    size_t n_points = 0;

    Pointf points[20];



    for (const auto &tuple : polyline)
    {
      const double lat = std::get<0>(tuple);
      const double lon = std::get<1>(tuple);

      Pointf point = {(float)lat, (float)lon };

      points[n_points++] = point;


    }

    encodeAll(points, n_points, c_polyline, POLYLINE_MAX_LENGTH);

    return std::string(c_polyline);
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
