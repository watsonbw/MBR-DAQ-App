#include "app/pages/plot.hpp"

#include <cstddef>
#include <qaction.h>
#include <qboxlayout.h>
#include <qplaintextedit.h>
#include <qtoolbutton.h>
#include <qtreewidget.h>
#include <qtreewidgetitemiterator.h>
#include <spdlog/common.h>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <gsl/span>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTimer>
#include <QTreeWidget>
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <qcontainerfwd.h>

#include <fmt/ostream.h>

#include "app/assets/style.hpp"
#include "app/backend_bridge.hpp"
#include "core/log.hpp"
#include "esp32/backend.hpp"
#include "esp32/data.hpp"

#include <qcustomplot.h>

namespace mbr::ui::pages {

namespace style  = ui::style;
namespace colors = ui::style::color;

void plot_page::on_enter() { log_info(context_->log, "Entered PlotPage"); }
void plot_page::on_exit() { log_info(context_->log, "Exited PlotPage"); }

void plot_page::build_page() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto* lhs = build_lhs();
    auto* rhs = build_rhs();

    plot_timer();

    layout->addWidget(lhs, 1);
    layout->addWidget(rhs, 3);
}

QWidget* plot_page::build_lhs() {
    style::button_style_options button_style;
    button_style.font_size = 10;

    auto* container = new QWidget(this);
    auto* v_layout  = new QVBoxLayout(container);
    auto* h_layout  = new QHBoxLayout();

    auto* log_button = new QToolButton();
    log_button->setText(context_->backend->is_logging() ? "Stop Logging" : "Start Logging");
    connect(log_button, &QToolButton::clicked, this, [this, log_button] {
        context_->backend->set_logging(!context_->backend->is_logging());
        log_button->setText(context_->backend->is_logging() ? "Stop Logging" : "Start Logging");
    });
    log_button->setMinimumHeight(32);
    log_button->setStyleSheet(style::make_button_style(button_style));
    h_layout->addWidget(log_button);

    auto* download_name_input = new QLineEdit();
    download_name_input->setPlaceholderText("current_time.txt");
    download_name_input->setMaximumWidth(160);
    download_name_input->setMinimumHeight(32);
    h_layout->addWidget(download_name_input);

    auto* download_button = new QToolButton();
    download_button->setText("Download Data");
    connect(download_button, &QToolButton::clicked, this, [this, download_name_input] {
        std::string name = download_name_input->text().toStdString();

        local_time  lt;
        std::string filepath;
        if (!name.empty()) {
            filepath = fmt::format("{}_{}.txt", lt.to_string(false), name);
        } else {
            filepath = fmt::format("{}.txt", lt.to_string(false));
        }

        auto raw_lines = context_->backend->get_data().get_raw_lines();
        if (raw_lines.empty()) {
            log_warn(context_->log, "Cannot download data as the data buffer is empty!");
            return;
        }

        std::ofstream out{filepath};
        if (!out.is_open()) {
            log_error(context_->log,
                      fmt::format("Failed to open output file: {}", std::strerror(errno)));
            return;
        }

        for (const auto& line : raw_lines) { fmt::println(out, "{}", line); }

        download_name_input->clear();
    });

    download_button->setMinimumHeight(32);
    download_button->setStyleSheet(style::make_button_style(button_style));
    h_layout->addWidget(download_button);

    text_log_ = new QPlainTextEdit(container);
    text_log_->setReadOnly(true);
    v_layout->addLayout(h_layout);
    v_layout->addWidget(text_log_);
    return container;
}

QWidget* plot_page::build_rhs() {

    auto* container = new QWidget(this);
    auto* layout    = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QToolButton* channel_selector =
        create_tree_dropdown(context_->backend->get_data().data_values, container);

    layout->addWidget(channel_selector);

    plot_ = new QCustomPlot(container);

    plot_->setBackground(QBrush(colors::bg_dark));
    plot_->axisRect()->setBackground(QBrush(colors::bg_dark));

    auto configure_axis = [&](QCPAxis* axis, const QString& label) {
        axis->setLabel(label);
        axis->setBasePen(QPen(colors::text_muted, 1.0));
        axis->setTickPen(QPen(colors::text_muted, 1.0));
        axis->setSubTickPen(QPen(colors::text_muted, 0.5));
        axis->setTickLabelColor(colors::text_muted);
        axis->setLabelColor(colors::text_muted);

        axis->grid()->setPen(QPen(colors::border, 1.0, Qt::SolidLine));
        axis->grid()->setSubGridPen(QPen(colors::border, 0.5, Qt::DotLine));
        axis->grid()->setSubGridVisible(true);
    };

    configure_axis(plot_->xAxis, "Time (s)");
    configure_axis(plot_->yAxis, "Value");

    plot_->axisRect()->setAutoMargins(QCP::msNone);
    plot_->axisRect()->setMargins(QMargins(50, 20, 20, 40));
    plot_->setNoAntialiasingOnDrag(true);
    plot_->rescaleAxes();
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot_->replot();
    plot_->legend->setVisible(true);
    plot_->legend->setBrush(QBrush(colors::bg_dark));
    plot_->legend->setTextColor(colors::text_muted);

    layout->addWidget(plot_, 1);

    QTreeWidget* tree = channel_selector->findChild<QTreeWidget*>();

    if (tree) {
        connect(tree, &QTreeWidget::itemSelectionChanged, this, [this, tree, channel_selector]() {
            plot_->clearGraphs();
            plotted_counts_.clear();
            usize count = tree->selectedItems().count();
            channel_selector->setText(count == 0 ? "Select Data"
                                                 : QString("%1 selected").arg(count));
            active_graphs_.clear();
            for (QTreeWidgetItem* item : tree->selectedItems()) {
                QString key = item->data(0, Qt::UserRole).toString();
                if (!key.isEmpty()) { plot_signal(key.toStdString()); }

                if (item->childCount() > 0) {
                    for (i32 i = 0; i < item->childCount(); ++i) {
                        QTreeWidgetItem* child = item->child(i);
                        std::string      child_key =
                            child->data(0, Qt::UserRole).toString().toStdString();
                        if (!child_key.empty()) { plot_signal(child_key); }
                    }
                }
            }
        });
    }

    return container;
}

