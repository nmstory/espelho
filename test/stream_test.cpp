#include <gtest/gtest.h>
#include <stream.h>

TEST(StreamRoundtrip, SingleInt) {
    uint8_t buf[16] = {};
    WriteStream ws{buf, sizeof(buf)};
    int written = 12345;
    ASSERT_TRUE(process_pod(ws, written));

    ReadStream rs{buf, ws.pos};
    int read = 0;
    ASSERT_TRUE(process_pod(rs, read));
    EXPECT_EQ(written, read);
}
