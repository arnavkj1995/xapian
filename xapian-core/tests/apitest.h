/** @file apitest.h
 * @brief test functionality of the Xapian API
 */
/* Copyright (C) 2007,2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_APITEST_H
#define XAPIAN_INCLUDED_APITEST_H

#include <xapian.h>

#include "testsuite.h"

#define DEFINE_TESTCASE(S,COND) bool test_##S()

const char * get_dbtype();

Xapian::Database get_database(const std::string &db);

Xapian::Database get_database(const std::string &db1, const std::string &db2);

Xapian::Database get_database(const std::string &db,
			      void (*gen)(Xapian::WritableDatabase&,
					  const std::string &),
			      const std::string &arg = std::string());

std::string get_database_path(const std::string &db);

std::string get_database_path(const std::string &db,
			      void (*gen)(Xapian::WritableDatabase&,
					  const std::string &),
			      const std::string &arg = std::string());

Xapian::WritableDatabase get_writable_database(const std::string &db = "");

Xapian::WritableDatabase get_named_writable_database(const std::string &name, const std::string &source = "");

std::string get_named_writable_database_path(const std::string &name);

Xapian::Database get_remote_database(const std::string &db, unsigned timeout);

Xapian::Database get_writable_database_as_database();

Xapian::WritableDatabase get_writable_database_again();

#define SKIP_TEST_UNLESS_BACKEND(B) \
    if (strcmp(get_dbtype(), (B)) != 0) SKIP_TEST("Test only supported for "#B" backend"); else (void)0

#define SKIP_TEST_FOR_BACKEND(B) \
    if (strcmp(get_dbtype(), (B)) == 0) SKIP_TEST("Test not supported for "#B" backend"); else (void)0

#endif // XAPIAN_INCLUDED_APITEST_H