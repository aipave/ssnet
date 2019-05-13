
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <string>

#include "EndPoint.h"

using namespace ssnet;
using std::string;

BOOST_AUTO_TEST_CASE(testEndPoint)
        {
                EndPoint addr(1234);
        BOOST_CHECK_EQUAL(addr.toIpString(), string("0.0.0.0"));
        BOOST_CHECK_EQUAL(addr.toIpPortString(), string("0.0.0.0:1234"));


        EndPoint addr2("1.2.3.4", 8888);
        BOOST_CHECK_EQUAL(addr2.toIpString(), string("1.2.3.4"));
        BOOST_CHECK_EQUAL(addr2.toIpPortString(), string("1.2.3.4:8888"));

        EndPoint addr3("255.254.253.252", 65535);
        BOOST_CHECK_EQUAL(addr3.toIpString(), string("255.254.253.252"));
        BOOST_CHECK_EQUAL(addr3.toIpPortString(), string("255.254.253.252:65535"));
        }


