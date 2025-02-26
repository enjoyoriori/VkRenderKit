#include "app.hpp"

int main() {
    // UTF-8出力のための設定
    std::ios_base::sync_with_stdio(false);
    std::locale::global(std::locale(".UTF-8"));
    
    // 日本語ロケールの設定
    setlocale(LC_ALL, "ja_JP.UTF-8");

    try {
        Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "エラーが発生しました: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}
