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


#include <QWidget>
#include <QStackedWidget>
#include <QMenu>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "app/assets/images/app_icon.hpp"
#include "app/pages/page.hpp"
#include "app/context.hpp"
#include "app/pages/home.hpp"
#include "app/pages/plot.hpp"
#include "app/pages/serialmon.hpp"
#include "app/pages/analysis.hpp"
#include "app/pages/settings.hpp"
#include "core/time.hpp"
#include "esp32/backend.hpp"

using namespace std::chrono;

namespace mbr {

    gui_t::gui_t(const std::shared_ptr<app_context>& ctx) : bridge_(ctx->backend.get(), this), context_(ctx) {
        pages_ = new QStackedWidget(this);
        setCentralWidget(pages_);
        for (auto type : {page_type_t::HOME, page_type_t::PLOT, page_type_t::ANALYSIS, page_type_t::SERIAL}) {
            pages::page* p = create_page(type, context_, pages_);
            pages_->addWidget(p);
            page_lookup_[type] = p;
        }
        change_page(page_type_t::HOME);
        build_menu_bar();
    }


    [[nodiscard]] pages::page* gui_t::create_page(page_type_t type, const std::shared_ptr<app_context>& ctx, QWidget* parent) {
        switch (type) {
        case page_type_t::HOME:   return new pages::home_page(ctx, parent);
        case page_type_t::PLOT:    return new pages::plot_page(ctx, parent);
        case page_type_t::ANALYSIS:  return new pages::analysis_page(ctx, parent);
        case page_type_t::SERIAL: return new pages::serial_page(ctx, parent);
        case page_type_t::SETTINGS: return new pages::settings_page(ctx, parent);
        default:                  return nullptr;
        }
    }


    void gui_t::change_page(page_type_t type) {
        pages_->setCurrentWidget(page_lookup_[type]);
    }

    void gui_t::build_menu_bar() {
        menuBar()->setStyleSheet(R"(
            QMenuBar {
                background-color: #2b2b2b;
                color: white;
                font-size: 20px;
                spacing: 2px;
            }
            QMenuBar::item {
                background-color: transparent;
                padding: 4px 6px;
                border-radius: 4px;
            }
            QMenuBar::item:selected {
                background-color: #3C3F41;
            }
        )");
        auto* file = menuBar()->addMenu("File");
        file->addAction("Settings");
        file->addAction("Export Log");
        file->addAction("Exit", this, [] { QApplication::quit(); });
        auto* nav = menuBar()->addMenu("Navigation");
        nav->addAction("Home", this, [this] { change_page(page_type_t::HOME); });
        nav->addAction("Plot", this, [this] { change_page(page_type_t::PLOT); });
        nav->addAction("Serial", this, [this] { change_page(page_type_t::SERIAL); });
        nav->addAction("Analysis", this, [this] { change_page(page_type_t::ANALYSIS); });

        menuBar()->addAction("Sync Time", this, [this] {
            local_time lt;
            const auto sync_time = lt.micros_since_midnight();
            context_->backend->send_cmd(std::format("CMD SYNC {}", sync_time));
        });

        menuBar()->addAction("Restart Connection", this, [this] { context_->backend->get_try_connection() = true; });

        menuBar()->addAction("Clear Data", this, [this] { context_->backend->get_data().clear(); });

        auto* rightWidget = new QWidget(menuBar());
        auto* rightLayout = new QHBoxLayout(rightWidget);
        rightLayout->setContentsMargins(0, 4, 8, 4);
        rightLayout->setSpacing(8);


        connection_status_ = new QLabel("Disconnected", rightWidget);

        connect(&bridge_, &backend_bridge::connection_changed, this, [this](bool connected) {
            is_connected_ = connected;
            if (!connected) is_receiving_ = false;
            connection_status_->setText(connected ? "Connected" : "Disconnected");
            update_status_dot();
        });
        connection_status_->setStyleSheet("font-size: 20px;");

        status_dot_ = new QLabel(rightWidget);
        status_dot_->setFixedSize(20, 20);
        status_dot_->setStyleSheet("background-color: #F44336; border-radius: 10px;");
        rightLayout->addWidget(connection_status_, 0, Qt::AlignVCenter);
        rightLayout->addWidget(status_dot_, 0, Qt::AlignVCenter);

        menuBar()->setCornerWidget(rightWidget, Qt::TopRightCorner);
    }

    void gui_t::update_status_dot() {
        QString color = !is_connected_ ? "#F44336"
                       : !is_receiving_ ? "#FFC107"
                       : "#00E676";
        status_dot_->setStyleSheet(QString("background-color: %1; border-radius: 10px;").arg(color));
    }



} // namespace mbr
