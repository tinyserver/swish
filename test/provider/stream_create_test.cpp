/**
    @file

    Unit tests for CSftpStream exercising stream creation.

    @if license

    Copyright (C) 2009, 2011  Alexander Lamaison <awl03@doc.ic.ac.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    In addition, as a special exception, the the copyright holders give you
    permission to combine this program with free software programs or the 
    OpenSSL project's "OpenSSL" library (or with modified versions of it, 
    with unchanged license). You may copy and distribute such a system 
    following the terms of the GNU GPL for this program and the licenses 
    of the other code concerned. The GNU General Public License gives 
    permission to release a modified version without this exception; this 
    exception also makes it possible to release a modified version which 
    carries forward this exception.

    @endif
*/

#include "swish/provider/SftpStream.hpp"  // Test subject

#include "test/provider/StreamFixture.hpp"

#include <comet/error.h> // com_error

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

using test::provider::StreamFixture;

using comet::com_error;

BOOST_FIXTURE_TEST_SUITE(StreamCreate, StreamFixture)

/**
 * Open a stream to a file that doesn't already exist.
 * The file should be created as the CSftpStream::create flag is set.
 */
BOOST_AUTO_TEST_CASE( new_file )
{
    // Delete sandbox file before creating stream
    remove(m_local_path);

    BOOST_REQUIRE(!exists(m_local_path));

    GetStream(CSftpStream::create);

    BOOST_REQUIRE(exists(m_local_path));
}

/**
 * Open a stream to a file that doesn't already exist.
 * This should fail and the file should not be created as the 
 * CSftpStream::create flag isn't set.
 */
BOOST_AUTO_TEST_CASE( new_file_fail )
{
    // Delete sandbox file before creating stream
    remove(m_local_path);

    BOOST_REQUIRE(!exists(m_local_path));

    BOOST_REQUIRE_THROW(GetStream(), com_error);

    BOOST_REQUIRE(!exists(m_local_path));
}

BOOST_AUTO_TEST_SUITE_END()
