#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/filesystem.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

#define CROW_MAIN
#include "helpers.h"


int main(int argc, char* argv[])
{
    crow::SimpleApp app;
    crow::mustache::set_base(".");

    mongocxx::instance inst{};
    char* mongo_uri = getenv("MONGO");
    std::string mongo = static_cast<std::string>(mongo_uri != NULL? std::string(mongo_uri): "mongodb://127.0.0.1:27017");
    std::string mongo_conn = std::string(mongo);
    mongocxx::client conn{mongocxx::uri{mongo_conn}};
    auto collection = conn["simple"]["contacts"];

    CROW_ROUTE(app, "/simple")
    ([](){
        return "<div><h1>Hello!<h1></div>";
    });

    CROW_ROUTE(app, "/styles/<string>")
    ([](const crow::request &req, crow::response &res, std::string filename){
        send_style(res, filename);
    });

    CROW_ROUTE(app, "/scripts/<string>")
    ([](const crow::request &req, crow::response &res, std::string filename){
        send_script(res, filename);
    });

    CROW_ROUTE(app, "/images/<string>")
    ([](const crow::request &req, crow::response &res, std::string filename){
        send_image(res, filename);
    });

    CROW_ROUTE(app, "/contacts-raw")
    ([&collection](const crow::request &req, crow::response &res){
        mongocxx::options::find opts;
        opts.skip(9);
        opts.limit(10);
        auto docs = collection.find({}, opts);
        std::ostringstream os;
        os << "[";
        for (auto &&doc : docs)
        {
            os << bsoncxx::to_json(doc) << ",";
        }
        
        std::string result = os.str();
        result[result.size() - 1] = ']';
        send_content(res, result, "application/json");
    });

    CROW_ROUTE(app, "/contact/<string>")
    ([&collection](std::string email) {
        auto doc = collection.find_one(make_document(kvp("email", email)));
        return crow::response(bsoncxx::to_json(doc.value().view()));
    });

    CROW_ROUTE(app, "/contacts")
    ([&collection]() {
        mongocxx::options::find opts;
        opts.skip(9);
        opts.limit(10);
        auto docs = collection.find({}, opts);
        crow::json::wvalue dto;
        std::vector<crow::json::rvalue> contacts;
        contacts.reserve(10);

        for (auto &&doc : docs)
        {
            contacts.push_back(crow::json::load(bsoncxx::to_json(doc)));
        }
        dto["contacts"] = contacts;
        return get_view("contacts", dto);
    });

    CROW_ROUTE(app, "/rest_test").methods(
        crow::HTTPMethod::Post,
        crow::HTTPMethod::Get,
        crow::HTTPMethod::Put
    )
    ([](const crow::request &req, crow::response &res) {
        std::string method = crow::method_name(req.method);
        res.set_header("Content-Type", "text/plain");
        res.write(method + " rest_test");
        res.end();
    });

    CROW_ROUTE(app, "/")
    ([](const crow::request &req, crow::response &res){
        send_html(res, "index");
    });

    CROW_ROUTE(app, "/about")
    ([](const crow::request &req, crow::response &res){
        send_html(res, "pages/about");
    });

    char* port = getenv("PORT");
    uint16_t iPort = static_cast<uint16_t>(port != NULL? std::stoi(port): 8000);
    std::cout << "PORT = " << iPort << "\n";
    app.port(iPort).multithreaded().run();
}