use eframe::egui;
use anyhow::{anyhow, Result};

mod assets;
mod context;
mod gui;
mod pages;

#[derive(Default)]
pub struct MBRApp {}

impl MBRApp {
    fn new(_cc: &eframe::CreationContext<'_>) -> Self {
        Self::default()
    }

    pub fn launch() -> Result<()> {
        let native_options = eframe::NativeOptions::default();
        eframe::run_native(
            "Michigan Baja Racing - Data Suite",
            native_options,
            Box::new(|cc| Ok(Box::new(Self::new(cc)))),
        ).map_err(|e| anyhow!("{:?}", e))
    }
}

impl eframe::App for MBRApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Hello, World!");
        });
    }
}
