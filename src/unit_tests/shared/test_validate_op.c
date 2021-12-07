/*
 * Copyright (C) 2015-2021, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../headers/shared.h"
#include "../../headers/validate_op.h"
#include "../wrappers/wazuh/shared/expression_wrappers.h"
#include "../wrappers/wazuh/os_net/os_net_wrappers.h"
#include "../../shared/validate_op.c"

/* tests */

#define TEST_MOCKED

void w_validate_bytes_non_number (void **state)
{
    const char * value = "hello";
    long long expected_value = -1;

    long long ret = w_validate_bytes(value);
    assert_memory_equal(&ret, &expected_value, sizeof(long long));
}

void w_validate_bytes_bytes (void **state)
{
    const char * value = "1024B";
    long long expected_value = 1024;

    long long ret = w_validate_bytes(value);
    assert_memory_equal(&ret, &expected_value, sizeof(long long));
}

void w_validate_bytes_kilobytes (void **state)
{
    const char * value = "1024KB";
    long long expected_value = 1024*1024;

    long long ret = w_validate_bytes(value);
    assert_memory_equal(&ret, &expected_value, sizeof(long long));
}

void w_validate_bytes_megabytes (void **state)
{
    const char * value = "1024MB";
    long long expected_value = 1024*1024*1024;

    long long ret = w_validate_bytes(value);
    assert_memory_equal(&ret, &expected_value, sizeof(long long));
}

void w_validate_bytes_gigabytes (void **state)
{
    const char * value = "1024GB";
    long long expected_value = 1024 * ((long long) 1024*1024*1024);

    long long ret = w_validate_bytes(value);
    assert_memory_equal(&ret, &expected_value, sizeof(long long));
}

void OS_IsValidIP_null(void **state)
{
    int ret = OS_IsValidIP(NULL, NULL);
    assert_int_equal(ret, 0);
}

void OS_IsValidIP_any(void **state)
{
    int ret = OS_IsValidIP("any", NULL);
    assert_int_equal(ret, 2);
}

