/**
    @file

    Swish version information.

    @if license

    Copyright (C) 2013  Alexander Lamaison <awl03@doc.ic.ac.uk>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    If you modify this Program, or any covered work, by linking or
    combining it with the OpenSSL project's OpenSSL library (or a
    modified version of that library), containing parts covered by the
    terms of the OpenSSL or SSLeay licenses, the licensors of this
    Program grant you additional permission to convey the resulting work.

    @endif
*/

#include "version.hpp"

#include "swish/version/git_version.hpp" // generated

#include <sstream> // ostringstream

using std::auto_ptr;
using std::ostringstream;
using std::string;

namespace swish {

string snapshot_version()
{
    return git_version;
}

std::string build_time()
{
    return __TIME__;
}

std::string build_date()
{
    return __DATE__;
}

/*
class structured_version_impl
{
public:

    virtual ~structured_version_impl() {}

    virtual int major() const = 0;
    virtual int minor() const = 0;
    virtual int bugfix() const = 0;

    virtual std::string as_string() const = 0;
};
*/

structured_version::structured_version(const structured_version_impl& impl)
    : m_pimpl(impl.clone()) {}

structured_version::structured_version(const structured_version& other)
    : m_pimpl(other.m_pimpl->clone()) {}

structured_version& structured_version::operator=(structured_version other)
{
    swap(*this, other);
    return *this;
}

int structured_version::major() const { return m_pimpl->major(); }
int structured_version::minor() const { return m_pimpl->minor(); }
int structured_version::bugfix() const { return m_pimpl->bugfix(); }
string structured_version::as_string() const { return m_pimpl->as_string(); }

structured_version release_version()
{
    class swish_version : public structured_version_impl
    {
    public:
        virtual int major() const
        {
            return 0;
        }

        virtual int minor() const
        {
            return 7;
        }

        virtual int bugfix() const
        {
            return 2;
        }

        virtual string as_string() const
        {
            ostringstream formatted;
            formatted << major() << "." << minor() << "." << bugfix();
            return formatted.str();
        }

        virtual auto_ptr<structured_version_impl> clone() const
        {
            return auto_ptr<structured_version_impl>(new swish_version(*this));
        }
    };

    return structured_version(swish_version());
}

}