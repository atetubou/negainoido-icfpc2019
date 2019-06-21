#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#include <string>

#include "absl/strings/string_view.h"

std::string ReadFile(absl::string_view filename);

#endif // BASE_BASE_H_
