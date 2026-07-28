#include "app/pages/plot.hpp"

#include <qtoolbutton.h>
#include <qtreewidget.h>
#include <qtreewidgetitemiterator.h>
#include <string>
#include <vector>

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

#include "core/log.hpp"
#include "esp32/backend.hpp"
#include "esp32/data.hpp"
#include "qcustomplot.h"

namespace mbr::pages {

void plot_page::on_enter() { log_info(context_->log, "Entered PlotPage"); }
void plot_page::on_exit() { log_info(context_->log, "Exited PlotPage"); }

void plot_page::build_page() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto* lhs = build_lhs();
    auto* rhs = build_rhs();

    layout->addWidget(lhs, 1);
    layout->addWidget(rhs, 3);
}

QWidget* plot_page::build_lhs() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);

    text_log_ = new QPlainTextEdit(container);
    text_log_->setReadOnly(true);
    layout->addWidget(text_log_);
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

    // Graph setup & trace styling
    QCPGraph* graph = plot_->addGraph();


    QPen pen(line_color);
    pen.setWidthF(2.0);
    graph->setPen(pen);

    // Axes & gridline styling
    auto configure_axis = [&](QCPAxis* axis, const QString& label) {
        axis->setLabel(label);
        axis->setBasePen(QPen(axis_color, 1.0));
        axis->setTickPen(QPen(axis_color, 1.0));
        axis->setSubTickPen(QPen(axis_color, 0.5));
        axis->setTickLabelColor(axis_color);
        axis->setLabelColor(axis_color);

        // Subtle dark gridlines
        axis->grid()->setPen(QPen(grid_color, 1.0, Qt::SolidLine));
        axis->grid()->setSubGridPen(QPen(grid_color, 0.5, Qt::DotLine));
        axis->grid()->setSubGridVisible(true);
    };

    configure_axis(plot_->xAxis, "Time (s)");
    configure_axis(plot_->yAxis, "Value");

    // Padding, scaling & interactions
    plot_->axisRect()->setAutoMargins(QCP::msNone);
    plot_->axisRect()->setMargins(QMargins(50, 20, 20, 40));
    plot_->rescaleAxes();
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot_->replot();

    layout->addWidget(plot_, 1);

    QTreeWidget* tree = channel_selector->findChild<QTreeWidget*>();

    if (tree) {
        connect(tree, &QTreeWidget::itemSelectionChanged, this, [this, tree]() {
            plot_->clearGraphs();
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

    select->setPopupMode(QToolButton::InstantPopup);
    select->setStyleSheet(
            "QPushButton {"
            "   background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #2d2d2d;"
            "   border-radius: 4px; padding: 6px 12px; text-align: left;"
            "}"
            "QPushButton::menu-indicator { image: none; }"
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
        auto* child_item = new QTreeWidgetItem(target_group);
        child_item->setText(0, QString::fromStdString(it.name));
        child_item->setData(0, Qt::UserRole, QString::fromStdString(it.key));
    }
    tree->expandAll();

    auto* menu = new QMenu(select);
    auto* widget_action = new QWidgetAction(menu);

    tree->setMinimumSize(300, 200);
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

    static const std::vector<QColor> palette = {
        QColor("#00adb5"), QColor("#ff5722"), QColor("#e91e63"),
        QColor("#9c27b0"), QColor("#4caf50"), QColor("#ffeb3b")
    };
    QColor color = palette[active_graphs_.size() % palette.size()];

    graph->setPen(QPen(color, 2.0));
    graph->setData(x_times, y_values);
    graph->setName(QString::fromStdString(key));

    active_graphs_[key] = graph;

    plot_->rescaleAxes();
    plot_->replot();
}

} // namespace mbr::pages
