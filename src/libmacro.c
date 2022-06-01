#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "libmacro.h"

#define LIBMACRO_READ_BUFFER_SIZE                               64
#define LIBMACRO_MAX_SEARCH_LENGTH                              256

#define LIBMACRO_OK                                             0x00000000
#define LIBMACRO_ERR_OPEN_TEXT_FILE                             0x00000001
#define LIBMACRO_ERR_INSUFFICIENT_BYTES_READ                    0x00000002

#define LIBMACRO_ERR_UNKNOWN_ERROR_CODE                         0xFFFFFFFF

const char * pszERR_OPEN_SOURCE_TEXT_FILE =     "Failed to open source file %s with error: '%s'";
const char * pszERR_OPEN_TARGET_TEXT_FILE =     "Failed to open target file %s with error: '%s'";
const char * pszERR_INSUFFICIENT_BYTES_READ =   "Expected %d bytes, read %d bytes";
const char * pszERR_UNKNOWN_ERROR_CODE =        "Unknown error code 0x%08X";

struct _textHandle {
    FILE *          fptrSource;
    FILE *          fptrTarget;

    char *          pszOutputText;
    
    char *          pszErrorString;
    uint32_t        errorCode;
};

void lm_printStdUsage(char * pszProgramName)
{
    printf("Usage:\n\n");
    printf("%s <options> input-file\n", pszProgramName);
    printf("\t-o <output file>\n");
    printf("\t-h print help\n\n");
}

HTXT lm_openWithStdArgs(int argc, char ** argv)
{
    char *          pszInputFile = NULL;
    char *          pszOutputFile = NULL;

    if (argc > 1) {
        pszInputFile = strdup(&argv[argc - 1][0]);

        for (int i = 1;i < (argc - 1);i++) {
            if (argv[i][0] == '-') {
                if (argv[i][1] == 'o') {
                    pszOutputFile = strdup(&argv[i+1][0]);
                }
                else if (argv[i][1] == 'h') {
                    lm_printStdUsage(&argv[0][0]);
                    exit(0);
                }
            }
        }
    }
    else {
        lm_printStdUsage(&argv[0][0]);
        exit(-1);
    }

    if (pszInputFile == NULL) {
        fprintf(stderr, "FATAL ERROR: No input file specified\n");
        exit(-1);
    }
    
    return lm_open(pszInputFile, pszOutputFile);
}

HTXT lm_open(const char * pszSourceFilename, const char * pszTargetFilename)
{
    HTXT        htxt;

    htxt = (HTXT)malloc(sizeof(struct _textHandle));

    if (htxt == NULL) {
        fprintf(stderr, "FATAL ERROR: Failed to allocate memory for handle\n");
        exit(-1);
    }

    htxt->pszErrorString = NULL;

    htxt->fptrSource = fopen(pszSourceFilename, "rt");

    if (htxt->fptrSource == NULL) {
        htxt->errorCode = LIBMACRO_ERR_OPEN_TEXT_FILE;

        htxt->pszErrorString = (char *)malloc(strlen(pszERR_OPEN_SOURCE_TEXT_FILE) + strlen(pszSourceFilename) + strlen(strerror(errno)));
        sprintf(htxt->pszErrorString, pszERR_OPEN_SOURCE_TEXT_FILE, pszSourceFilename, strerror(errno));

        return NULL;
    }

    htxt->fptrTarget = fopen(pszTargetFilename, "wt");

    if (htxt->fptrTarget == NULL) {
        htxt->errorCode = LIBMACRO_ERR_OPEN_TEXT_FILE;

        htxt->pszErrorString = (char *)malloc(strlen(pszERR_OPEN_TARGET_TEXT_FILE) + strlen(pszTargetFilename) + strlen(strerror(errno)));
        sprintf(htxt->pszErrorString, pszERR_OPEN_TARGET_TEXT_FILE, pszTargetFilename, strerror(errno));

        return NULL;
    }

    return htxt;
}

