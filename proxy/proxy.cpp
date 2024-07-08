#include <iostream>
#include <boost/asio.hpp>
#include <boost/regex.hpp>

using boost::asio::ip::tcp;

class ProxySession : public std::enable_shared_from_this<ProxySession> {
public:
    ProxySession(tcp::socket client_socket) 
        : client_socket_(std::move(client_socket)), 
          server_socket_(client_socket_.get_executor()) {}

    void start() {
        std::cout << "Proxy session started.\n";
        read_from_client();
    }

private:
    void read_from_client() {
        auto self(shared_from_this());
        client_socket_.async_read_some(boost::asio::buffer(client_buffer_), 
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "read from client\n";
                    for (const auto& elem : client_buffer_) {
                        std::cout << elem << " ";
                    }
                    std::cout << "\n";
                    connect_to_server("www.baidu.com", "80");
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void connect_to_server(const std::string& host, const std::string& port) {
        auto self(shared_from_this());
        tcp::resolver resolver(client_socket_.get_executor());
        auto endpoints = resolver.resolve(host, port);
        boost::asio::async_connect(server_socket_, endpoints, 
            [host, this, self](boost::system::error_code ec, tcp::endpoint) {
                if (!ec) {
                    std::cout << "Host: " << host << "\n";
                    std::cout << "connect success\n";
                    write_to_server();
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void write_to_server() {
        auto self(shared_from_this());
        client_buffer_.fill('a');
        boost::asio::async_write(server_socket_, boost::asio::buffer(client_buffer_), 
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "start write\n";
                    read_from_server();
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void read_from_server() {
        auto self(shared_from_this());
        server_socket_.async_read_some(boost::asio::buffer(server_buffer_), 
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "server response\n";
                    for (const auto& elem : client_buffer_) {
                        std::cout << elem << " ";
                    }
                    std::cout << "\n";
                    write_to_client(length);
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void write_to_client(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(client_socket_, boost::asio::buffer(server_buffer_, length), 
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    read_from_server();
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    tcp::socket client_socket_;
    tcp::socket server_socket_;
    std::array<char, 8192> client_buffer_;
    std::array<char, 8192> server_buffer_;
};

class ProxyServer {
public:
    ProxyServer(boost::asio::io_context& io_context, short port) 
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<ProxySession>(std::move(socket))->start();
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
                accept();
            });
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: proxy <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        ProxyServer server(io_context, std::atoi(argv[1]));
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
