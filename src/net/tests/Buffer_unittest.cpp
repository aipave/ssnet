#include "Buffer.h"
#include "Logger.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <string.h>

using namespace ssnet;
using namespace std;

BOOST_AUTO_TEST_CASE(testReadableBytes)
        {
                Buffer buf;
        BOOST_CHECK_EQUAL(buf.readableBytes(), 0);

        string str(200, 'x');
        buf.append(str.c_str(), str.size());
        BOOST_CHECK_EQUAL(buf.readableBytes(), 200);

        buf.retrieve(50);
        BOOST_CHECK_EQUAL(buf.readableBytes(), str.size()-50);

        buf.append(str.c_str(), str.size());
        BOOST_CHECK_EQUAL(buf.readableBytes(), str.size()*2-50);
        }

BOOST_AUTO_TEST_CASE(testAppendReadPeek)
        {
                Buffer buf;
        uint8_t v8 = 233;
        uint16_t v16 = 65530;
        uint32_t v32 = 23123123;
        uint64_t v64 = 21341411231;
        buf.appendUint8(v8);
        buf.appendUint16(v16);
        buf.appendUint32(v32);
        buf.appendUint64(v64);
        BOOST_CHECK_EQUAL(buf.peekUint8(), v8);
        BOOST_CHECK_EQUAL(buf.readUint8(), v8);
        BOOST_CHECK_EQUAL(buf.peekUint16(), v16);
        BOOST_CHECK_EQUAL(buf.readUint16(), v16);
        BOOST_CHECK_EQUAL(buf.peekUint32(), v32);
        BOOST_CHECK_EQUAL(buf.readUint32(), v32);
        BOOST_CHECK_EQUAL(buf.peekUint64(), v64);
        BOOST_CHECK_EQUAL(buf.readUint64(), v64);
        }

