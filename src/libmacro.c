/******************************************************************************
**
** Definitions of the libmacro library functions. This library allows the user
** to write simple, but powerful C programs to perform text processing. See
** the test.c file for some examples.
**
** Copyright: Guy Wilson 2022
**
** License:
**                    GNU GENERAL PUBLIC LICENSE
**                        Version 3, 29 June 2007
**
**  Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
**  Everyone is permitted to copy and distribute verbatim copies
**  of this license document, but changing it is not allowed.
**
**                            Preamble
**
**  The GNU General Public License is a free, copyleft license for
** software and other kinds of works.
**
**   The licenses for most software and other practical works are designed
** to take away your freedom to share and change the works.  By contrast,
** the GNU General Public License is intended to guarantee your freedom to
** share and change all versions of a program--to make sure it remains free
** software for all its users.  We, the Free Software Foundation, use the
** GNU General Public License for most of our software; it applies also to
** any other work released this way by its authors.  You can apply it to
** your programs, too.
** 
**   When we speak of free software, we are referring to freedom, not
** price.  Our General Public Licenses are designed to make sure that you
** have the freedom to distribute copies of free software (and charge for
** them if you wish), that you receive source code or can get it if you
** want it, that you can change the software or use pieces of it in new
** free programs, and that you know you can do these things.
** 
**   To protect your rights, we need to prevent others from denying you
** these rights or asking you to surrender the rights.  Therefore, you have
** certain responsibilities if you distribute copies of the software, or if
** you modify it: responsibilities to respect the freedom of others.
** 
**   For example, if you distribute copies of such a program, whether
** gratis or for a fee, you must pass on to the recipients the same
** freedoms that you received.  You must make sure that they, too, receive
** or can get the source code.  And you must show them these terms so they
** know their rights.
** 
**   Developers that use the GNU GPL protect your rights with two steps:
** (1) assert copyright on the software, and (2) offer you this License
** giving you legal permission to copy, distribute and/or modify it.
** 
**   For the developers' and authors' protection, the GPL clearly explains
** that there is no warranty for this free software.  For both users' and
** authors' sake, the GPL requires that modified versions be marked as
** changed, so that their problems will not be attributed erroneously to
** authors of previous versions.
** 
**   Some devices are designed to deny users access to install or run
** modified versions of the software inside them, although the manufacturer
** can do so.  This is fundamentally incompatible with the aim of
** protecting users' freedom to change the software.  The systematic
** pattern of such abuse occurs in the area of products for individuals to
** use, which is precisely where it is most unacceptable.  Therefore, we
** have designed this version of the GPL to prohibit the practice for those
** products.  If such problems arise substantially in other domains, we
** stand ready to extend this provision to those domains in future versions
** of the GPL, as needed to protect the freedom of users.
** 
**   Finally, every program is threatened constantly by software patents.
** States should not allow patents to restrict development and use of
** software on general-purpose computers, but in those that do, we wish to
** avoid the special danger that patents applied to a free program could
** make it effectively proprietary.  To prevent this, the GPL assures that
** patents cannot be used to render the program non-free.
**
******************************************************************************/
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
    
    char *          pszErrorString;
    uint32_t        errorCode;
};

/******************************************************************************
**
** Name: lm_printStdUsage
**
** Description:
** Prints the usage message for macro programs with standard arguments.
** Arguments are processed by the lm_openWithStdArgs() function.
**
** Arguments:
**      pszProgramName      - The name of the macro program
**
** Returns: void
**
******************************************************************************/
void lm_printStdUsage(char * pszProgramName)
{
    printf("Usage:\n\n");
    printf("%s <options> input-file\n", pszProgramName);
    printf("\t-o <output file>\n");
    printf("\t-h print help\n\n");
}

/******************************************************************************
**
** Name: lm_openWithStdArgs
**
** Description:
** Calls lm_open() taking the source and target file names from the standard
** program arguments, as specified by lm_printStdUsage().
**
** Arguments:
**      argc                - The number of command line arguments
**      argv                - A pointer to an array of char * containing
**                            the program arguments
**
** Returns: A handle that is passed to all other lm_ library functions
**
******************************************************************************/
HTXTFILE lm_openWithStdArgs(int argc, char ** argv)
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
    if (pszOutputFile == NULL) {
        fprintf(stderr, "FATAL ERROR: No output file specified\n");
        exit(-1);
    }
    
    return lm_open(pszInputFile, pszOutputFile);
}