void lm_close(HTXT htxt)
{
    fflush(htxt->fptrTarget);
    fclose(htxt->fptrTarget);
    fclose(htxt->fptrSource);

    free(htxt->pszOutputText);

    if (htxt->pszErrorString != NULL) {
        free(htxt->pszErrorString);
    }

    free(htxt);
}

int lm_isEOF(HTXT htxt)
{
    return (feof(htxt->fptrSource) != 0 ? 1 : 0);
}

void lm_rewind(HTXT htxt)
{
    rewind(htxt->fptrSource);
}

char * lm_getLastError(HTXT htxt)
{
    char * pszCopy = NULL;
    
    if (htxt->pszErrorString != NULL) {
        pszCopy = strdup(htxt->pszErrorString);
        free(htxt->pszErrorString);
    }

    return pszCopy;
}

int lm_find(HTXT htxt, const char * pszFindStr)
{
    char        szBuffer[LIBMACRO_MAX_SEARCH_LENGTH + 1];
    size_t      bufferLength;
    int         bytesRead;
    int         isFound = 0;

    /*
    ** Our buffer is the length of the search string, this is
    ** our 'window' on the text file...
    */
    bufferLength = strnlen(pszFindStr, LIBMACRO_MAX_SEARCH_LENGTH);

    /*
    ** Read a block bufferLength bytes long at position n, 
    ** see if our 'window' is our search string. If not, rewind 
    ** the file pointer to shift our 'window' on 1 byte and read 
    ** the next block... 
    */
    while (!isFound) {
        bytesRead = fread(szBuffer, 1, bufferLength, htxt->fptrSource);
        szBuffer[bytesRead] = 0;

        if (bytesRead == 0) {
            return 0;
        }
        
        isFound = (strncmp(szBuffer, pszFindStr, bufferLength) == 0 ? 1 : 0);

        if (!isFound) {
            fputc(szBuffer[0], htxt->fptrTarget);

            // Rewind the file pointer...
            fseek(htxt->fptrSource, -(bytesRead - 1), SEEK_CUR);
        }
    }

    fseek(htxt->fptrSource, -bytesRead, SEEK_CUR);

    return 1;
}

int lm_findReplace(HTXT htxt, const char * pszFindStr, const char * pszReplaceStr)
{
    int         isFound;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        fseek(htxt->fptrSource, strlen(pszFindStr), SEEK_CUR);
        fwrite(pszReplaceStr, 1, strlen(pszReplaceStr), htxt->fptrTarget);
    }

    return isFound;    
}

int lm_findDeleteNum(HTXT htxt, const char * pszFindStr, long numChars)
{
    int         isFound;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        fseek(htxt->fptrSource, numChars, SEEK_CUR);
    }

    return isFound;    
}

int lm_findDeleteLineEnd(HTXT htxt, const char * pszFindStr)
{
    int         isFound;
    int         c;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        c = fgetc(htxt->fptrSource);

        while (c != '\n') {
            c = fgetc(htxt->fptrSource);
        }
    }

    return isFound;    
}

int lm_findDeleteFileEnd(HTXT htxt, const char * pszFindStr)
{
    int         isFound;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        fseek(htxt->fptrSource, 0L, SEEK_END);
    }

    return isFound;    
}

int lm_findMoveNum(HTXT htxt, const char * pszFindStr, long numChars)
{
    int         isFound;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        fseek(htxt->fptrSource, numChars, SEEK_CUR);
    }

    return isFound;    
}

int lm_findMoveLineEnd(HTXT htxt, const char * pszFindStr)
{
    int         isFound;
    int         c;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        c = fgetc(htxt->fptrSource);

        while (c != '\n') {
            c = fgetc(htxt->fptrSource);
        }
    }

    return isFound;    
}

int lm_findMoveFileEnd(HTXT htxt, const char * pszFindStr)
{
    int         isFound;

    isFound = lm_find(htxt, pszFindStr);

    if (isFound) {
        fseek(htxt->fptrSource, 0L, SEEK_END);
    }

    return isFound;    
}
