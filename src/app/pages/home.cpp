#include "app/pages/home.hpp"

#include <qcolor.h>
#include <qwidget.h>
#include <string>
#include <utility>

#include <QFrame>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QTimer>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QToolButton>
#include <QScrollBar>
#include <qgridlayout.h>
#include <qlabel.h>

#include <fmt/format.h>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/assets/style.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

namespace mbr::ui::pages {

namespace style  = ui::style;
namespace colors = ui::style::color;

using namespace std::chrono_literals;

void home_page::on_enter() { log_info(context_->log, "Entered HomePage"); }
void home_page::on_exit() { log_info(context_->log, "Exited HomePage"); }

void home_page::build_page() {
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(1);

    auto* lhs = build_lhs();
    auto* rhs = build_rhs();

    layout->addWidget(lhs, 1);
    layout->addWidget(rhs, 1);

    connect_signals();
}

QWidget* home_page::build_rhs() {
    auto* container  = new QWidget;
    auto* v_layout   = new QVBoxLayout(container);
    auto* jason_grid = new QGridLayout();

    auto* title = new QLabel("Data Status");
    title->setStyleSheet(QString::fromStdString(
        fmt::format("color: {}; font: 12pt", colors::text_main.name().toStdString())));

    auto* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    usize row = 0;
    for (auto& d : context_->backend->get_data().data_values) {
        auto* label = new QLabel(QString::fromStdString(d.name));
        label->setStyleSheet(QString::fromStdString(
            fmt::format("color: {}; font: 12pt", colors::text_main.name().toStdString())));
        jason_grid->addWidget(label, row, 0);

        auto* status = new QLabel();
        status->setFixedSize(20, 20);
        status->setStyleSheet(
            QString::fromStdString(fmt::format("background-color: {}; border-radius: 10px;",
                                               colors::status_error.name().toStdString())));
        jason_grid->addWidget(status, row, 1);
        labels_.try_emplace(QString::fromStdString(d.key), status);
        row++;
    }
    jason_grid->setColumnStretch(2, 1);

    v_layout->addWidget(title);
    v_layout->addWidget(line);
    v_layout->addLayout(jason_grid);
    v_layout->addStretch();

    return container;
}

QWidget* home_page::build_lhs() {
    auto* container = new QWidget;
    auto* v_layout  = new QVBoxLayout(container);

    auto* sd_grid           = new QGridLayout();

    auto* sd_title = new QLabel("SD Card Control");
    sd_title->setStyleSheet(QString::fromStdString(
        fmt::format("color: {}; font: 12pt", colors::text_main.name().toStdString())));

    auto* sd_name_input = new QLineEdit();
    sd_name_input->setPlaceholderText("current_time.txt");
    sd_name_input->setMaximumWidth(160);
    sd_name_input->setMinimumHeight(32);

    auto* sd_name_button = new QToolButton();
    sd_name_button->setText("Create File");
    sd_name_button->setStyleSheet(style::make_button_style());


    auto* sd_open_button = new QToolButton();
    sd_open_button->setText("Open File");
    sd_open_button->setStyleSheet(style::make_button_style());

    sd_grid->addWidget(sd_name_input, 0, 0);
    sd_grid->addWidget(sd_name_button, 0, 1);
    sd_grid->addWidget(sd_open_button, 0, 2);

    sd_grid->setColumnStretch(0, 1);
    sd_grid->setColumnStretch(3, 2);

    auto* title = new QLabel("Log Output");
    title->setStyleSheet(QString::fromStdString(
        fmt::format("color: {}; font: 12pt", colors::text_main.name().toStdString())));

    auto* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    auto* line2 = new QFrame;
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);

    log_view_ = new QPlainTextEdit(this);
    log_view_->setReadOnly(true);
    log_view_->setMaximumBlockCount(2000);
    log_view_->setStyleSheet(QString::fromStdString(
        fmt::format("color: {}; font: 12pt", colors::text_main.name().toStdString())));


    v_layout->addWidget(sd_title);
    v_layout->addWidget(line);
    v_layout->addLayout(sd_grid);
    v_layout->addStretch();
    v_layout->addWidget(title);
    v_layout->addWidget(line2);
    v_layout->addWidget(log_view_);
    return container;
}

void home_page::connect_signals() {

    // rhs
    connect(context_->bridge.get(),
            &ui::bridge::backend_bridge::fields_received,
            this,
            [this](const std::vector<std::string>& keys) {
                const auto now = std::chrono::steady_clock::now();
                for (const auto& key : keys) {
                    auto qkey           = QString::fromStdString(key);
                    last_updated_[qkey] = now;
                    if (auto it = labels_.find(qkey); it != labels_.end()) {
                        it->second->setStyleSheet(QString::fromStdString(
                            fmt::format("background-color: {}; border-radius: 10px;",
                                        QColor(colors::status_ok).name().toStdString())));
                    }
                }
            });

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        const auto now = std::chrono::steady_clock::now();
        for (const auto& [key, time] : last_updated_) {
            if (now - time > 250ms) {
                if (auto it = labels_.find(key); it != labels_.end()) {
                    it->second->setStyleSheet(QString::fromStdString(
                        fmt::format("background-color: {}; border-radius: 10px;",
                                    QColor(colors::status_error).name().toStdString())));
                }
            }
        }
    });
    timer->start(100);

    // lhs
    auto* poll_timer = new QTimer(this);
    connect(poll_timer, &QTimer::timeout, this, [this]() {
        log_view_->setPlainText(QString::fromStdString(context_->logger.get_streamed_logs()));
    });
    poll_timer->start(250);
}


} // namespace mbr::ui::pages