void plot_page::plot_timer() {

    connect(context_->bridge.get(),
            &ui::bridge::backend_bridge::data_updated,
            this,
            [this](const auto&) { pending_update_ = true; });

    redraw_timer_ = new QTimer(this);
    connect(redraw_timer_, &QTimer::timeout, this, [this]() {
        if (pending_update_) {
            on_data_updated();
            pending_update_ = false;
        }
    });
    redraw_timer_->start(17);
}

QToolButton* plot_page::create_tree_dropdown(const std::vector<telemetry_data::data_info>& data,
                                             QWidget*                                      parent) {
    auto* select = new QToolButton(parent);

    select->setText("Select Data");
    select->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    select->setPopupMode(QToolButton::InstantPopup);
    select->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    select->setStyleSheet(style::make_button_style());

    auto* tree = new QTreeWidget(select);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->setHeaderHidden(true);
    tree->setColumnCount(1);
    tree->setStyleSheet(style::make_tree_style());

    QString current_group;
    for (auto& it : data) {
        bool             found        = false;
        QTreeWidgetItem* target_group = nullptr;
        current_group                 = QString::fromStdString(it.group);
        for (i32 i = 0; i < tree->topLevelItemCount(); ++i) {
            if (current_group == tree->topLevelItem(i)->text(0)) {
                found        = true;
                target_group = tree->topLevelItem(i);
                break;
            }
        }
        if (!found && it.plot) {
            target_group       = new QTreeWidgetItem;
            QString group_name = QString::fromStdString(it.group);
            target_group->setText(0, group_name);
            tree->addTopLevelItem(target_group);
        }
        if (!target_group) { continue; }
        auto* child_item = new QTreeWidgetItem(target_group);
        child_item->setText(0, QString::fromStdString(it.name));
        child_item->setData(0, Qt::UserRole, QString::fromStdString(it.key));
    }
    tree->expandAll();

    auto* menu          = new QMenu(select);
    auto* widget_action = new QWidgetAction(menu);

    tree->setMinimumSize(300, 0);
    widget_action->setDefaultWidget(tree);
    menu->addAction(widget_action);

    select->setMenu(menu);

    return select;
}

void plot_page::plot_signal(const std::string& key) {
    auto [it, inserted] = active_graphs_.try_emplace(key);
    if (!inserted) { return; }
    auto series = context_->backend->get_data().get_series(key);
    auto time   = context_->backend->get_data().get_time();

    if (!has_offset_ && !time.empty()) {
        time_offset_ = time.front();
        has_offset_  = true;
    }

    QVector<double> x_times;
    x_times.reserve(time.size());
    for (auto t : time) { x_times.append(t - time_offset_); }
    QVector<double> y_values(series.begin(), series.end());

    QCPGraph* graph = plot_->addGraph();
    graph->setAdaptiveSampling(true);

    static const std::array<QColor, 6> palette = colors::graph_palette;
    QColor                             color   = palette[active_graphs_.size() % palette.size()];

    graph->setPen(QPen(color, 2.0));
    graph->setData(x_times, y_values);
    for (auto& d : context_->backend->get_data().data_values) {
        if (d.key == key) {
            graph->setName(QString::fromStdString(d.name));
            break;
        }
    }
    graph->addToLegend();

    it->second           = graph;
    plotted_counts_[key] = series.size();

    plot_->rescaleAxes();
    plot_->replot();
}

void plot_page::on_data_updated() {
    update_plot();
    update_stream();
}

void plot_page::update_plot() {
    auto sync_opt = context_->backend->get_data().get_sync_lt();
    plot_->xAxis->setLabel(
        sync_opt
            ? QString("Data Synced at: %1").arg(QString::fromStdString(sync_opt->to_string(false)))
            : "No Time Synced");

    if (active_graphs_.empty()) { return; }

    auto time = context_->backend->get_data().get_time();

    for (auto& [key, graph] : active_graphs_) {
        auto series = context_->backend->get_data().get_series(key);

        usize& already_plotted = plotted_counts_[key];
        usize  total           = series.size();

        if (total < already_plotted) {
            graph->data()->clear();
            already_plotted = 0;
        }

        if (total == already_plotted) { continue; }

        QVector<double> new_x;
        new_x.reserve(total - already_plotted);
        for (auto it = time.begin() + already_plotted; it != time.end(); ++it) {
            new_x.append(*it - time_offset_);
        }
        QVector<double> new_y(series.begin() + already_plotted, series.end());
        graph->addData(new_x, new_y);

        already_plotted = total;
    }

    plot_->xAxis->rescale();
    plot_->replot();
}

void plot_page::update_stream() {

    if (!context_->backend->is_logging()) { return; }

    auto series = context_->backend->get_data().get_raw_lines();

    usize displayed = 0;
    if (text_log_->document()->isEmpty()) {
        displayed = 0;
    } else {
        displayed = static_cast<usize>(text_log_->document()->blockCount());
    }

    usize total = series.size();

    if (total < displayed) {
        text_log_->clear();
        displayed = 0;
    }

    if (total <= displayed) { return; }

    for (usize i = displayed; i < total; ++i) {
        text_log_->appendPlainText(QString::fromStdString(series[i]));
    }
}

} // namespace mbr::ui::pages
