#define DOMAIN_VSPACE "vspace"
#define COMMAND_VSPACE_PING "--ping"

static void handle_domain_vspace(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command = get_token(&message);
    if (token_equals(command, COMMAND_VSPACE_PING)) {
        struct token argument = get_token(&message);
        if (token_is_valid(argument)) {
            daemon_fail(rsp, "unexpected argument '%.*s' for command '%.*s' for domain '%.*s'\n", argument.length, argument.text, command.length, command.text, domain.length, domain.text);
            return;
        }

        fprintf(rsp, "{\"status\":\"ok\",\"component\":\"vspace\"}\n");
    } else {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
    }
}
