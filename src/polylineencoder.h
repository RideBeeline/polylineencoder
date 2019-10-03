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
#ifndef POLYLINEENCODER_H
#define POLYLINEENCODER_H

#include <cmath>   // uses round()
#include <cstdint> // for the uint32_t definition
#include <cstring> // uses strcat()

//! Implements Google's polyline compression algorithm.
/*!
  For more details refer to the algorithm definition at
  https://developers.google.com/maps/documentation/utilities/polylinealgorithm
*/
namespace polyline
{

struct Point {
    double lat; // latitude in degrees
    double lon; // longitude in degrees
};

/** Encodes a single point.
 * Takes a Point structure and a pointer to a c-style string for the result.
 * The result string must have space for 10 characters + null character.
 * Returns the length of the encoded result string.
 */
size_t encode_point(Point* pt, char* result);

/** Encodes a polyline
 * Takes a pointer to a Point structure array of size num_points
 * and a c-style string with a max length of len_coords for the result.
 * Returns the number of points encoded.
 * Check that num_points equals the result for successful encoding of all points.
 */
size_t encode_line(Point* points, size_t num_points, char* coords, size_t len_coords);

/** Decodes a polyline
 * Takes a c-style string and a Point array
 * for the result.
 * Returns the number of points decoded.
 * If the return equals max_points, the decoding might have ended
 * prematurely.
 */
size_t decode_line(const char* coords, Point* points, const size_t max_points);

class Step_decoder
{
  public:
    /** Resets the decoding state machine.
     * Has to be called before starting the decoding of a new
     * polyline string.
     */
    void start();

    /** A single step (one character) of the decoding process.
     * Given the next character and a pointer to a Point,
     * this function will attempt to fill the Point.
     * Returns:
     *      0 if the full point has been decoded
     *      1 if the latitude has been decoded
     *      2 if the decoding process is unfinshed
     * Call this function repeatedly while checking the return code.
     */
    int step(char c, Point* point);

  private:
    // States for the decoding state machine.
    enum Decoding_state {
        waiting_for_first_point,
        waiting_for_first_lat_char,
        decoding_latitude,
        waiting_for_first_lon_char,
        decoding_longitude,
    };
    int state = 0;

    int32_t partial_result = 0;
    int partial_shift = 0; // partial shift as used by the decoding algorithm
    Point previous_point = {0, 0};
};

} // namespace polyline

#endif // POLYLINEENCODER_H
