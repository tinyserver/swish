/**
    @file

    Tests for CProvider.

    @if license

    Copyright (C) 2010, 2012  Alexander Lamaison <awl03@doc.ic.ac.uk>

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

#include "swish/provider/sftp_provider.hpp" // sftp_provider, listing

#include "test/common_boost/helpers.hpp"
#include "test/common_boost/ProviderFixture.hpp" // ProviderFixture

#include <comet/bstr.h> // bstr_t
#include <comet/error.h> // com_error
#include <comet/ptr.h> // com_ptr

#include <boost/filesystem/path.hpp> // wpath
#include <boost/filesystem/fstream.hpp> // ofstream taking a path
#include <boost/range/size.hpp>

#include <boost/test/unit_test.hpp>

#include <exception>
#include <string>
#include <vector>

using test::ProviderFixture;

using swish::provider::sftp_provider;
using swish::provider::directory_listing;

using comet::bstr_t;
using comet::com_error;
using comet::com_error_from_interface;
using comet::com_ptr;

using boost::filesystem::ofstream;
using boost::filesystem::wpath;
using boost::shared_ptr;
using boost::test_tools::predicate_result;

using std::exception;
using std::string;
using std::wstring;


namespace {

    const string longentry = 
        "-rw-r--r--    1 swish    wheel         767 Dec  8  2005 .cshrc";

}

BOOST_FIXTURE_TEST_SUITE(provider_tests, ProviderFixture)

/**
 * List contents of an empty directory.
 */
BOOST_AUTO_TEST_CASE( list_empty_dir )
{
    shared_ptr<sftp_provider> provider = Provider();
    BOOST_CHECK_EQUAL(
        boost::size(provider->listing(Consumer(), ToRemotePath(Sandbox()))),
        0U);
}

/**
 * List contents of a directory.
 */
BOOST_AUTO_TEST_CASE( list_dir )
{
    wpath file1 = NewFileInSandbox();
    wpath file2 = NewFileInSandbox();

    directory_listing listing =
        Provider()->listing(Consumer(), ToRemotePath(Sandbox()));

    BOOST_CHECK_EQUAL(boost::size(listing), 2U);
    BOOST_CHECK_EQUAL(listing[0].filename(), file1.filename());
    BOOST_CHECK_EQUAL(listing[1].filename(), file2.filename());
}

namespace {

    predicate_result is_failed_to_open(const exception& e)
    {
        string expected = "Failed opening remote file: FX_NO_SUCH_FILE";
        string actual = e.what();

        if (expected != actual)
        {
            predicate_result res(false);
            res.message()
                << "Exception is not failure to open [" << expected
                << " != " << actual << "]";
            return res;
        }

        return true;
    }

}

/**
 * Try to list non-existent directory.
 */
BOOST_AUTO_TEST_CASE( list_dir_error )
{
    shared_ptr<sftp_provider> provider = Provider();

    BOOST_CHECK_EXCEPTION(
        provider->listing(Consumer(), L"/i/dont/exist"),
        exception, is_failed_to_open);
}

/**
 * Can we handle a unicode filename?
 */
BOOST_AUTO_TEST_CASE( unicode )
{
    // create an empty file with a unicode filename in the sandbox
    wpath unicode_file_name = Sandbox() / L"русский";
    ofstream(unicode_file_name).close();
    BOOST_CHECK(exists(unicode_file_name));
    BOOST_CHECK(is_regular_file(unicode_file_name));
    BOOST_CHECK(unicode_file_name.is_complete());

    directory_listing listing =
        Provider()->listing(Consumer(), ToRemotePath(Sandbox()));

    BOOST_CHECK_EQUAL(
        listing[0].filename(), unicode_file_name.filename());
}

/**
 * Can we see inside directories whose names are non-latin Unicode?
 */
BOOST_AUTO_TEST_CASE( list_unicode_dir )
{
    wpath directory = Sandbox() / L"漢字 العربية русский 47";
    create_directory(directory);
    wpath file = directory / L"latin filename";
    ofstream(file).close();

    Provider()->listing(Consumer(), ToRemotePath(directory));
}

/**
 * Rename a file.
 */
BOOST_AUTO_TEST_CASE( rename_file )
{
    wpath file = NewFileInSandbox();
    wpath renamed_file = file.parent_path() / (file.filename() + L"renamed");

    shared_ptr<sftp_provider> provider = Provider();

    bstr_t old_name(ToRemotePath(file).string());
    bstr_t new_name(ToRemotePath(renamed_file).string());

    BOOST_CHECK_EQUAL(
        provider->rename(Consumer().get(), old_name.in(), new_name.in()),
        VARIANT_FALSE);
    BOOST_CHECK(exists(renamed_file));
    BOOST_CHECK(!exists(file));
}

/**
 * Rename a Unicode file.
 */
