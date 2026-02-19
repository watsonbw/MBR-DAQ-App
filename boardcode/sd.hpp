
class SDCard {
    public:
        explicit SDCard();
        void OpenSD();
        void WriteSD(const char* message);
        void CloseSD();

    private:
        bool isOpen = 0;
};