
#include <gtest/gtest.h>
#include <commkit/commkit.h>

TEST(BasicsTest, Init) {

    commkit::Node n;
    EXPECT_TRUE(n.init("tester"));

    // more here...
}
