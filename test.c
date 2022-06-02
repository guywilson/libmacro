#include <stdio.h>
#include <libmacro.h>

int main(int argc, char ** argv)
{
    HTXT    ht;
    HTXT    ht2;

    ht = lm_openWithStdArgs(argc, argv);

    while (!lm_isEOF(ht)) {
        lm_findReplace(ht, "fox", "dog");
        lm_findReplace(ht, "dog", "fox");
    }

    lm_close(ht);

    ht2 = lm_open("email.txt", "email_out.txt");

    while (!lm_isEOF(ht2)) {
        lm_findDeleteToFound(ht2, "<");
        lm_findDeleteNum(ht2, "<", 1);
        lm_findReplace(ht2, ">;", ",\n");
    }

    lm_close(ht2);
}