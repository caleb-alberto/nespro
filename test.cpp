#include <iostream>
#include <string>
int main() {
    std::string bruh = "<html><body>"
                "<h1>400 Not Found</h1>"
                "<p>"
                "Invalid request"
                "</p>"
                "</body></html>";
    std::cout << bruh.size() << '\n';
}
