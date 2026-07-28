#pragma once

#include <memory>
#include <qplaintextedit.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qcustomplot.h>
#include <string>
#include <vector>

#include <gsl/span>
#include <QWidget>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "app/pages/page.hpp"
#include "esp32/data.hpp"
#include "esp32/data.hpp"

namespace mbr::pages {

class plot_page : public page {
  public:
    explicit plot_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr) :  page{ctx, parent} {
        build_page();
    }
    ~plot_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void build_page() override;

  private:
    QWidget* build_rhs();
    QWidget* build_lhs();
    QToolButton* create_tree_dropdown(const std::vector<telemetry_data::data_info>& data, QWidget* parent);
    void plot_signal(const std::string key);
    QPlainTextEdit* text_log_;
    QCustomPlot* plot_;
    ankerl::unordered_dense::map<std::string, QCPGraph*> active_graphs_;
    std::string         download_fd_text_;
};

} // namespace mbr::pages
