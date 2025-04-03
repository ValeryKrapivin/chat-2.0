#include <iostream>
#include <boost/asio.hpp>
#include <sqlite3.h>

using boost::asio::ip::tcp;

class ChatServer {
public:
    ChatServer(short port) : acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }

    void run() {
        io_service_.run();
    }

private:
    void start_accept() {
        tcp::socket socket(io_service_);
        acceptor_.async_accept(socket,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket))->start();
                }
                start_accept();
            });
    }

    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    // Обработка запроса клиента
                    // Например, регистрация пользователя или получение сообщений
                    do_write(length);
                }
            });
    }

    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {});
    }

    tcp::socket socket_;
    char data_[1024];
};

int main() {
    try {
        ChatServer server(12345);
        server.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
