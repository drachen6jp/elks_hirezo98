#include "testlib.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

TEST_CASE(stdio_init)
{
    char sector[512];
    for (size_t i = 0; i < sizeof(sector); i++)
        sector[i] = i & 0xff;

    FILE* fp = fopen("/dev/null", "w");
    if (!fp) {
        perror("/dev/null");
        return;
    }

    struct timeval start, end, diff;
    gettimeofday(&start, NULL);

    /* Every fread/fwrite ensures atexit handlers are registered,
     * so that all buffered file handles can be flushed on exit,
     * but if this registration check is naive it could impact throughput.
     */
    const long iter = 2L * 1024;
    for (long i = 0; i < iter; ++i) {
        size_t n = fwrite(sector, sizeof(sector), 1, fp);
        ASSERT_SYS(n, 1, errno);
    }

    gettimeofday(&end, NULL);
    testlib_tvSub(&end, &start, &diff);

    long bytes = sizeof(sector) * iter;
    struct timeval per;
    per.tv_sec = diff.tv_sec / bytes;
    per.tv_usec = diff.tv_sec % bytes * 1000000L + diff.tv_usec / bytes;
    testlib_tvNormalize(&per);

    TEST_INFO("fwrite: %ld bytes, %ld usec\n", bytes,
            diff.tv_sec * 1000000L + diff.tv_usec);

    fclose(fp);
}

TEST_CASE(stdio_seek) {
    char wbuf[256];
    char rbuf[256];
    size_t n;
    FILE* fp;
    int i;

    fp = fopen("/tmp/test-libc.txt", "w+");
    ASSERT_EQ(!fp, 0);

    /* beginning? */
    n = ftell(fp);
    ASSERT_EQ(n, 0);
    n = fseek(fp, 0, SEEK_SET);
    ASSERT_EQ(n, 0);
    n = ftell(fp);
    ASSERT_EQ(n, 0);
    n = fseek(fp, 0, SEEK_END);
    ASSERT_EQ(n, 0);
    n = ftell(fp);
    ASSERT_EQ(n, 0);

    /* random-access write */
    for (i = 0; i < sizeof(wbuf); ++i)
        wbuf[i] = i;
    for (i = sizeof(wbuf) - 1; i >= 0; --i) {
        n = fseek(fp, i, SEEK_SET);
        ASSERT_EQ(n, 0);
        n = ftell(fp);
        ASSERT_EQ(n, i);
        n = fwrite(wbuf + i, 1, 1, fp);
        ASSERT_EQ(n, 1);
        n = ftell(fp);
        ASSERT_EQ(n, i + 1);
    }

    /* rewind */
    n = fseek(fp, 0, SEEK_END);
    ASSERT_EQ(n, 0);
    n = ftell(fp);
    ASSERT_EQ(n, sizeof(wbuf));
    n = fseek(fp, -1, SEEK_END);
    ASSERT_EQ(n, 0);
    n = ftell(fp);
    ASSERT_EQ(n, sizeof(wbuf) - 1);
    n = fseek(fp, -1, SEEK_CUR);
    ASSERT_EQ(n, 0);
    n = ftell(fp);
    ASSERT_EQ(n, sizeof(wbuf) - 2);
    rewind(fp);
    n = ftell(fp);
    ASSERT_EQ(n, 0);

    /* read */
    n = fread(rbuf, 1, sizeof(rbuf), fp);
    ASSERT_EQ(n, sizeof(rbuf));
    ASSERT_EQ(memcmp(rbuf, wbuf, sizeof(rbuf)), 0);
    n = ftell(fp);
    ASSERT_EQ(n, sizeof(rbuf));

    fclose(fp);
}
