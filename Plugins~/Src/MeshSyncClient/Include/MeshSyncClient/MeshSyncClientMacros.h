#pragma once

// Disallow the copy constructor and operator= functions. 
// For use in private: declarations 
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;      \
    void operator=(const TypeName&) = delete


