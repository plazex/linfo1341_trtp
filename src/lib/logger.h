/*!
 *  Logger file which handles io to file redirection and printing.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "utils.h"

#include <stdio.h>

/*! Logger file */
extern FILE* log_file;
/*! Stat file */
extern FILE* stat_file;
/*! stdout file */
extern FILE* stdout_file;
/*! Stat type string array */
extern const char* stats[15];

/*!
 * Open the logger file.
 * \param filename : logger file name
 */
void initLog(const char* filename);

/*!
 * Open the stat file.
 * \param filename : stat file name
 */
void initStat(const char* filename);

/*!
 * Redirect the standard output to the given file.
 * \param filename : file name
 */
void initStdout(const char* filename);

/*!
 * Close the logger and stat files if they are opened.
 */
void closeFiles();

/*!
 * Print the stat related to the given type.
 * \param type : stat type
 * \param value : stat value
 */
void printStat(int type, int value);

/*!
 * Open a file with the given file name and print the content
 * in the standard output.
 * \param filename : file name to print
 */
void printFile(const char* filename);

#endif //LOGGER_H