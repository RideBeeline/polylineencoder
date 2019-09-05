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

#include <cstring>
#include <cstdio>

#include "../src/polylineencoder.cpp"


using namespace Polyline;

static Point line[20];

// Test encoding of single point.
bool test1()
{
    Point pt = {38.5, -120.2};
    char result[16]; // 13 minimum
    encodePoint(&pt, result);

    if(strcmp(result, "_p~iF~ps|U") != 0)
    {
        printf("encodePoint() resulted in %s instead of %s. \n",result, "_p~iF~ps|U");
        return false;
    }
    
    return true;
}

// Helper function which encodes and decodes a set of points.
bool transcodeAllPoints(size_t num_points, const char * coords) {
    char result_str[40];

    size_t ret = encodeLine(line, num_points, result_str, sizeof(result_str));
    if( ret != num_points ) {
        printf("encodeLine() returned %u instead of %u. \n",(unsigned int)ret, (unsigned int)num_points);
        return false;
    }

    if ( strcmp(result_str, coords) != 0) {
        printf("encodeLine() resulted in %s instead of %s. \n",result_str, coords);
        return false;
    }

    Point result_line[40];
    ret = decodeLine(result_str, result_line, 40);
    if( ret != num_points ) {
        printf("decodeLine() returned %u instead of %u. \n",(unsigned int)ret, (unsigned int)num_points);
        return false;
    }

    // compare results
    for(unsigned int i=0; i<num_points; i++) {
        if( line[i].lat != result_line[i].lat) {
            printf("Latitude of point %u is %f, expected %f \n",i,result_line[i].lat,line[i].lat );
            return false;
        }
        if( line[i].lon != result_line[i].lon) {
            printf("Longitude of point %u is %f, expected %f \n",i,result_line[i].lon,line[i].lon );
            return false;
        }
    }


    return true;
}

// Points from Google's example.
bool test2()
{
    line[0] = {38.5, -120.2};
    line[1] = {40.7, -120.95};
    line[2] = {43.252, -126.453};

    return transcodeAllPoints(3,"_p~iF~ps|U_ulLnnqC_mqNvxq`@");
}

// Extreme points at poles.
bool test3()
{
    line[0] = {90, 180};
    line[1] = {0, 0};
    line[2] = {-90, -180};

    return transcodeAllPoints(3,"_cidP_gsia@~bidP~fsia@~bidP~fsia@");
}

// Just a single 0 0 point.
bool test4()
{
    line[0] = {0, 0};

    return transcodeAllPoints(1,"??");
}

// The result string is too short for the number of points.
bool test5()
{
    char result_str[15]; // too short, just enough space for one point

    line[0] = {90, 180};
    line[1] = {0, 0};
    line[2] = {-90, -180};

    size_t ret = encodeLine(line, 3, result_str, sizeof(result_str));
    if( ret != 1 ) {
        printf("encodeLine() returned %u instead of 1. \n",(unsigned int)ret);
        return false;
    }
    return true;
}

// The string is too long for the points Array.
bool test6()
{
    size_t ret = decodeLine("_p~iF~ps|U_ulLnnqC_mqNvxq`@", line, 1); // only give space for one point
    if( ret != 1 ) {
        printf("decodeLine() returned %u instead of 1. \n",(unsigned int)ret);
        return false;
    }
    return true;
}


int main() {
    printf("Running Polyline Encoder tests ... \n");

    if( !test1() ) printf("test1 failed\n");
    if( !test2() ) printf("test2 failed\n");
    if( !test3() ) printf("test3 failed\n");
    if( !test4() ) printf("test4 failed\n");
    if( !test5() ) printf("test5 failed\n");
    if( !test6() ) printf("test6 failed\n");

    printf("All tests ran.\n");
    return 0;
}