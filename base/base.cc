#include "base/base.h"

#include <fstream>

#include "glog/logging.h"
#include <iostream>

std::string ReadFile(absl::string_view filename) {
  // http://www.cplusplus.com/reference/istream/istream/read/
  std::ifstream is(std::string(filename), std::ifstream::binary);
  std::string buffer;

  LOG_IF(FATAL, !is) << "failed to read file " << filename;

  // get length of file:
  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  buffer.resize(length);

  // read data as a block:
  is.read(&buffer[0], length);

  LOG_IF(FATAL, !is) << "error: only " << is.gcount() << " could be read";

  return buffer;
}
