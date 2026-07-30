#include "app/gui.hpp"

#include <memory>
#include <qapplication.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <string>

#include <fmt/format.h>
#include <gsl/pointers>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QStackedWidget>
#include <QWidget>

#include "app/assets/images/app_icon.hpp"
#include "app/assets/style.hpp"
#include "app/backend_bridge.hpp"
#include "app/context.hpp"
#include "app/pages/analysis.hpp"
#include "app/pages/home.hpp"
#include "app/pages/page.hpp"
#include "app/pages/plot.hpp"
#include "app/pages/serialmon.hpp"
#include "app/pages/settings.hpp"
#include "core/time.hpp"
#include "esp32/backend.hpp"
#include "stdx/enum.hh"

using namespace std::chrono;

namespace mbr {

namespace style  = ui::style;
namespace colors = ui::style::color;
namespace page   = ui::pages;

gui_t::gui_t(const std::shared_ptr<app_context>& ctx) : context_(ctx) {
    pages_ = new QStackedWidget(this);
    setCentralWidget(pages_);
    for (auto type : stdx::enum_range<page_type_t>()) {
        auto* p = create_page(type, context_, pages_);
        pages_->addWidget(p);
        page_lookup_[type] = p;
    }
    change_page(page_type_t::HOME);
    build_menu_bar();
}

[[nodiscard]] page::page*
gui_t::create_page(page_type_t type, const std::shared_ptr<app_context>& ctx, QWidget* parent) {
    switch (type) {
    case page_type_t::HOME:           return new page::home_page(ctx, parent);
    case page_type_t::PLOT:           return new page::plot_page(ctx, parent);
    case page_type_t::ANALYSIS:       return new page::analysis_page(ctx, parent);
    case page_type_t::SERIAL_MONITOR: return new page::serial_page(ctx, parent);
    case page_type_t::SETTINGS:       return new page::settings_page(ctx, parent);
    default:                          return nullptr;
    }
}

void gui_t::change_page(page_type_t type) { pages_->setCurrentWidget(page_lookup_[type]); }

void gui_t::build_menu_bar() {

    menuBar()->setStyleSheet(style::make_menubar_style());

    auto* file = menuBar()->addMenu("File");
    file->setStyleSheet(style::make_menu_style());
    file->addAction("Settings");
    file->addAction("Export Log");
    file->addAction("Exit", this, [] { QApplication::quit(); });

    auto* nav = menuBar()->addMenu("Navigation");
    nav->setStyleSheet(style::make_menu_style());
    nav->addAction("Home", this, [this] { change_page(page_type_t::HOME); });
    nav->addAction("Plot", this, [this] { change_page(page_type_t::PLOT); });
    nav->addAction("Serial", this, [this] { change_page(page_type_t::SERIAL_MONITOR); });
    nav->addAction("Analysis", this, [this] { change_page(page_type_t::ANALYSIS); });

    menuBar()->addAction("Sync Time", this, [this] {
        local_time lt;
        const auto sync_time = lt.micros_since_midnight();
        context_->backend->send_cmd(std::format("CMD SYNC {}", sync_time));
    });

    menuBar()->addAction("Restart Connection", this, [this] { context_->backend->restart(); });

    menuBar()->addAction("Clear Data", this, [this] { context_->backend->get_data().clear(); });

    auto* rightWidget = new QWidget(menuBar());
    auto* rightLayout = new QHBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 4, 8, 4);
    rightLayout->setSpacing(8);

    connection_status_ = new QLabel("Disconnected", rightWidget);
    connection_status_->setStyleSheet("font-size: 20px;");
    connection_status_->setMinimumWidth(120);
    connection_status_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(context_->bridge.get(),
            &ui::bridge::backend_bridge::connection_changed,
            this,
            [this](bool connected) {
                is_connected_ = connected;
                if (!connected) { is_receiving_ = false; }
                connection_status_->setText(connected ? "Connected" : "Disconnected");
                update_status_dot();
            });

    connect(context_->bridge.get(),
            &ui::bridge::backend_bridge::receiving_changed,
            this,
            [this](bool receiving) {
                is_receiving_ = receiving;
                update_status_dot();
            });
    connection_status_->setStyleSheet("font-size: 20px;");

    status_dot_ = new QLabel(rightWidget);
    status_dot_->setFixedSize(20, 20);
    status_dot_->setStyleSheet(QString::fromStdString(fmt::format(
        "background-color: {0}; border-radius: 10px;", colors::status_error.name().toStdString())));
    rightLayout->addWidget(connection_status_, 0, Qt::AlignVCenter);
    rightLayout->addWidget(status_dot_, 0, Qt::AlignVCenter);

    menuBar()->setCornerWidget(rightWidget, Qt::TopRightCorner);
}

void gui_t::update_status_dot() {
    QColor color = !is_connected_   ? colors::status_error.name()
                   : !is_receiving_ ? colors::status_warn.name()
                                    : colors::status_ok.name();
    status_dot_->setStyleSheet(
        QString("background-color: %1; border-radius: 10px;").arg(color.name().toStdString()));
}

} // namespace mbr
