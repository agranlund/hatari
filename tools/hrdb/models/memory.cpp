#include "memory.h"

#include <string.h>

Memory::Memory(uint32_t addr, uint32_t size) :
    m_addr(addr),
    m_size(size)
{
    m_pData = new uint8_t[size];
}

Memory::~Memory()
{
    delete [] m_pData;
}

void Memory::Clear()
{
    delete [] m_pData;
    m_pData = nullptr;
    m_addr = 0;
    m_size = 0;
}

bool Memory::ReadAddressMulti(uint32_t address, uint32_t numBytes, uint32_t& value) const
{
    value = 0U;
    if (!HasAddressRange(address, numBytes))
        return false;

    // Check that all the bytes are available in this block.
    // Shift "offset" into the realms of this memory block
    uint32_t offset = address - m_addr;
    assert(offset + numBytes <= m_size);
    uint32_t longContents = 0;
    for (uint32_t i = 0; i < numBytes; ++i)
    {
        longContents <<= 8;
        longContents += m_pData[offset + i];
    }
    value = longContents;
    return true;
}

Memory& Memory::operator=(const Memory &other)
{
    this->m_size = other.m_size;
    this->m_addr = other.m_addr;
    delete [] this->m_pData;

    this->m_pData = new uint8_t[other.m_size];
    memcpy(this->m_pData, other.m_pData, other.m_size);
    return *this;
}

bool Overlaps(uint32_t addr1, uint32_t size1, uint32_t addr2, uint32_t size2)
{
    // Case 1: block one entirely to left of block two
    if (addr1 + size1 <= addr2)
        return false;
    // Case 2: block two entirely to left of block one
    if (addr2 + size2 <= addr1)
        return false;
    // Case 3: they overlap!
    return true;
}
