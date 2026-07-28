#include "app/pages/plot.hpp"

#include <qaction.h>
#include <qboxlayout.h>
#include <qplaintextedit.h>
#include <qtoolbutton.h>
#include <qtreewidget.h>
#include <qtreewidgetitemiterator.h>
#include <string>
#include <vector>
#include <cstddef>

#include <fmt/format.h>
#include <gsl/span>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include <QWidget>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QComboBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTimer>
#include <qcontainerfwd.h>

#include "core/log.hpp"
#include "esp32/backend.hpp"
#include "esp32/data.hpp"
#include "app/backend_bridge.hpp"

#include <qcustomplot.h>

namespace mbr::pages {

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
    QString button_style =
        "QToolButton {"
        "   background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #2d2d2d;"
        "   border-radius: 4px; padding: 4px 16px; font-size: 16px;"
        "}"
        "QToolButton:hover { background-color: #2a2a2a; }";
    auto* container = new QWidget(this);
    auto* v_layout = new QVBoxLayout(container);
    auto* h_layout = new QHBoxLayout();

    auto* log_button = new QToolButton();
    auto* log_action = new QAction(context_->backend->is_logging() ? "Stop Logging" : "Start Logging", this);
    connect(log_action, &QAction::triggered, this, [this, log_action] {
        context_->backend->set_logging(!context_->backend->is_logging());
        log_action->setText(context_->backend->is_logging() ? "Stop Logging" : "Start Logging");
    });
    log_button->setDefaultAction(log_action);
    log_button->setMinimumHeight(32);
    log_button->setStyleSheet(button_style);
    h_layout->addWidget(log_button);

    auto* download_name_input = new QLineEdit();
    download_name_input->setPlaceholderText("current_time.txt");
    download_name_input->setMaximumWidth(160);
    download_name_input->setMinimumHeight(32);
    h_layout->addWidget(download_name_input);

    auto* download_button = new QToolButton();
    auto* download_action = new QAction("Download Data", this);
    connect(download_action, &QAction::triggered, this, [this, download_name_input] {
        std::string name = download_name_input->text().toStdString();

        local_time lt;
        std::string filepath;
        if (!name.empty()) {
            filepath = fmt::format("{}_{}.txt", lt.to_string(false), name);
        } else {
            filepath = fmt::format("{}.txt", lt.to_string(false));
        }

        auto raw_lines = context_->backend->get_data().get_raw_lines();
        if (raw_lines.empty()) {
            log_warn(context_->log, "Cannot download data as the data buffer is empty!");
            download_name_input->clear();
            return;
        }

        std::ofstream out{filepath};
        if (!out.is_open()) {
            log_error(context_->log, fmt::format("Failed to open output file: {}", std::strerror(errno)));
            download_name_input->clear();
            return;
        }

        for (const auto& line : raw_lines) {
            out << line << "\n";
        }

        download_name_input->clear();
    });

    download_button->setMinimumHeight(32);
    download_button->setStyleSheet(button_style);
    download_button->setDefaultAction(download_action);
    h_layout->addWidget(download_button);

    text_log_ = new QPlainTextEdit(container);
    text_log_->setReadOnly(true);
    v_layout->addLayout(h_layout);
    v_layout->addWidget(text_log_);
    return container;
}

