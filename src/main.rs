use anyhow::Result;

mod app;
mod esp32;
mod util;

use crate::app::MBRApp;

fn main() -> Result<()> {
    MBRApp::launch()
}