BOOST_AUTO_TEST_CASE( rename_unicode_file )
{
    // create an empty file with a unicode filename in the sandbox
    wpath unicode_file_name = Sandbox() / L"русский.txt";
    ofstream(unicode_file_name).close();
    BOOST_CHECK(exists(unicode_file_name));

    wpath renamed_file = Sandbox() / L"Россия";

    shared_ptr<sftp_provider> provider = Provider();

    bstr_t old_name(ToRemotePath(unicode_file_name).string());
    bstr_t new_name(ToRemotePath(renamed_file).string());

    BOOST_CHECK_EQUAL(
        provider->rename(Consumer().get(), old_name.in(), new_name.in()),
        VARIANT_FALSE);

    BOOST_CHECK(exists(renamed_file));
    BOOST_CHECK(!exists(unicode_file_name));
}

/**
 * Delete a file and ensure other files in the same folder aren't also removed.
 */
BOOST_AUTO_TEST_CASE( delete_file )
{
    wpath file_before = NewFileInSandbox();
    wpath file = NewFileInSandbox();
    wpath file_after = NewFileInSandbox();

    Provider()->delete_file(
        Consumer().get(), bstr_t(ToRemotePath(file).string()).in());

    BOOST_CHECK(exists(file_before));
    BOOST_CHECK(!exists(file));
    BOOST_CHECK(exists(file_after));
}

/**
 * Delete a file with a unicode filename.
 */
BOOST_AUTO_TEST_CASE( delete_unicode_file )
{
    wpath unicode_file_name = Sandbox() / L"العربية.txt";
    ofstream(unicode_file_name).close();

    Provider()->delete_file(
        Consumer().get(),
        bstr_t(ToRemotePath(unicode_file_name).string()).in());

    BOOST_CHECK(!exists(unicode_file_name));
}

/**
 * Delete an empty directory.
 */
BOOST_AUTO_TEST_CASE( delete_empty_directory )
{
    wpath directory = Sandbox() / L"العربية";
    create_directory(directory);

    Provider()->delete_directory(
        Consumer().get(), bstr_t(ToRemotePath(directory).string()).in());

    BOOST_CHECK(!exists(directory));
}

/**
 * Delete a non-empty directory.  This is trickier as the contents have to be
 * deleted before the directory.
 */
BOOST_AUTO_TEST_CASE( delete_directory_recursively )
{
    wpath directory = Sandbox() / L"العربية";
    create_directory(directory);
    wpath file = directory / L"русский.txt";
    ofstream(file).close();

    Provider()->delete_directory(
        Consumer().get(), bstr_t(ToRemotePath(directory).string()).in());

    BOOST_CHECK(!exists(directory));
}

/**
 * Create a file with a unicode filename.
 */
BOOST_AUTO_TEST_CASE( create_file )
{
    wpath file = Sandbox() / L"漢字 العربية русский.txt";
    BOOST_CHECK(!exists(file));

    Provider()->create_new_file(
        Consumer().get(), bstr_t(ToRemotePath(file).string()).in());

    BOOST_CHECK(exists(file));
}

/**
 * Create a file that already exists in the directory.
 */
BOOST_AUTO_TEST_CASE( create_file_overwrite )
{
    wpath file = Sandbox() / L"漢字 العربية русский.txt";
    ofstream(file).close();
    BOOST_CHECK(exists(file));

    Provider()->create_new_file(
        Consumer().get(), bstr_t(ToRemotePath(file).string()).in());

    BOOST_CHECK(exists(file));
}

/**
 * Create a directory with a unicode filename.
 */
BOOST_AUTO_TEST_CASE( create_directory )
{
    wpath file = Sandbox() / L"漢字 العربية русский 47";
    BOOST_CHECK(!exists(file));

    Provider()->create_new_directory(
        Consumer().get(), bstr_t(ToRemotePath(file).string()).in());

    BOOST_CHECK(exists(file));
}

/**
 * Create a stream to a file with a unicode filename.
 *
 * Tests file creation as we don't create the file before the call and we
 * check that it exists after.
 */
BOOST_AUTO_TEST_CASE( get_file_stream )
{
    wpath file = Sandbox() / L"漢字 العربية русский.txt";
    BOOST_CHECK(!exists(file));

    com_ptr<IStream> stream = Provider()->get_file(
        Consumer(), ToRemotePath(file).string(), true);

    BOOST_CHECK(stream);
    BOOST_CHECK(exists(file));

    STATSTG statstg = STATSTG();
    HRESULT hr = stream->Stat(&statstg, STATFLAG_DEFAULT);
    BOOST_CHECK_EQUAL(statstg.pwcsName, file.filename());
    ::CoTaskMemFree(statstg.pwcsName);
    BOOST_REQUIRE_OK(hr);
}

/**
 * Try to get a read-only stream to a non-existent file.
 *
 * This must fail as out DropTarget uses it to check whether the file already
 * exists.
 */
BOOST_AUTO_TEST_CASE( get_file_stream_fail )
{
    wpath file = Sandbox() / L"漢字 العربية русский.txt";
    BOOST_CHECK(!exists(file));

    BOOST_CHECK_THROW(
        Provider()->get_file(Consumer(), ToRemotePath(file).string(), false),
        exception);

    BOOST_CHECK(!exists(file));
}

BOOST_AUTO_TEST_SUITE_END()
