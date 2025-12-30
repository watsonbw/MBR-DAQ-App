#include "data.hpp"

#include <chrono>

Data::Data() : m_Time{0.0f} {
  m_RPMData.reserve();
  m_ShockData.reserve();

  // Make this do something if you add more member variables
}

void Data::PopulateData([[maybe_unused]] const char *esp32_data) {
  // Empty the data buffers but retain their capacity
  m_RPMData.clear();
  m_ShockData.clear();

  // Do something fun with the raw bytes to parse
}
