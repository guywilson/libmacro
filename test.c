#include <stdio.h>
#include <libmacro.h>

int main(void)
{
    HTXT    ht;
    int     isFound;

    ht = lm_open("test.txt", "out.txt");

    while (!lm_isEOF(ht)) {
        lm_findReplace(ht, "fox", "dog");
        lm_findReplace(ht, "dog", "fox");
    }

    lm_close(ht);
}