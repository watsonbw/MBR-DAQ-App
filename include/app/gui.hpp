#pragma once
#include <memory>
#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <qlabel.h>
#include <qtmetamacros.h>
#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "app/backend_bridge.hpp"
#include <stdx/assert.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

namespace mbr {

class gui_t : public QMainWindow {
    Q_OBJECT
public:
    explicit gui_t(const std::shared_ptr<app_context>& ctx);
    ~gui_t() = default;

private:
    void change_page(page_type_t type);
    void build_menu_bar();
    void update_status_dot();
    [[nodiscard]] pages::page* create_page(page_type_t type, const std::shared_ptr<app_context>& ctx, QWidget* parent);

private:
    backend_bridge bridge_;
    QStackedWidget*              pages_;
    std::shared_ptr<app_context> context_;
    ankerl::unordered_dense::map<page_type_t, QWidget*> page_lookup_;
    QLabel* connection_status_;
    QLabel* status_dot_ = nullptr;
    bool    is_connected_ = false;
    bool    is_receiving_ = false;
};

} // namespace mbr
