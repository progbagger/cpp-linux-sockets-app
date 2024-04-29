#include "include/interrupt.h"

#include <string>

std::string Interrupted::what() const { return "received ctrl+c signal"; }
