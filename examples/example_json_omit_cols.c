#include <stdio.h>
#include <stdlib.h>

#define PPP_IMPLEMENTATION
#include "../ppp.h"

void example_parser(size_t *argc,  char *argv[], PpP_Elda *elda, String_View line)
{
    size_t argc_cpy = (*argc);

    char *out_string = "";
    char buf[512];
    while (*argc > 0) {
        String_View word = sv_chop_by_delim(&line, ' '); 
        if (*argc == 1) {
            snprintf(buf, 512, "%s: "SV_Fmt"", argv[argc_cpy - (*argc)], SV_Arg(word));
        } else {
            snprintf(buf, 512, "%s: "SV_Fmt", ", argv[argc_cpy - (*argc)], SV_Arg(word));
        }
        out_string = ppp_string_append(out_string, buf);
        (*argc) = (*argc) - 1;
    }
    (*argc) = argc_cpy;
    PpP_Elem entry = {0};
    snprintf(entry.value, 256, "%s", out_string);
    ppp_elda_insert(entry, elda);
}

int main()
{
    size_t elda_cap = 128;
    PpP_Elda elda = {0};
    ppp_elda_init(elda_cap, &elda);

    const char *path = "./samples/sample_2.pdf";
    const char* text = ppp_pdf_to_text(path); 

    const char *ppp_argv[] = {"ID", "NAME", "SURNAME", "BIRTH DATE"};
    size_t ppp_argc = ARRAY_LEN(ppp_argv);

    ppp_text_to_json(4, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    ppp_elda_clear(&elda);
    ppp_elda_init(elda_cap, &elda);
    ppp_text_to_json(2, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    ppp_elda_clear(&elda);
    ppp_elda_init(elda_cap, &elda);
    ppp_text_to_json(1, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    return 0;
}
