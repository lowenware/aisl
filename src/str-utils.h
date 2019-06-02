/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file str-utils.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief String utilities header file
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef STR_UTILS_H_799148B5_F829_437C_8B96_55876A37C38E
#define STR_UTILS_H_799148B5_F829_437C_8B96_55876A37C38E


char *
str_copy(const char *source);

int
str_cmpi(const char *anycase, const char *lowcase);

#endif /* !STR_UTILS_H */
