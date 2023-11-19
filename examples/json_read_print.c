#define PPP_IMPLEMENTATION
#include "../ppp.h"

String_View extract_word_with_spaces(String_View sv)
{
    size_t i = 0;
    size_t wsc = 0;
    while (i < sv.count) {
        char c = sv.data[i++];
        if (c == ' ') {
            wsc++;
        }
    }

    char *str = malloc(sizeof(char)*i);
    size_t j = 0;
    while (wsc > 0) {
        char c = sv.data[j];
        str[j] = c;
        if (c == ' ') {
            wsc--;
        }
        j++;
    }

    return sv_from_cstr(str);
}

void example_parser(size_t *ppp_argc, char *ppp_argv[], PpP_Elda *elda, String_View line)
{
    char *out_str = "";
    size_t col_ind = 0;
    while (col_ind < (*ppp_argc)) {
        String_View word = sv_chop_by_delim(&line, ' ');
        ppp_string_append_json_sv(&out_str, col_ind++, *ppp_argc, ppp_argv, word);
    }

    PpP_Elem entry = {0};
    snprintf(entry.value, sizeof(entry.value), "%s", out_str);
    ppp_elda_insert(entry, elda);
}

int main()
{
    size_t elda_cap = 64;
    PpP_Elda elda = {0};
    ppp_elda_init(elda_cap, &elda);

    char *path = "./samples/sample_1.pdf";
    char *pdf_text = "";
    ppp_pdf_to_text(path, &pdf_text);

    printf("------------------------------------\n");
    printf("pdf_text:\n%s", pdf_text);

    char *ppp_argv[] = {"ID", "NAME", "AMOUNT"};
    size_t ppp_argc = ARRAY_LEN(ppp_argv);

    ppp_text_to_json(ppp_argc, ppp_argc, ppp_argv, "\n", &elda, pdf_text, example_parser);
    ppp_elda_print_json(elda);

    return 0;
}
