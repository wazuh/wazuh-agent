#pragma once

#include "CoreFoundation/CFBase.h"
#include "IOKit/IOKitLib.h"

class IOsPrimitivesMac
{
public:
    /// @brief Default destructor
    virtual ~IOsPrimitivesMac() = default;

    /// @brief sysctl wrapper
    /// @param name sysctl name
    /// @param namelen sysctl name length
    /// @param oldp old value
    /// @param oldlenp old value length
    /// @param newp new value
    /// @param newlen new value length
    /// @return 0 on success, -1 otherwise
    virtual int sysctl(int* name, u_int namelen, void* oldp, size_t* oldlenp, void* newp, size_t newlen) const = 0;

    /// @brief sysctlbyname wrapper
    /// @param name sysctl name
    /// @param oldp old value
    /// @param oldlenp old value length
    /// @param newp new value
    /// @param newlen new value length
    /// @return 0 on success, -1 otherwise
    virtual int sysctlbyname(const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen) const = 0;

    /// @brief IOServiceMatching wrapper
    /// @param name sysctl name
    /// @return CFMutableDictionaryRef
    virtual CFMutableDictionaryRef IOServiceMatching(const char* name) const = 0;

    /// @brief IOServiceGetMatchingServices wrapper
    /// @param mainPort main port
    /// @param matching matching dictionary
    /// @param existing iterator
    /// @return kern_return_t
    virtual kern_return_t
    IOServiceGetMatchingServices(mach_port_t mainPort, CFDictionaryRef matching, io_iterator_t* existing) const = 0;

    /// @brief IOIteratorNext wrapper
    /// @param iterator iterator
    /// @return io_object_t
    virtual io_object_t IOIteratorNext(io_iterator_t iterator) const = 0;

    /// @brief IORegistryEntryGetName wrapper
    /// @param entry entry
    /// @param name name
    /// @return kern_return_t
    virtual kern_return_t IORegistryEntryGetName(io_registry_entry_t entry, io_name_t name) const = 0;

    /// @brief IORegistryEntryCreateCFProperties wrapper
    /// @param entry entry
    /// @param properties properties
    /// @param allocator allocator
    /// @param options options
    /// @return kern_return_t
    virtual kern_return_t IORegistryEntryCreateCFProperties(io_registry_entry_t entry,
                                                            CFMutableDictionaryRef* properties,
                                                            CFAllocatorRef allocator,
                                                            IOOptionBits options) const = 0;

    /// @brief IOObjectRelease wrapper
    /// @param object object
    /// @return kern_return_t
    virtual kern_return_t IOObjectRelease(io_object_t object) const = 0;

    /// @brief CFStringCreateWithCString wrapper
    /// @param alloc allocator
    /// @param cStr c string
    /// @param encoding encoding
    /// @return CFStringRef
    virtual CFStringRef
    CFStringCreateWithCString(CFAllocatorRef alloc, const char* cStr, CFStringEncoding encoding) const = 0;

    /// @brief CFDictionaryGetValue wrapper
    /// @param theDict dictionary
    /// @param key key
    /// @return const void* value
    virtual const void* CFDictionaryGetValue(CFDictionaryRef theDict, const void* key) const = 0;

    /// @brief CFGetTypeID wrapper
    /// @param cf cf
    /// @return CFTypeID
    virtual CFTypeID CFGetTypeID(CFTypeRef cf) const = 0;

    /// @brief CFDataGetTypeID wrapper
    /// @return CFTypeID
    virtual CFTypeID CFDataGetTypeID(void) const = 0;

    /// @brief CFDataGetLength wrapper
    /// @param theData data
    /// @return CFIndex
    virtual CFIndex CFDataGetLength(CFDataRef theData) const = 0;

    /// @brief CFDataGetBytes wrapper
    /// @param theData data
    /// @param range range
    /// @param buffer buffer
    virtual void CFDataGetBytes(CFDataRef theData, CFRange range, UInt8* buffer) const = 0;

    /// @brief CFRangeMake wrapper
    /// @param loc location
    /// @param len length
    /// @return CFRange
    virtual CFRange CFRangeMake(CFIndex loc, CFIndex len) const = 0;

    /// @brief CFRelease wrapper
    /// @param cf cf
    virtual void CFRelease(CFTypeRef cf) const = 0;
};
