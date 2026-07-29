#pragma once
#include <QObject>
#include <esp32/backend.hpp>
#include <esp32/data.hpp>
#include <qtmetamacros.h>

namespace mbr {

class backend_bridge : public QObject {
    Q_OBJECT
  public:
    explicit backend_bridge(telemetry_backend* backend, QObject* parent = nullptr)
        : QObject(parent) {
        backend->set_on_data(
            [this](const ankerl::unordered_dense::map<std::string, std::vector<double>>& d) {
                emit data_updated(d);
            });
        backend->set_on_connection_changed([this](bool c) { emit connection_changed(c); });
        backend->set_on_receiving_changed([this](bool r) { emit receiving_changed(r); });
        backend->set_on_fields_received(
            [this](const std::vector<std::string>& keys) { emit fields_received(keys); });
    }
  signals:
    void data_updated(const ankerl::unordered_dense::map<std::string, std::vector<double>>&);
    void connection_changed(bool connected);
    void receiving_changed(bool receiving);
    void fields_received(const std::vector<std::string>& keys);
};

} // namespace mbr
