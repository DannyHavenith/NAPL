#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H

#include <iostream>
#include <sstream>



// Variadic function to print arguments to std::cerr
template<typename... Args>
void debugprint(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args)); // Fold expression to concatenate arguments
    std::cerr << oss.str() << std::endl;
}

#endif // DEBUGPRINT_H
