#pragma once

#include <memory>

namespace mbr {

struct app_context;
class gui_t;

class app_t {
  public:
    explicit app_t(int arc, char* argv[]);
    ~app_t();

    void run();

  private:
    std::unique_ptr<gui_t>       gui_;
    std::shared_ptr<app_context> context_;
};

} // namespace mbr
