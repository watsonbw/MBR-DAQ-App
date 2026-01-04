use crate::app::MBRApp;

mod app;
mod esp32;
mod util;

fn main() -> Result<(), eframe::Error> {
    MBRApp::launch()
}