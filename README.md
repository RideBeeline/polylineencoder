# Polyline Encoder/Decoder for Embedded Systems
C-Style C++ implementation of [Google Encoded Polyline Algorithm Format](https://developers.google.com/maps/documentation/utilities/polylinealgorithm) forked from [PolylineEncoder](https://github.com/vahancho/polylineencoder)

## Installation

No installation required. Just compile *polylineencoder.h(.cpp)* in your project and use `Polyline` namespaced functions and the `StepDecoder` Class.

## Prerequisites

No special requirements except a *C++* compliant compiler. The class is tested with *g++ 7.4.0*.

## C-Style

This library is designed to run on embedded systems and does not use resource intensive C++ functionality or the Standard Library. The class encapsulation is useful if the state of several `StepEncoders` should be preserved. However, it would be trivial to port it to pure C.

## Streams
The library can be used to en/decode polylines one point or character at a time. This allows stream-like implementation for on-the-fly encoding / decoding and minimal memory consumption.

## Usage Example:

```cpp
char line[50];              // Big enough for result.
Polyline::Point points[3];  // Array of points forms a line.

points[0] = {38.5, -120.2};   // Add points.
points[1] = {40.7, -120.95};
points[2] = {43.252, -126.453};

size_t ret; // Save return value for overrun error checking.

ret = Polyline::encodeLine(points, 3, line, sizeof(line)); // encode
if( ret != 3 ) { ... } // check for sucessful encoding

// line now contains "_cidP_gsia@~bidP~fsia@~bidP~fsia@" (null terminated) 

ret = Polyline::decodeLine(line, points, 3) //decode
if( ret != 3 ) { ... } // check for sucessful decoding

// points contains original coordinates again.
```
For an example of the `StepDecoder` class, look at the `decodeLine()` function which uses the class internally. Similarly, the `encodeLine()` function can be used as a reference for using the `encodePoint()` function.

## Test

There are unit tests provided in the *test/* directory.
To run them you have to build and run the test application. For doing that you must invoke the following commands from the terminal, assuming that compiler and environment are already configured:

##### Linux (gcc)
```
cd test
g++ -std=c++11 test.cpp -o test.out
./test.out
```

## See Also

* [Encoded Polyline Algorithm](https://developers.google.com/maps/documentation/utilities/polylinealgorithm)