/******************************************************************************
**
** Name: lm_open
**
** Description:
** Opens the input and output text files and initialises the HTXTFILE handle
**
** Arguments:
**      pszSourceFilename   - The input file name to open
**      pszTargetFilename   - The output file name to open
**
** Returns: A handle that is passed to all other lm_ library functions
**
******************************************************************************/
HTXTFILE lm_open(const char * pszSourceFilename, const char * pszTargetFilename)
{
    HTXTFILE        htxt;

    htxt = (HTXTFILE)malloc(sizeof(struct _textHandle));

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

/******************************************************************************
**
** Name: lm_close
**
** Description:
** Close both input and output files, and free the handle
**
** Arguments:
**      htxt                - The handle
**
** Returns: void
**
******************************************************************************/
void lm_close(HTXTFILE htxt)
{
    fflush(htxt->fptrTarget);
    fclose(htxt->fptrTarget);
    fclose(htxt->fptrSource);

    if (htxt->pszErrorString != NULL) {
        free(htxt->pszErrorString);
    }

    free(htxt);
}

int lm_isEOF(HTXTFILE htxt)
{
    return (feof(htxt->fptrSource) != 0 ? 1 : 0);
}

void lm_rewind(HTXTFILE htxt)
{
    rewind(htxt->fptrSource);
}

char * lm_getLastError(HTXTFILE htxt)
{
    char * pszCopy = NULL;
    
    if (htxt->pszErrorString != NULL) {
        pszCopy = strdup(htxt->pszErrorString);
        free(htxt->pszErrorString);
    }

    return pszCopy;
}

int _find(HTXTFILE htxt, const char * pszFindStr, int writeOutput)
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
    ** the file pointer by (bufferLength - 1) to shift our 
    ** 'window' on 1 byte and read the next block... 
    */
    while (!isFound) {
        bytesRead = fread(szBuffer, 1, bufferLength, htxt->fptrSource);
        szBuffer[bytesRead] = 0;

        if (bytesRead == 0) {
            return 0;
        }
        
        isFound = (strncmp(szBuffer, pszFindStr, bufferLength) == 0 ? 1 : 0);

        if (!isFound) {
            if (writeOutput) {
                fputc(szBuffer[0], htxt->fptrTarget);
            }

            /*
            ** Rewind the file pointer to shift our window on
            ** by 1 byte...
            */
            fseek(htxt->fptrSource, -(bytesRead - 1), SEEK_CUR);
        }
    }

    fseek(htxt->fptrSource, -bytesRead, SEEK_CUR);

    return 1;
}

int lm_find(HTXTFILE htxt, const char * pszFindStr)
{
    return _find(htxt, pszFindStr, 1);
}

int lm_findReplace(HTXTFILE htxt, const char * pszFindStr, const char * pszReplaceStr)
{
    int         isFound;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        fseek(htxt->fptrSource, strlen(pszFindStr), SEEK_CUR);
        fwrite(pszReplaceStr, 1, strlen(pszReplaceStr), htxt->fptrTarget);
    }

    return isFound;    
}

int lm_findDeleteNum(HTXTFILE htxt, const char * pszFindStr, long numChars)
{
    int         isFound;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        fseek(htxt->fptrSource, numChars, SEEK_CUR);
    }

    return isFound;    
}

int lm_findDeleteToFound(HTXTFILE htxt, const char * pszFindStr)
{
    int         isFound;

    isFound = _find(htxt, pszFindStr, 0);

    return isFound;    
}

int lm_findDeleteLineEnd(HTXTFILE htxt, const char * pszFindStr)
{
    int         isFound;
    int         c;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        c = fgetc(htxt->fptrSource);

        while (c != '\n') {
            c = fgetc(htxt->fptrSource);
        }
    }

    return isFound;    
}

int lm_findDeleteFileEnd(HTXTFILE htxt, const char * pszFindStr)
{
    int         isFound;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        fseek(htxt->fptrSource, 0L, SEEK_END);
    }

    return isFound;    
}

int lm_findMoveNum(HTXTFILE htxt, const char * pszFindStr, long numChars)
{
    int         isFound;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        fseek(htxt->fptrSource, numChars, SEEK_CUR);
    }

    return isFound;    
}

int lm_findMoveLineEnd(HTXTFILE htxt, const char * pszFindStr)
{
    int         isFound;
    int         c;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        c = fgetc(htxt->fptrSource);

        while (c != '\n') {
            c = fgetc(htxt->fptrSource);
        }
    }

    return isFound;    
}

int lm_findMoveFileEnd(HTXTFILE htxt, const char * pszFindStr)
{
    int         isFound;

    isFound = _find(htxt, pszFindStr, 1);

    if (isFound) {
        fseek(htxt->fptrSource, 0L, SEEK_END);
    }

    return isFound;    
}
