#include <stdio.h>
#include <stdlib.h>

#define PPP_IMPLEMENTATION
#include "../ppp.h"

void example_parser(size_t *printc, size_t ppp_argc, char *ppp_argv[], PpP_Elda *elda, String_View line)
{
    char *out_string = "";
    size_t col_ind = 0;
    while (col_ind < ppp_argc) {
        String_View word = sv_chop_by_delim(&line, ' '); 
        ppp_string_append_json_sv(&out_string, col_ind++, *printc, ppp_argc, ppp_argv, word);
    }
    PpP_Elem entry = {0};
    snprintf(entry.value, 256, "%s", out_string);
    ppp_elda_insert(entry, elda);
}

int main()
{
    size_t elda_cap = 32;
    PpP_Elda elda = {0};
    ppp_elda_init(elda_cap, &elda);

    char *path = "./samples/sample_2.pdf";
    char *text = "";
    ppp_pdf_to_text(path, &text); 

    char *ppp_argv[] = {"ID", "NAME", "SURNAME", "BIRTH DATE"};
    size_t ppp_argc = ARRAY_LEN(ppp_argv);

    ppp_extract_table(4, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    ppp_elda_clear(&elda);

    ppp_elda_init(elda_cap, &elda);
    ppp_extract_table(3, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    ppp_elda_clear(&elda);

    ppp_elda_init(elda_cap, &elda);
    ppp_extract_table(2, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    ppp_elda_clear(&elda);
    
    ppp_elda_init(elda_cap, &elda);
    ppp_extract_table(1, ppp_argc, ppp_argv, "\n", &elda, text, example_parser);
    ppp_elda_print_json(elda);

    return 0;
}
