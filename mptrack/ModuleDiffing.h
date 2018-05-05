/*
* ModuleDiffing.h
* ---------------
* Purpose: Implements functionality for comparing(diffing) module files that can be used e.g. with version control diffing tools.
*
* Logical dependencies in functionality:
*   Audio engine.......: no
*   GUI/MFC............: no
*   Module file reading: yes
*/

#pragma once

#include <iosfwd>

namespace mpt
{
	class PathString;
}

OPENMPT_NAMESPACE_BEGIN

// Note: The diffable text is not guaranteed to be a complete representation of the module (i.e. different modules may have identical text representation).
void ModuleToDiffableText(const mpt::PathString& path, std::ostream& strm);

OPENMPT_NAMESPACE_END
