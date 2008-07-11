/*  Component that handles Shell Folder View interaction with Explorer.

    Copyright (C) 2008  Alexander Lamaison <awl03@doc.ic.ac.uk>

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
*/

#ifndef EXPLORERCALLBACK_H
#define EXPLORERCALLBACK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma once
#include "stdafx.h"
#include "resource.h"       // main symbols

// CExplorerCallback
[
	coclass,
	// default(IShellFolderViewCB) causes unsatisfied forward declaration error
	default(IUnknown), 
	threading(apartment),
	vi_progid("Swish.ExplorerCallback"),
	progid("Swish.ExplorerCallback.1"),
	version(1.0),
	uuid("b816a848-5022-11dc-9153-0090f5284f85"),
	helpstring("ExplorerCallback Class")
]
class ATL_NO_VTABLE CExplorerCallback :
	public IShellFolderViewCB
{
public:

	// IShellFolderViewCB
	IFACEMETHODIMP MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	// Menu command ID offsets for Explorer Tools menu
	enum MENUOFFSET
	{
		MENUIDOFFSET_FIRST = 0,
		MENUIDOFFSET_ADD = MENUIDOFFSET_FIRST,
		MENUIDOFFSET_REMOVE,
		MENUIDOFFSET_LAST = MENUIDOFFSET_REMOVE
	};
};

#endif // EXPLORERCALLBACK_H