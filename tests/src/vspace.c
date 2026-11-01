static bool vspace_test_response(char *message, char *expected)
{
    char *response = NULL;
    size_t length = 0;
    FILE *rsp = open_memstream(&response, &length);
    if (!rsp) return false;

    handle_message(rsp, message);
    int status = fclose(rsp);
    bool result = status == 0 && string_equals(response, expected);
    if (!result) fprintf(stderr, "expected: %sreceived: %s", expected, response ? response : "(null)\n");
    free(response);
    return result;
}

TEST_FUNC(vspace_ping,
{
    bool matches = vspace_test_response("vspace\0--ping\0", "{\"status\":\"ok\",\"component\":\"vspace\"}\n");
    TEST_CHECK(matches, true);
});

TEST_FUNC(vspace_missing_command,
{
    bool matches = vspace_test_response("vspace\0", FAILURE_MESSAGE "unknown command '' for domain 'vspace'\n");
    TEST_CHECK(matches, true);
});

TEST_FUNC(vspace_unknown_command,
{
    bool matches = vspace_test_response("vspace\0--unknown\0", FAILURE_MESSAGE "unknown command '--unknown' for domain 'vspace'\n");
    TEST_CHECK(matches, true);
});

TEST_FUNC(vspace_extra_argument,
{
    bool matches = vspace_test_response("vspace\0--ping\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--ping' for domain 'vspace'\n");
    TEST_CHECK(matches, true);
});
