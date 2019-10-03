/**********************************************************************************
 *  MIT License                                                                    *
 *                                                                                 *
 *  Copyright (c) 2017 Vahan Aghajanyan <vahancho@gmail.com>                       *
 *  Copyright (c) 2019 Mark Uckermann <mark.uckermann@gmail.com>                   *
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

#include "polylineencoder.h"

namespace polyline
{

static const double precision = 100000.0f;
static const int chunk_size = 5;
static const int ascii_offset = 63;

static const int mask_5_bit = 0x1f; // 0b11111 = 31
static const int mask_6_bit = 0x20; // 0b100000 = 32

static const int point_max_length = 12;

size_t encode_point(Point* pt, char* result)
{
    size_t r_len = 0; // length of result string so far

    double* p_value = &(pt->lat);

    for (int deg = 0; deg < 2; ++deg) { // encode both latitude and longitude

        int32_t e5 = round(*p_value * precision); // (2)

        e5 <<= 1; // (4)

        if (*p_value < 0) {
            e5 = ~e5; // (5)
        }

        bool hasNextChunk = true;

        // Split the value into 5-bit chunks and convert each of them to integer
        while (hasNextChunk) {
            int32_t nextChunk = (e5 >> chunk_size); // (6), (7) - start from the left 5 bits.
            hasNextChunk = nextChunk > 0;

            int charVar = e5 & mask_5_bit; // 5-bit mask (0b11111 == 31). Extract the left 5 bits.
            if (hasNextChunk) {
                charVar |= mask_6_bit; // (8)
            }
            charVar += ascii_offset; // (10)

            result[r_len++] = (char)charVar; // (11)

            if (r_len > point_max_length) {
                // Error: Point too long. (This should never happen)
                return 0;
            }

            e5 = nextChunk;

        }

        p_value = &(pt->lon);
    }
    result[r_len] = 0; // zero terminate string

    return r_len;
}

size_t encode_line(Point* points, size_t num_points, char* coords, size_t len_coords)
{
    // The first segment: offset from (.0, .0)
    Point prev_pt = {0.0, 0.0};
    Point curr_pt;

    coords[0] = 0;      // make string size 0
    size_t res_len = 0; // length of result string at the moment

    size_t ret = 0; // number of points

    // buffer to store point result in
    char c_point[point_max_length + 1];

    for (size_t pt = 0; pt < num_points; pt++) {
        curr_pt.lat = points[pt].lat - prev_pt.lat;
        curr_pt.lon = points[pt].lon - prev_pt.lon;

        int len = encode_point(&curr_pt, c_point);
        if (len) {
            if (res_len + len >= len_coords) {// check to avoid overflows
            
                // Error: ran out of space
                return ret;
            }

            strcat(coords, c_point);
            res_len += len;
            ret++;
        }
        prev_pt.lat = points[pt].lat;
        prev_pt.lon = points[pt].lon;
    }

    return ret;
}

void Step_decoder::start() { state = waiting_for_first_point; }

int Step_decoder::step(char c, Point* point)
{
    switch (state) {
    case waiting_for_first_point:
        previous_point.lat = 0.0;
        previous_point.lon = 0.0;
        ++state;

    case waiting_for_first_lat_char:
    case waiting_for_first_lon_char:
        partial_result = 0;
        partial_shift = 0;
        ++state;
        break;
    default:
        break;
    }

    // do this for every new character
    c -= ascii_offset; // (10)
    partial_result |= (c & mask_5_bit) << partial_shift;
    partial_shift += chunk_size; // (7)

    if (c < mask_6_bit) {

        if (partial_result & 1) {
            partial_result = ~partial_result; // (5)
        }

        partial_result >>= 1; // (4)

        if (state == decoding_latitude) {
            point->lat = previous_point.lat + partial_result / precision; // (2)
            state = waiting_for_first_lon_char;

            return 1;
        } else if (state == decoding_longitude) {
            point->lon = previous_point.lon + partial_result / precision; // (2)
            state = waiting_for_first_lat_char;

            previous_point = *point;
            return 0;
        }
    }

    return 2;
}

size_t decode_line(const char* coords, Point* points, const size_t max_points)
{
    Step_decoder decoder;
    decoder.start(); // start decoding process

    size_t len_coords = strlen(coords);

    // keep track of both array lengths
    size_t num_points = 0;
    size_t ii = 0;

    Point decoded_point; // location for step result

    while (ii < len_coords && num_points < max_points) {

        // step() returns 0 for a succesful decode.
        if (!decoder.step(coords[ii], &decoded_point)) {

            points[num_points++] = decoded_point;
        }
        ii++; // move to the next character
    }

    return num_points;
}

} // namespace polyline