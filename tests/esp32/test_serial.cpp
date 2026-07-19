#include <string_view>

#include <catch_amalgamated.hpp>

#include "esp32/serial.hpp"

namespace mbr::tests {

TEST_CASE("baud_rate helper functions") {
    CHECK(baud_rate_value(baud_rate_t::THREE) == 300);
    CHECK(baud_rate_value(baud_rate_t::NINETYSIX) == 9'600);
    CHECK(baud_rate_value(baud_rate_t::ONEONEFIFTYTWO) == 115'200);

    CHECK(std::string_view{baud_rate_string(baud_rate_t::THREE)} == "300");
    CHECK(std::string_view{baud_rate_string(baud_rate_t::NINETYSIX)} == "9600");
    CHECK(std::string_view{baud_rate_string(baud_rate_t::ONEONEFIFTYTWO)} == "115200");
}

TEST_CASE("serial_manager_t state and setup") {
    serial_manager_t manager{baud_rate_t::ONEONEFIFTYTWO, 500, nullptr};

    SECTION("Initial states are correct") {
        CHECK(manager.get_baud_rate() == baud_rate_t::ONEONEFIFTYTWO);
        CHECK_FALSE(manager.should_send_data());
        CHECK_FALSE(manager.is_serial_write());
        CHECK(manager.get_all_ports().empty());
        CHECK(manager.get_chosen_ports().empty());
        CHECK_FALSE(manager.is_running());
    }

    SECTION("Atomic state changes work") {
        manager.set_send_data(true);
        CHECK(manager.should_send_data());
        manager.set_send_data(false);
        CHECK_FALSE(manager.should_send_data());
        manager.set_serial_write(true);
        CHECK(manager.is_serial_write());
        manager.set_serial_write(false);
        CHECK_FALSE(manager.is_serial_write());
    }

    SECTION("Chosen port additions and removals") {
        CHECK_FALSE(manager.is_port_selected("COM3"));

        manager.add_port("COM3");
        CHECK(manager.is_port_selected("COM3"));
        CHECK(manager.get_chosen_ports().contains("COM3"));

        manager.add_port("COM4");
        CHECK(manager.is_port_selected("COM4"));
        CHECK(manager.get_chosen_ports().size() == 2);

        manager.remove_port("COM3");
        CHECK_FALSE(manager.is_port_selected("COM3"));
        CHECK(manager.is_port_selected("COM4"));
        CHECK(manager.get_chosen_ports().size() == 1);
    }

    SECTION("Baud rate modifications") {
        manager.change_baud_rate(baud_rate_t::NINETYSIX);
        CHECK(manager.get_baud_rate() == baud_rate_t::NINETYSIX);
    }
}

} // namespace mbr::tests
