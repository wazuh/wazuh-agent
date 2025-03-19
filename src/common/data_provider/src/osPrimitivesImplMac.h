#pragma once

#include "osPrimitivesInterfaceMac.h"

class OsPrimitivesMac : public IOsPrimitivesMac
{
public:
    /// @brief Default constructor
    OsPrimitivesMac() = default;

    /// @brief Default destructor
    virtual ~OsPrimitivesMac() = default;

    /// @copydoc IOsPrimitivesMac::sysctl
    int sysctl(int* name, u_int namelen, void* oldp, size_t* oldlenp, void* newp, size_t newlen) const override
    {
        return ::sysctl(name, namelen, oldp, oldlenp, newp, newlen);
    }

    /// @copydoc IOsPrimitivesMac::sysctlbyname
    int sysctlbyname(const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen) const override
    {
        return ::sysctlbyname(name, oldp, oldlenp, newp, newlen);
    }

    /// @copydoc IOsPrimitivesMac::IOServiceMatching
    CFMutableDictionaryRef IOServiceMatching(const char* name) const override
    {
        return ::IOServiceMatching(name);
    }

    /// @copydoc IOsPrimitivesMac::IOServiceGetMatchingServices
    kern_return_t
    IOServiceGetMatchingServices(mach_port_t mainPort, CFDictionaryRef matching, io_iterator_t* existing) const override
    {
        return ::IOServiceGetMatchingServices(mainPort, matching, existing);
    }

    /// @copydoc IOsPrimitivesMac::IOIteratorNext
    io_object_t IOIteratorNext(io_iterator_t iterator) const override
    {
        return ::IOIteratorNext(iterator);
    }

    /// @copydoc IOsPrimitivesMac::IORegistryEntryGetName
    kern_return_t IORegistryEntryGetName(io_registry_entry_t entry, io_name_t name) const override
    {
        return ::IORegistryEntryGetName(entry, name);
    }

    /// @copydoc IOsPrimitivesMac::IORegistryEntryCreateCFProperties
    kern_return_t IORegistryEntryCreateCFProperties(io_registry_entry_t entry,
                                                    CFMutableDictionaryRef* properties,
                                                    CFAllocatorRef allocator,
                                                    IOOptionBits options) const override
    {
        return ::IORegistryEntryCreateCFProperties(entry, properties, allocator, options);
    }

    /// @copydoc IOsPrimitivesMac::IOObjectRelease
    kern_return_t IOObjectRelease(io_object_t object) const override
    {
        return ::IOObjectRelease(object);
    }

    /// @copydoc IOsPrimitivesMac::CFStringCreateWithCString
    CFStringRef
    CFStringCreateWithCString(CFAllocatorRef alloc, const char* cStr, CFStringEncoding encoding) const override
    {
        return ::CFStringCreateWithCString(alloc, cStr, encoding);
    }

    /// @copydoc IOsPrimitivesMac::CFDictionaryGetValue
    const void* CFDictionaryGetValue(CFDictionaryRef theDict, const void* key) const override
    {
        return ::CFDictionaryGetValue(theDict, key);
    }

    /// @copydoc IOsPrimitivesMac::CFGetTypeID
    CFTypeID CFGetTypeID(CFTypeRef cf) const override
    {
        return ::CFGetTypeID(cf);
    }

    /// @copydoc IOsPrimitivesMac::CFDataGetTypeID
    CFTypeID CFDataGetTypeID(void) const override
    {
        return ::CFDataGetTypeID();
    }

    /// @copydoc IOsPrimitivesMac::CFDataGetLength
    CFIndex CFDataGetLength(CFDataRef theData) const override
    {
        return ::CFDataGetLength(theData);
    }

    /// @copydoc IOsPrimitivesMac::CFDataGetBytes
    void CFDataGetBytes(CFDataRef theData, CFRange range, UInt8* buffer) const override
    {
        return ::CFDataGetBytes(theData, range, buffer);
    }

    /// @copydoc IOsPrimitivesMac::CFRangeMake
    CFRange CFRangeMake(CFIndex loc, CFIndex len) const override
    {
        return ::CFRangeMake(loc, len);
    }

    /// @copydoc IOsPrimitivesMac::CFRelease
    void CFRelease(CFTypeRef cf) const override
    {
        ::CFRelease(cf);
    }
};
