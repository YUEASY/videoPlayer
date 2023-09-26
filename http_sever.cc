#include "util.hpp"
#include "video_operations.hpp"
#include "http_sever.hpp"
void FileTest()
{
    ns_util::FileUtil("./www").CreateDirectory();
    ns_util::FileUtil("./www/index.html").SetContent("<html></html>");
    std::string body;
    ns_util::FileUtil("./www/index.html").GetContent(body);
    std::cout << body << std::endl;
    std::cout << ns_util::FileUtil("./www/index.html").FileSize() << std::endl;
}

void JsonTest()
{
    Json::Value root;
    root["id"] = 1;
    root["name"] = "小明";
    root["score"]["Chinese"] = 90;
    root["score"]["Math"].append(87.5);
    root["score"]["Math"].append(99);

    std::string body;
    ns_util::JsonUtil::Serialize(root, body);
    std::cout << body << std::endl;

    ns_util::JsonUtil::UnSerialize(body, root);
    std::cout << root.toStyledString() << std::endl;
}

void DateTest()
{
    ns_operation::TableVideo tb_video;
    Json::Value video;
    // video["name"] = "阿甘正传";
    // video["info"] = "美国经典电影";
    // video["video"] = "/video/forrest_gump.mp4";
    // video["image"] = "/img/forrest_gump.jpg";

    // tb_video.Insert(video);
    tb_video.SelectAll(video);
    std::string str;
    ns_util::JsonUtil::Serialize(video, str);
    std::cout << str << std::endl;
}

void SeverTest()
{
    ns_httpsever::Server svr(8080);
    svr.RunModule();
}

int main()
{
    // FileTest();
    // JsonTest();
    // DateTest();
    SeverTest();
    return 0;
}