QWidget* plot_page::build_rhs() {

    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QToolButton* channel_selector = create_tree_dropdown(context_->backend->get_data().data_values, container);

    layout->addWidget(channel_selector);

    plot_ = new QCustomPlot(container);

    const QColor bg_color("#1e1e1e");
    const QColor axis_color("#a0a0a0");
    const QColor grid_color("#2d2d2d");
    const QColor line_color("#00adb5");

    plot_->setBackground(QBrush(bg_color));
    plot_->axisRect()->setBackground(QBrush(bg_color));

    auto configure_axis = [&](QCPAxis* axis, const QString& label) {
        axis->setLabel(label);
        axis->setBasePen(QPen(axis_color, 1.0));
        axis->setTickPen(QPen(axis_color, 1.0));
        axis->setSubTickPen(QPen(axis_color, 0.5));
        axis->setTickLabelColor(axis_color);
        axis->setLabelColor(axis_color);

        axis->grid()->setPen(QPen(grid_color, 1.0, Qt::SolidLine));
        axis->grid()->setSubGridPen(QPen(grid_color, 0.5, Qt::DotLine));
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
    plot_->legend->setBrush(QBrush(bg_color));
    plot_->legend->setTextColor(axis_color);

    layout->addWidget(plot_, 1);

    QTreeWidget* tree = channel_selector->findChild<QTreeWidget*>();

    if (tree) {
        connect(tree, &QTreeWidget::itemSelectionChanged, this, [this, tree, channel_selector]() {
            plot_->clearGraphs();
            plotted_counts_.clear();
            int count = tree->selectedItems().count();
            channel_selector->setText(count == 0 ? "Select Data" : QString("%1 selected").arg(count));
            active_graphs_.clear();
            for (QTreeWidgetItem* item : tree->selectedItems()) {
                QString key = item->data(0, Qt::UserRole).toString();
                if (!key.isEmpty()) {
                    plot_signal(key.toStdString());
                }

                if (item->childCount() > 0) {
                    for (int i = 0; i < item->childCount(); ++i) {
                        QTreeWidgetItem* child = item->child(i);
                        std::string child_key = child->data(0, Qt::UserRole).toString().toStdString();
                        if (!child_key.empty()) {
                            plot_signal(child_key);
                        }
                    }
                }
            }
        });
    }


    return container;
}

void plot_page::plot_timer() {

    connect(context_->bridge.get(), &backend_bridge::data_updated, this, [this](const auto&) { pending_update_ = true; });

    redraw_timer_ = new QTimer(this);
        connect(redraw_timer_, &QTimer::timeout, this, [this]() {
            if (pending_update_) {
                on_data_updated();
                pending_update_ = false;
            }
        });
        redraw_timer_->start(17);
}

QToolButton* plot_page::create_tree_dropdown(const std::vector<telemetry_data::data_info>& data, QWidget* parent) {
    auto* select = new QToolButton(parent);

    select->setText("Select Data");
    select->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    select->setPopupMode(QToolButton::InstantPopup);
    select->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    select->setStyleSheet(
        "QToolButton {"
        "   background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #2d2d2d;"
        "   border-radius: 4px; padding: 6px 12px; text-align: left;"
        "}"
        "QToolButton:hover { background-color: #2a2a2a; }"
    );

    auto* tree  = new QTreeWidget(select);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->setHeaderHidden(true);
    tree->setColumnCount(1);
    tree->setStyleSheet(
        "QTreeWidget { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #2d2d2d; }"
        "QTreeWidget::item:hover { background-color: #00adb5; color: white; }"
    );

    for (auto it : data) {
        bool found = false;
        QTreeWidgetItem* target_group = nullptr;
        for (int i = 0; i < tree->topLevelItemCount(); ++i) {
            if (QString::fromStdString(it.group) == tree->topLevelItem(i)->text(0)) {
                found = true;
                target_group = tree->topLevelItem(i);
                break;
            }
        }
        if (!found && it.plot) {
            target_group = new QTreeWidgetItem;
            QString group_name = QString::fromStdString(it.group);
            target_group->setText(0, group_name);
            tree->addTopLevelItem(target_group);
        }
        if (!target_group) {
            continue;
        }
        auto* child_item = new QTreeWidgetItem(target_group);
        child_item->setText(0, QString::fromStdString(it.name));
        child_item->setData(0, Qt::UserRole, QString::fromStdString(it.key));
    }
    tree->expandAll();

    auto* menu = new QMenu(select);
    auto* widget_action = new QWidgetAction(menu);

    tree->setMinimumSize(300, 0);
    widget_action->setDefaultWidget(tree);
    menu->addAction(widget_action);

    select->setMenu(menu);

    return select;
}



void plot_page::plot_signal(const std::string key) {
    if (active_graphs_.contains(key)) {
            return;
        }
    auto series = context_->backend->get_data().get_series(key);
    auto time   = context_->backend->get_data().get_time();

    QVector<double> x_times(time.begin(), time.end());
    QVector<double> y_values(series.begin(), series.end());

    QCPGraph* graph = plot_->addGraph();
    graph->setAdaptiveSampling(true);

    static const std::vector<QColor> palette = {
        QColor("#00adb5"), QColor("#ff5722"), QColor("#e91e63"),
        QColor("#9c27b0"), QColor("#4caf50"), QColor("#ffeb3b")
    };
    QColor color = palette[active_graphs_.size() % palette.size()];

    graph->setPen(QPen(color, 2.0));
    graph->setData(x_times, y_values);
    for (auto d : context_->backend->get_data().data_values) {
        if (d.key == key) {
            graph->setName(QString::fromStdString(d.name));
            break;
        }
    }
    graph->addToLegend();

    active_graphs_[key] = graph;
    plotted_counts_[key] = series.size();

    plot_->rescaleAxes();
    plot_->replot();
}

void plot_page::on_data_updated() {
    update_plot();
    update_stream();
}

void plot_page::update_plot() {

    if (active_graphs_.empty()) {
        return;
    }

    auto time = context_->backend->get_data().get_time();

    for (auto& [key, graph] : active_graphs_) {
        auto series = context_->backend->get_data().get_series(key);

        std::size_t already_plotted = plotted_counts_[key];
        std::size_t total = series.size();

        if (total < already_plotted) {
            graph->data()->clear();
            already_plotted = 0;
        }

        if (total <= already_plotted) {
            plotted_counts_[key] = total;
            continue;
        }

        QVector<double> new_x(time.begin() + already_plotted, time.end());
        QVector<double> new_y(series.begin() + already_plotted, series.end());
        graph->addData(new_x, new_y);

        plotted_counts_[key] = total;
    }

    plot_->xAxis->rescale();
    plot_->replot();
}

void plot_page::update_stream() {

    if (!context_->backend->is_logging()) {
        return;
    }

    auto series = context_->backend->get_data().get_raw_lines();

    std::size_t displayed = 0;
    if (text_log_->document()->isEmpty()) {
        displayed = 0;
    } else {
        displayed = static_cast<std::size_t>(text_log_->document()->blockCount());
    }

    std::size_t total = series.size();

    if (total < displayed) {
        text_log_->clear();
        displayed = 0;
    }

    if (total <= displayed) {
        return;
    }

    for (std::size_t i = displayed; i < total; ++i) {
        text_log_->appendPlainText(QString::fromStdString(series[i]));
    }
}

} // namespace mbr::pages
