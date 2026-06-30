#include "rivemail/rivemail.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_basic_render(void)
{
    const unsigned char msg[] = "From: A <a@example.test>\r\nTo: B <b@example.test>\r\nSubject: hello\r\n\r\nBody\r\n";
    rv_message m;
    rv_string out;
    rv_status st;
    rv_string_init(&out);
    st = rv_message_parse(msg, sizeof(msg) - 1, NULL, &m);
    assert(st == RV_OK);
    assert(rv_message_find_header(&m, "subject") != NULL);
    st = rv_message_render(&m, &out);
    assert(st == RV_OK);
    assert(out.data != NULL);
    assert(strstr(out.data, "Subject: hello") != NULL);
    rv_string_free(&out);
    rv_message_free(&m);
}

static void test_content_type(void)
{
    const unsigned char msg[] = "MIME-Version: 1.0\r\nContent-Type: text/plain; charset=utf-8\r\n\r\nhello";
    rv_message m;
    rv_status st = rv_message_parse(msg, sizeof(msg) - 1, NULL, &m);
    rv_header *h;
    assert(st == RV_OK);
    h = rv_message_find_header(&m, "content-type");
    assert(h != NULL);
    assert(h->content_type.params.len == 1);
    rv_message_free(&m);
}

static void test_validate_multipart(void)
{
    const unsigned char msg[] = "Content-Type: multipart/mixed\r\n\r\nempty";
    rv_message m;
    rv_validation_result r;
    rv_status st = rv_message_parse(msg, sizeof(msg) - 1, NULL, &m);
    assert(st == RV_OK);
    st = rv_message_validate(&m, &r);
    assert(st == RV_OK);
    assert(r.warning_count == 1);
    rv_validation_free(&r);
    rv_message_free(&m);
}

static void test_decode(void)
{
    rv_string s;
    rv_string_init(&s);
    assert(rv_decode_transfer("base64", (const unsigned char *)"SGVsbG8=", 8, &s) == RV_OK);
    assert(strcmp(s.data, "Hello") == 0);
    rv_string_free(&s);
}

int main(void)
{
    test_basic_render();
    test_content_type();
    test_validate_multipart();
    test_decode();
    puts("rivemail tests passed");
    return 0;
}
