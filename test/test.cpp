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

#include <cstring>
#include <cstdio>

#include "../src/polylineencoder.cpp"


using namespace Polyline;

static Point line[20];

// test encoding of single point
bool test1()
{
    Point pt = {38.5, -120.2};
    char result[12]; // 11 minimum
    encodePoint(&pt, result);

    if(strcmp(result, "_p~iF~ps|U") != 0)
    {
        printf("encodePoint() resulted in %s instead of %s. \n",result, "_p~iF~ps|U");
        return false;
    }
    
    return true;
}


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

bool test2()
{
    line[0] = {38.5, -120.2};
    line[1] = {40.7, -120.95};
    line[2] = {43.252, -126.453};

    return transcodeAllPoints(3,"_p~iF~ps|U_ulLnnqC_mqNvxq`@");
}

bool test3()
{
    // line[0] = {180, 180};
    // line[1] = {0, 0};
    // line[2] = {180, 0};

    // return transcodeAllPoints(3,"_p~iF~ps|U_ulLnnqC_mqNvxq`@");
}



int main() {
    printf("Running Polyline Encoder tests ... \n");

    if( !test1() ) printf("test1 failed\n");
    if( !test2() ) printf("test2 failed\n");

    return 0;
}