/*
 * Copyright (C) 2015, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>

#include "remoted/remoted.h"
#include "remoted/state.h"

#include "../wrappers/posix/select_wrappers.h"
#include "../wrappers/posix/unistd_wrappers.h"
#include "../wrappers/wazuh/shared/debug_op_wrappers.h"
#include "../wrappers/wazuh/os_net/os_net_wrappers.h"
#include "../wrappers/wazuh/remoted/state_wrappers.h"

char* remcom_output_builder(int error_code, const char* message, cJSON* data_json);
size_t remcom_dispatch(char * command, char ** output);

/* setup/teardown */

static int test_teardown(void ** state) {
    char* string = *state;
    os_free(string);
    return 0;
}

// Wrappers

int __wrap_accept() {
    return mock();
}

/* Tests */

void test_remcom_output_builder(void ** state) {
    int error_code = 5;
    const char* message = "test msg";

    cJSON* data_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_json, "test1", 18);
    cJSON_AddStringToObject(data_json, "test2", "remoted");

    char* msg = remcom_output_builder(error_code, message, data_json);

    *state = msg;

    assert_non_null(msg);
    assert_string_equal(msg, "{\"error\":5,\"message\":\"test msg\",\"data\":{\"test1\":18,\"test2\":\"remoted\"}}");
}

void test_remcom_dispatch_getstats(void ** state) {
    char* request = "{\"command\":\"getstats\"}";
    char *response = NULL;

    cJSON* data_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_json, "test1", 18);
    cJSON_AddStringToObject(data_json, "test2", "remoted");

    will_return(__wrap_rem_create_state_json, data_json);

    size_t size = remcom_dispatch(request, &response);

    *state = response;

    assert_non_null(response);
    assert_string_equal(response, "{\"error\":0,\"message\":\"ok\",\"data\":{\"test1\":18,\"test2\":\"remoted\"}}");
    assert_int_equal(size, strlen(response));
}

void test_remcom_dispatch_unknown_command(void ** state) {
    char* request = "{\"command\":\"unknown\"}";
    char *response = NULL;

    size_t size = remcom_dispatch(request, &response);

    *state = response;

    assert_non_null(response);
    assert_string_equal(response, "{\"error\":3,\"message\":\"Unrecognized command\",\"data\":{}}");
    assert_int_equal(size, strlen(response));
}

void test_remcom_dispatch_empty_command(void ** state) {
    char* request = "{}";
    char *response = NULL;

    size_t size = remcom_dispatch(request, &response);

    *state = response;

    assert_non_null(response);
    assert_string_equal(response, "{\"error\":2,\"message\":\"Empty command\",\"data\":{}}");
    assert_int_equal(size, strlen(response));
}

void test_remcom_dispatch_invalid_json(void ** state) {
    char* request = "unknown";
    char *response = NULL;

    size_t size = remcom_dispatch(request, &response);

    *state = response;

    assert_non_null(response);
    assert_string_equal(response, "{\"error\":1,\"message\":\"Invalid JSON input\",\"data\":{}}");
    assert_int_equal(size, strlen(response));
}

