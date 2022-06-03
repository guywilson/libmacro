#include <stdio.h>
#include <libmacro.h>

int main(int argc, char ** argv)
{
    HTXT        htxt;

    htxt = lm_openWithStdArgs(argc, argv);

    while (!lm_isEOF(htxt)) {
        lm_findReplace(htxt, "HTXT", "HMACRO");
    }

    lm_close(htxt);
}