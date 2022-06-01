#include <stdio.h>
#include <libmacro.h>

int main(int argc, char ** argv)
{
    HTXT    ht;

    ht = lm_openWithStdArgs(argc, argv);

    while (!lm_isEOF(ht)) {
        lm_findReplace(ht, "fox", "dog");
        lm_findReplace(ht, "dog", "fox");
    }

    lm_close(ht);
}