#ifndef NAMEDITEM_HPP
#define NAMEDITEM_HPP

#include "Pex_global.hpp"

#include <cstdint>

#include "StringTable.hpp"

namespace Pex {

/**
 * @brief Base Mixin for named item.
 *
 * This mixin defines the name field for named elements
 */
class PEX_API NamedItem
{
public:
    NamedItem();
    virtual ~NamedItem();

    StringTable::Index getName() const;
    void setName(StringTable::Index value);

protected:
    StringTable::Index m_Name;
};
}

#endif // NAMEDITEM_HPP