void OS_IsValidIP_any_struct(void **state)
{
    int ret = 0;
    os_ip *ret_ip;

    os_calloc(1, sizeof(os_ip), ret_ip);

    ret = OS_IsValidIP("any", ret_ip);
    assert_int_equal(ret, 2);
    assert_int_equal(ret_ip->is_ipv6, FALSE);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_not_valid_ip(void **state)
{
    unsigned int i = 0;
    while (ip_address_regex[i] != NULL) {
        expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
        will_return(__wrap_w_expression_compile, 1);
        will_return(__wrap_w_expression_match, 0);
        i++;
    }

    int ret = OS_IsValidIP("12.0", NULL);
    assert_int_equal(ret, 0);
}

void OS_IsValidIP_valid_multi_ipv4(void **state)
{
    const char * ip_to_test[] = {
        "1.1.1.1",
        "255.255.255.255",
        "100.100.100.100",
        "10.10.10.10",
        "111.111.111.111",
        "222.222.222.222",
        "127.0.0.1",
        // valid ip with '!'
        "!127.0.0.1",
        NULL,
    };

    int ret = 0;
    os_ip *ret_ip;

    for (int i = 0; ip_to_test[i] != NULL; i++) {

        os_calloc(1, sizeof(os_ip), ret_ip);

        expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
        will_return(__wrap_w_expression_compile, 1);
        will_return(__wrap_w_expression_match, -1);
        will_return(__wrap_w_expression_match, ip_to_test);

        will_return(__wrap_get_ipv4_numeric, 0);

        ret = OS_IsValidIP(ip_to_test[i], ret_ip);
        assert_string_equal(ip_to_test[i], ret_ip->ip);
        assert_int_equal(ret, 1);
        assert_int_equal(ret_ip->is_ipv6, FALSE);

        w_free_os_ip(ret_ip);
    }
}

void OS_IsValidIP_not_valid_multi_ipv4(void **state)
{
    const char * ip_to_test[] = {
        // more or less than 4 octets
        "111",
        "01.01",
        "01.01.01",
        "10.10.10.10.10",
        "222.222.222.222.222",
        // octet limit exceeded (more than 255)
        "333.333.334.334",
        "256.1.01.001",
        "1.1.1.256",
        "327.0.0.1",
        "4000.00.0.1",
        // ip with ! at beginnig and invalids ips
        "!256.256.256.256",
        "!!10.100.10.1",
        // ip with index limit exceeded (more than 32)
        "10.10.10.10/",
        "10.10.10.10/33",
        "10.10.10.10/99",
        "10.10.10.10/123",
        "10.10.10.10/12345",
        // ip with extra 0
        "01.01.01.01",
        "001.001.001.001",
        "000.00.0.1",
        "!000.00.0.1",
        // ip with invalid netmask
        "1.1.1.10/36.255.255",
        "1.1.1.1/36.1.1.256",
        "1.1.1.300/36.1.1.255",
        NULL,
    };

    int ret = 0;
    os_ip *ret_ip;

    for (int i = 0; ip_to_test[i] != NULL; i++) {

        os_calloc(1, sizeof(os_ip), ret_ip);

        unsigned int a = 0;
        while (ip_address_regex[a] != NULL) {
            expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
            will_return(__wrap_w_expression_compile, 1);
            will_return(__wrap_w_expression_match, 0);
            a++;
        }

        ret = OS_IsValidIP(ip_to_test[i], ret_ip);
        assert_string_equal(ip_to_test[i], ret_ip->ip);
        assert_int_equal(ret, 0);

        w_free_os_ip(ret_ip);
    }
}

void OS_IsValidIP_valid_ipv4_CIDR(void **state)
{
    char ip_to_test[] = {"192.168.10.12/32"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "192.168.10.12");
    will_return(__wrap_w_expression_match, "32");

    will_return(__wrap_get_ipv4_numeric, 0);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_string_equal(ip_to_test, ret_ip->ip);
    assert_int_equal(ret, 2);
    assert_int_equal(ret_ip->is_ipv6, FALSE);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_fail(void **state)
{
    char ip_to_test[] = {"192.168.10.12/32"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "192.168.10.12");
    will_return(__wrap_w_expression_match, "32");

    will_return(__wrap_get_ipv4_numeric, -1);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_zero_fail(void **state)
{
    char ip_to_test[] = {"0.0.0.0/32"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "0.0.0");
    will_return(__wrap_w_expression_match, "32");

    will_return(__wrap_get_ipv4_numeric, -1);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_zero_pass(void **state)
{
    char ip_to_test[] = {"0.0.0.0/32"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "0.0.0.0");
    will_return(__wrap_w_expression_match, "32");

    will_return(__wrap_get_ipv4_numeric, -1);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 2);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_netmask(void **state)
{
    char ip_to_test[] = {"32.32.32.32/255.255.255.255"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "32.32.32.32");
    will_return(__wrap_w_expression_match, "255.255.255.255");

    will_return(__wrap_get_ipv4_numeric, 0);
    will_return(__wrap_get_ipv4_numeric, 0);


    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_string_equal(ip_to_test, ret_ip->ip);
    assert_int_equal(ret, 2);
    assert_int_equal(ret_ip->is_ipv6, FALSE);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_0_netmask(void **state)
{
    char ip_to_test[] = {"0.0.0.0/255.255.255.255"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "0.0.0.0");
    will_return(__wrap_w_expression_match, "255.255.255.255");

    will_return(__wrap_get_ipv4_numeric, 0);
    will_return(__wrap_get_ipv4_numeric, 0);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_string_equal(ip_to_test, ret_ip->ip);
    assert_int_equal(ret, 2);
    assert_int_equal(ret_ip->is_ipv6, FALSE);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_netmask_0(void **state)
{
    char ip_to_test[] = {"16.16.16.16/255.255.255.0"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "16.16.16.16");
    will_return(__wrap_w_expression_match, "255.255.255.0");

    will_return(__wrap_get_ipv4_numeric, 0);
    will_return(__wrap_get_ipv4_numeric, 0);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_string_equal(ip_to_test, ret_ip->ip);
    assert_int_equal(ret, 2);
    assert_int_equal(ret_ip->is_ipv6, FALSE);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_netmask_fail(void **state)
{
    char ip_to_test[] = {"32.32.32.32/255.255.255.255"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "32.32.32.32");
    will_return(__wrap_w_expression_match, "255.255.255.255");

    will_return(__wrap_get_ipv4_numeric, 0);
    will_return(__wrap_get_ipv4_numeric, -1);


    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_sub_string_0(void **state)
{
    char ip_to_test[] = {"32.32.32.32/255.255.255.255"};

    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -3);

    int ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv4_netmask_0_NULL_struct(void **state)
{
    char ip_to_test[] = {"16.16.16.16/255.255.255.0"};

    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 1);

    int ret = OS_IsValidIP(ip_to_test, NULL);
    assert_int_equal(ret, 1);
}

void OS_IsValidIP_valid_multi_ipv6(void **state)
{
    const char * ip_to_test[] = {
        "2001:db8:abcd:0012:0000:0000:0000:0000",
        "2001:db8:abcd:0012:ffff:ffff:ffff:ffff",
        "fe80::ceaf:9ff2:b33c:1ca7",
        "11AA:11AA:11AA:11AA:11AA:11AA:11AA:11AA",
        "11AA::11AA:11AA:11AA:11AA:11AA:11AA",
        "11AA::11AA:11AA:11AA:11AA:11AA",
        "11AA::11AA:11AA:11AA:11AA",
        "11AA::11AA:11AA:11AA",
        "11AA::11AA:11AA",
        "11AA::11AA",
        "11AA:11AA:11AA:11AA:11AA:11AA::11AA",
        "11AA:11AA:11AA:11AA:11AA::11AA",
        "11AA:11AA:11AA:11AA::11AA",
        "11AA:11AA:11AA::11AA",
        "11AA:11AA::11AA",
        "11AA::11AA",
        "11AA::11AA:11AA:11AA:11AA:11AA:11AA",
        "11AA:11AA::11AA:11AA:11AA:11AA:11AA",
        "11AA:11AA:11AA::11AA:11AA:11AA:11AA",
        "11AA:11AA:11AA:11AA::11AA:11AA:11AA",
        "11AA:11AA:11AA:11AA:11AA::11AA:11AA",
        "11AA:11AA:11AA:11AA:11AA:11AA::11AA",
        "11AA:11AA:11AA:11AA:11AA:11AA:11AA::",
        "11AA:11AA:11AA:11AA:11AA:11AA::",
        "11AA:11AA:11AA:11AA:11AA::",
        "11AA:11AA:11AA:11AA::",
        "11AA:11AA:11AA::",
        "11AA:11AA::",
        "11AA::",
        "::11AA:11AA:11AA:11AA:11AA:11AA:11AA",
        "::11AA:11AA:11AA:11AA:11AA:11AA",
        "::11AA:11AA:11AA:11AA:11AA",
        "::11AA:11AA:11AA:11AA",
        "::11AA:11AA:11AA",
        "::11AA:11AA",
        "::11AA",
        "::",
        NULL,
    };

    int ret = 0;
    os_ip *ret_ip;

    for (int i = 0; ip_to_test[i] != NULL; i++) {

        os_calloc(1, sizeof(os_ip), ret_ip);

        /* First call to __wrap_w_expression_match fail */
        expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
        will_return(__wrap_w_expression_compile, 1);
        will_return(__wrap_w_expression_match, 0);

        /* Second call to __wrap_w_expression_match pass */
        expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
        will_return(__wrap_w_expression_compile, 1);
        will_return(__wrap_w_expression_match, -1);
        will_return(__wrap_w_expression_match, ip_to_test);

        will_return(__wrap_get_ipv6_numeric, 0);

        ret = OS_IsValidIP(ip_to_test[i], ret_ip);
        assert_string_equal(ip_to_test[i], ret_ip->ip);
        assert_int_equal(ret, 1);
        assert_int_equal(ret_ip->is_ipv6, TRUE);
        assert_non_null(ret_ip->ipv6->ip_address);

        w_free_os_ip(ret_ip);
    }
}

void OS_IsValidIP_not_valid_multi_ipv6(void **state)
{

    const char * ip_to_test[] = {
        "::11AA:11AA:11AA:11AA:11AA::11AA",
        "::11AA:11AA:11AA:11AA:11AA:11AA:",
        "::11AA:11AA::11AA:11AA:11AA:",
        "::11AA:11AA:11AA:11AA:::",
        "::11AA::11AA:11AA::11AA:11AA:11AA::11AA",
        "11AA:11AA:11AA:11AA:11AA:11AA:11AA:11AA:11AA",
        "GGAA:11AA:11AA:11AA:11AA:11AA:11AA:11AA",
        NULL,
    };

    int ret = 0;
    os_ip *ret_ip;

    for (int i = 0; ip_to_test[i] != NULL; i++) {

    os_calloc(1, sizeof(os_ip), ret_ip);

        int a = 0;
        while (ip_address_regex[a] != NULL) {
            expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
            will_return(__wrap_w_expression_compile, 1);
            will_return(__wrap_w_expression_match, 0);
            a++;
        }

        ret = OS_IsValidIP(ip_to_test[i], ret_ip);
        assert_string_equal(ip_to_test[i], ret_ip->ip);
        assert_int_equal(ret, 0);

        w_free_os_ip(ret_ip);
    }
}

void OS_IsValidIP_valid_ipv6_prefix(void **state)
{
    char ip_to_test[] = {"2001:db8:abcd:0012:0000:0000:0000:0000/60"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    /* First call to __wrap_w_expression_match fail */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 0);

    /* Second call to __wrap_w_expression_match pass */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "2001:db8:abcd:0012:0000:0000:0000:0000");
    will_return(__wrap_w_expression_match, "60");

    will_return(__wrap_get_ipv6_numeric, 0);

    ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_string_equal(ip_to_test, ret_ip->ip);
    assert_int_equal(ret, 2);
    assert_int_equal(ret_ip->is_ipv6, TRUE);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv6_prefix_NULL_struct(void **state)
{
    char ip_to_test[] = {"2001:db8:abcd:0012:0000:0000:0000:0000/64"};

    /* First call to __wrap_w_expression_match fail */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 0);

    /* Second call to __wrap_w_expression_match pass */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 1);

    int ret = OS_IsValidIP(ip_to_test, NULL);
    assert_int_equal(ret, 1);
}

void OS_IsValidIP_valid_ipv6_numeric_fail(void **state)
{
    char ip_to_test[] = {"2001:db8:abcd:0012:0000:0000:0000:0000"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    /* First call to __wrap_w_expression_match fail */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 0);

    /* Second call to __wrap_w_expression_match pass */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -1);
    will_return(__wrap_w_expression_match, "2001:db8:abcd:0012:0000:0000:0000:0000");

    will_return(__wrap_get_ipv6_numeric, -1);

    ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv6_converNetmask_fail(void **state)
{
    char ip_to_test[] = {"2001:db8:abcd:0012:0000:0000:0000:0000/64"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    /* First call to __wrap_w_expression_match fail */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 0);

    /* Second call to __wrap_w_expression_match pass */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -2);
    will_return(__wrap_w_expression_match, "2001:db8:abcd:0012:0000:0000:0000:0000");
    will_return(__wrap_w_expression_match, "644");

    will_return(__wrap_get_ipv6_numeric, 0);

    ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IsValidIP_valid_ipv6_sub_string_0(void **state)
{
    char ip_to_test[] = {"2001:db8:abcd:0012:0000:0000:0000:0000/64"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    /* First call to __wrap_w_expression_match fail */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, 0);

    /* Second call to __wrap_w_expression_match pass */
    expect_value(__wrap_w_calloc_expression_t, type, EXP_TYPE_PCRE2);
    will_return(__wrap_w_expression_compile, 1);
    will_return(__wrap_w_expression_match, -3);

    ret = OS_IsValidIP(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IPFound_not_valid_ip(void **state)
{
    char ip_to_test[] = {"2001::db8:abcd::0012/64"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);

    will_return(__wrap_get_ipv4_numeric, -1);
    will_return(__wrap_get_ipv6_numeric, -1);

    ret = OS_IPFound(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IPFound_valid_ipv4(void **state)
{
    char ip_to_test[] = {"255.255.255.255"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);
    os_strdup("255.255.255.255", ret_ip->ip);
    os_calloc(1, sizeof(os_ipv4), ret_ip->ipv4);

    ret_ip->ipv4->ip_address = 0xFFFFFFFF;
    ret_ip->ipv4->netmask = 0xFFFFFFFF;

    will_return(__wrap_get_ipv4_numeric, 1);
    will_return(__wrap_get_ipv4_numeric, 0xFFFFFFFF);

    ret = OS_IPFound(ip_to_test, ret_ip);
    assert_int_equal(ret, 1);

    w_free_os_ip(ret_ip);
}

void OS_IPFound_valid_ipv4_negated(void **state)
{
    //char ip_to_test[] = {"2001:db8:abcd:0012:0000:0000:0000:0000/64"};
    char ip_to_test[] = {"16.16.16.16"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);
    os_strdup("!16.16.16.16", ret_ip->ip);
    os_calloc(1, sizeof(os_ipv4), ret_ip->ipv4);

    ret_ip->ipv4->ip_address = 0x10101010;
    ret_ip->ipv4->netmask = 0xFFFFFFFF;

    will_return(__wrap_get_ipv4_numeric, 1);
    will_return(__wrap_get_ipv4_numeric, 0x10101010);

    ret = OS_IPFound(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

void OS_IPFound_valid_ipv6(void **state)
{
    char ip_to_test[] = {"1010:1010:1010:1010:1010:1010:1010:1010"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);
    os_strdup("1010:1010:1010:1010:1010:1010:1010:1010", ret_ip->ip);
    os_calloc(1, sizeof(os_ipv6), ret_ip->ipv6);

    unsigned int a = 0;
    for(a = 0; a < 16; a++) {
        ret_ip->ipv6->ip_address[a] = 0x10;
    }
    for(a = 0; a < 16; a++) {
        ret_ip->ipv6->netmask[a] = 0xFF;
    }

    will_return(__wrap_get_ipv4_numeric, -1);
    will_return(__wrap_get_ipv6_numeric, 1);
    will_return(__wrap_get_ipv6_numeric, 0x10);

    ret = OS_IPFound(ip_to_test, ret_ip);
    assert_int_equal(ret, 1);

    w_free_os_ip(ret_ip);
}

void OS_IPFound_valid_ipv6_fail(void **state)
{
    char ip_to_test[] = {"1010:1010:1010:1010:1010:1010:1010:1010"};

    int ret = 0;
    os_ip *ret_ip;
    os_calloc(1, sizeof(os_ip), ret_ip);
    os_strdup("1010:1010:1010:1010:1010:1010:1010:1010", ret_ip->ip);
    os_calloc(1, sizeof(os_ipv6), ret_ip->ipv6);

    unsigned int a = 0;
    for(a = 0; a < 16; a++) {
        ret_ip->ipv6->ip_address[a] = 0x10;
    }
    for(a = 0; a < 16; a++) {
        ret_ip->ipv6->netmask[a] = 0xFF;
    }

    will_return(__wrap_get_ipv4_numeric, -1);
    will_return(__wrap_get_ipv6_numeric, 1);
    will_return(__wrap_get_ipv6_numeric, 0x00);

    ret = OS_IPFound(ip_to_test, ret_ip);
    assert_int_equal(ret, 0);

    w_free_os_ip(ret_ip);
}

int main(void) {

    const struct CMUnitTest tests[] = {
        // Tests w_validate_bytes
        cmocka_unit_test(w_validate_bytes_non_number),
        cmocka_unit_test(w_validate_bytes_bytes),
        cmocka_unit_test(w_validate_bytes_kilobytes),
        cmocka_unit_test(w_validate_bytes_megabytes),
        cmocka_unit_test(w_validate_bytes_gigabytes),
        // Test OS_IsValidIP
        cmocka_unit_test(OS_IsValidIP_null),
        cmocka_unit_test(OS_IsValidIP_any),
        cmocka_unit_test(OS_IsValidIP_any_struct),
        cmocka_unit_test(OS_IsValidIP_not_valid_ip),
        cmocka_unit_test(OS_IsValidIP_valid_multi_ipv4),
        cmocka_unit_test(OS_IsValidIP_not_valid_multi_ipv4),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_CIDR),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_fail),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_zero_fail),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_zero_pass),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_netmask),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_0_netmask),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_netmask_0),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_netmask_fail),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_sub_string_0),
        cmocka_unit_test(OS_IsValidIP_valid_ipv4_netmask_0_NULL_struct),
        cmocka_unit_test(OS_IsValidIP_valid_multi_ipv6),
        cmocka_unit_test(OS_IsValidIP_not_valid_multi_ipv6),
        cmocka_unit_test(OS_IsValidIP_valid_ipv6_prefix),
        cmocka_unit_test(OS_IsValidIP_valid_ipv6_prefix_NULL_struct),
        cmocka_unit_test(OS_IsValidIP_valid_ipv6_numeric_fail),
        cmocka_unit_test(OS_IsValidIP_valid_ipv6_converNetmask_fail),
        cmocka_unit_test(OS_IsValidIP_valid_ipv6_sub_string_0),
        // Test OS_IPFound
        cmocka_unit_test(OS_IPFound_not_valid_ip),
        cmocka_unit_test(OS_IPFound_valid_ipv4),
        cmocka_unit_test(OS_IPFound_valid_ipv4_negated),
        cmocka_unit_test(OS_IPFound_valid_ipv6),
        cmocka_unit_test(OS_IPFound_valid_ipv6_fail),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
