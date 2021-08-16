#include <iostream>
#include <string>

#include "crow_all.h"

void send_content(crow::response &res, std::string content, std::string content_type)
{
    res.set_header("Content-Type", content_type);
    res.write(content);
    res.end();
}

void send_file(crow::response &res, std::string filename, std::string content_type)
{
    std::ifstream in("../public/" + filename, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res.set_header("Content-Type", content_type);
        res.write(contents.str());
    } 
    else 
    {
        res.code = 404;
        res.write("Not Found");
    }
    res.end();
}

void send_html(crow::response &res, std::string filename)
{
    send_file(res, filename + ".html", "text/html");
}

void send_image(crow::response &res, std::string filename)
{
    send_file(res, "images/" + filename, "image/jpeg");
}

void send_script(crow::response &res, std::string filename)
{
    send_file(res, "scripts/" + filename, "text/javascript");
}

void send_style(crow::response &res, std::string filename)
{
    send_file(res, "styles/" + filename, "text/css");
}

std::string get_view(const std::string &filename, crow::mustache::context &x)
{
    return crow::mustache::load("../public/templates/" + filename + ".html").render(x);
}
