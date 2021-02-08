#pragma once

// Create default constructor but disallow copy and assign
#define DEFAULT_NOCOPY_NOASSIGN(TypeName) \
    TypeName() = default;      \
    TypeName(const TypeName&) = delete;      \
    void operator=(const TypeName&) = delete

// disallow copy and assign constructors
#define NOCOPY_NOASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;      \
    void operator=(const TypeName&) = delete


// Create default constructor but disallow assign
#define DEFAULT_NOASSIGN(TypeName) \
    TypeName() = default;      \
    void operator=(const TypeName&) = delete
