#ifndef _INCL_LIBMACRO
#define _INCL_LIBMACRO

struct _textHandle;
typedef struct _textHandle *    HTXTFILE;

#define lm_repeat(htxt)         while (!lm_isEOF(htxt))

void lm_printStdUsage(char * pszProgramName);
HTXTFILE lm_openWithStdArgs(int argc, char ** argv);
HTXTFILE lm_open(const char * pszSourceFilename, const char * pszTargetFilename);
void lm_close(HTXTFILE htxt);
int lm_isEOF(HTXTFILE htxt);
void lm_rewind(HTXTFILE htxt);
char * lm_getLastError(HTXTFILE htxt);
int lm_find(HTXTFILE htxt, const char * pszFindStr);
int lm_findReplace(HTXTFILE htxt, const char * pszFindStr, const char * pszReplaceStr);
int lm_findDeleteNum(HTXTFILE htxt, const char * pszFindStr, long numChars);
int lm_findDeleteToFound(HTXTFILE htxt, const char * pszFindStr);
int lm_findDeleteLineEnd(HTXTFILE htxt, const char * pszFindStr);
int lm_findDeleteFileEnd(HTXTFILE htxt, const char * pszFindStr);
int lm_findMoveNum(HTXTFILE htxt, const char * pszFindStr, long numChars);
int lm_findMoveLineEnd(HTXTFILE htxt, const char * pszFindStr);
int lm_findMoveFileEnd(HTXTFILE htxt, const char * pszFindStr);

#endif