void test_remcom_main(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";
    size_t input_size = strlen(input) + 1;

    cJSON* data_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_json, "test1", 18);
    cJSON_AddStringToObject(data_json, "test2", "remoted");

    char *response = "{\"error\":0,\"message\":\"ok\",\"data\":{\"test1\":18,\"test2\":\"remoted\"}}";
    size_t response_size = strlen(response);

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, input_size);

    will_return(__wrap_rem_create_state_json, data_json);

    expect_value(__wrap_OS_SendSecureTCP, sock, peer);
    expect_value(__wrap_OS_SendSecureTCP, size, response_size);
    expect_string(__wrap_OS_SendSecureTCP, msg, response);
    will_return(__wrap_OS_SendSecureTCP, 0);

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_max_size(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, OS_MAXLEN);

    expect_string(__wrap__merror, formatted_msg, "Received message > '4194304'");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_empty(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, 0);

    expect_string(__wrap__mdebug1, formatted_msg, "Empty message from local client");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_error(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, -1);

    expect_string(__wrap__merror, formatted_msg, "At OS_RecvSecureTCP(): 'Success'");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_sockerror(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, OS_SOCKTERR);

    expect_string(__wrap__merror, formatted_msg, "At OS_RecvSecureTCP(): response size is bigger than expected");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_accept_error(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    errno = 0;

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, -1);

    expect_string(__wrap__merror, formatted_msg, "At accept(): 'Success'");

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, OS_SOCKTERR);

    expect_string(__wrap__merror, formatted_msg, "At OS_RecvSecureTCP(): response size is bigger than expected");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_select_zero(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    errno = 0;

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, 0);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, OS_SOCKTERR);

    expect_string(__wrap__merror, formatted_msg, "At OS_RecvSecureTCP(): response size is bigger than expected");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_select_error_eintr(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    errno = EINTR;

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, -1);

    will_return(__wrap_select, 1);

    will_return(__wrap_accept, peer);

    expect_value(__wrap_OS_RecvSecureTCP, sock, peer);
    expect_value(__wrap_OS_RecvSecureTCP, size, OS_MAXSTR);
    will_return(__wrap_OS_RecvSecureTCP, input);
    will_return(__wrap_OS_RecvSecureTCP, OS_SOCKTERR);

    expect_string(__wrap__merror, formatted_msg, "At OS_RecvSecureTCP(): response size is bigger than expected");

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread finished");

    remcom_main(NULL);
}

void test_remcom_main_select_error(void **state)
{
    int socket = 0;
    int peer = 1111;

    char *input = "{\"command\":\"getstats\"}";

    errno = 0;

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, socket);

    will_return(__wrap_select, -1);

    expect_string(__wrap__merror_exit, formatted_msg, "At select(): 'Success'");

    expect_assert_failure(remcom_main(NULL));
}

void test_remcom_main_bind_error(void **state)
{
    int socket = 0;
    int peer = 1111;

    expect_string(__wrap__mdebug1, formatted_msg, "Local requests thread ready");

    expect_string(__wrap_OS_BindUnixDomain, path, REMOTE_LOCAL_SOCK);
    expect_value(__wrap_OS_BindUnixDomain, type, SOCK_STREAM);
    expect_value(__wrap_OS_BindUnixDomain, max_msg_size, OS_MAXSTR);
    will_return(__wrap_OS_BindUnixDomain, -1);

    expect_string(__wrap__merror, formatted_msg, "Unable to bind to socket 'queue/sockets/remote': (0) 'Success'");

    remcom_main(NULL);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // Test remcom_output_builder
        cmocka_unit_test_teardown(test_remcom_output_builder, test_teardown),
        // Test remcom_dispatch
        cmocka_unit_test_teardown(test_remcom_dispatch_getstats, test_teardown),
        cmocka_unit_test_teardown(test_remcom_dispatch_unknown_command, test_teardown),
        cmocka_unit_test_teardown(test_remcom_dispatch_empty_command, test_teardown),
        cmocka_unit_test_teardown(test_remcom_dispatch_invalid_json, test_teardown),
        // Test remcom_main
        cmocka_unit_test(test_remcom_main),
        cmocka_unit_test(test_remcom_main_max_size),
        cmocka_unit_test(test_remcom_main_empty),
        cmocka_unit_test(test_remcom_main_error),
        cmocka_unit_test(test_remcom_main_sockerror),
        cmocka_unit_test(test_remcom_main_accept_error),
        cmocka_unit_test(test_remcom_main_select_zero),
        cmocka_unit_test(test_remcom_main_select_error_eintr),
        cmocka_unit_test(test_remcom_main_select_error),
        cmocka_unit_test(test_remcom_main_bind_error),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
