#ifndef _INCL_LIBMACRO
#define _INCL_LIBMACRO

struct _textHandle;
typedef struct _textHandle *     HTXT;

void lm_printStdUsage(char * pszProgramName);
HTXT lm_openWithStdArgs(int argc, char ** argv);
HTXT lm_open(const char * pszSourceFilename, const char * pszTargetFilename);
void lm_close(HTXT htxt);
int lm_isEOF(HTXT htxt);
void lm_rewind(HTXT htxt);
char * lm_getLastError(HTXT htxt);
int lm_find(HTXT htxt, const char * pszFindStr);
int lm_findReplace(HTXT htxt, const char * pszFindStr, const char * pszReplaceStr);
int lm_findDeleteNum(HTXT htxt, const char * pszFindStr, long numChars);
int lm_findDeleteLineEnd(HTXT htxt, const char * pszFindStr);
int lm_findDeleteFileEnd(HTXT htxt, const char * pszFindStr);
int lm_findMoveNum(HTXT htxt, const char * pszFindStr, long numChars);
int lm_findMoveLineEnd(HTXT htxt, const char * pszFindStr);
int lm_findMoveFileEnd(HTXT htxt, const char * pszFindStr);

#endif
