#pragma once
#include "httplib.h"
#include "video_operations.hpp"

namespace ns_httpsever
{
    const std::string WWW_ROOT = "./www";
    const std::string VIDEO_ROOT = "/video/";
    const std::string IMAGE_ROOT = "/image/";
    // 因为httplib基于多线程，因此数据管理对象需要在多线程中访问，为了便于访问定义全局变量
    ns_operation::TableVideo *tb_video = NULL;
    // 这里为了更加功能模块划分清晰一些，不使用lamda表达式完成，否则所有的功能实现集中到一个函数中太过庞大
    class Server
    {
    private:
        int port;            // 服务器的 监听端口
        httplib::Server svr; // 用于搭建http服务器
    private:
        // 对应的业务处理接口
        static void Insert(const httplib::Request &req, httplib::Response &rsp)
        {
            if (!(req.has_file("name") && req.has_file("info") && req.has_file("video") && req.has_file("image")))
            {
                rsp.status = 400;
                rsp.body = R"({"result":false,"reason":"上传数据不正确"})"; // c++11原始字符串字面量,避免转义字符 “ \ ” 频繁出现
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            httplib::MultipartFormData name = req.get_file_value("name");
            httplib::MultipartFormData info = req.get_file_value("info");
            httplib::MultipartFormData video = req.get_file_value("video");
            httplib::MultipartFormData image = req.get_file_value("image");

            std::string video_name = name.content;
            std::string video_info = info.content;
            std::string video_path = WWW_ROOT + VIDEO_ROOT + video_name + video.filename;
            std::string image_path = WWW_ROOT + IMAGE_ROOT + video_name + image.filename;

            if (ns_util::FileUtil(video_path).SetContent(video.content) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"视频文件存储失败"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            if (ns_util::FileUtil(image_path).SetContent(image.content) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"图片文件存储失败"})";
                return;
            }
            ns_util::FileUtil(video_path).SetContent(video.content);
            ns_util::FileUtil(image_path).SetContent(image.content);

            Json::Value video_json;
            video_json["name"] = video_name;
            video_json["info"] = video_info;
            video_json["video"] = VIDEO_ROOT + video_name + video.filename;
            video_json["image"] = IMAGE_ROOT + video_name + image.filename;

            if (tb_video->Insert(video_json) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"数据库新增失败"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            // rsp.status = 200;
            // rsp.body = R"({"result":true})";
            // rsp.set_header("Content-Type", "application/json");

            //成功新增之后重定向回到主页（刷新）
            rsp.set_redirect("/index.html",303);
        }
        static void Update(const httplib::Request &req, httplib::Response &rsp)
        {
            int video_id = std::stoi(std::string (req.matches[1].first, req.matches[1].second));
            Json::Value video;
            if (ns_util::JsonUtil::UnSerialize(req.body, video) == false)
            {
                rsp.status = 400;
                rsp.body = R"({"result":false,"reason":"上传视频信息格式解析失败"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            if (tb_video->Update(video_id, video) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"数据库信息更新失败"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            rsp.status = 200;
            rsp.body = R"({"result":true})";
            rsp.set_header("Content-Type", "application/json");
        }
        static void Delete(const httplib::Request &req, httplib::Response &rsp)
        {
            int video_id = std::stoi(std::string (req.matches[1].first, req.matches[1].second));
            Json::Value video;
            if (tb_video->SelectOne(video_id, video) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"数据库中无相关信息"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            std::string video_path = WWW_ROOT + video["video"].asString();
            std::string image_path = WWW_ROOT + video["image"].asString();
            remove(video_path.c_str());
            remove(image_path.c_str());

            if (tb_video->Delete(video_id) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"数据库信息删除失败"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            rsp.status = 200;
            rsp.body = R"({"result":true})";
            rsp.set_header("Content-Type", "application/json");
        }
        static void SelectOne(const httplib::Request &req, httplib::Response &rsp)
        {
            int video_id = std::stoi(std::string (req.matches[1].first, req.matches[1].second));
            Json::Value video;
            if (tb_video->SelectOne(video_id, video) == false)
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"数据库中无相关信息"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            rsp.status = 200;
            ns_util::JsonUtil::Serialize(video, rsp.body);
            rsp.set_header("Content-Type", "application/json");
        }
        static void SelectAll(const httplib::Request &req, httplib::Response &rsp)
        {
            int select_flag = 0; // 0 全部查询; 1 模糊查询
            std::string search_key;
            if (req.has_param("search"))
            {
                select_flag = 1;
                search_key = req.get_param_value("search");
            }
            Json::Value videos;
            if (select_flag == 0)
            {
                if (tb_video->SelectAll(videos) == false)
                {
                    rsp.status = 500;
                    rsp.body = R"({"result":false,"reason":"查询失败 select_flag:0"})";
                    rsp.set_header("Content-Type", "application/json");
                    return;
                }
            }
            else if (select_flag == 1)
            {
                if (tb_video->SelectLike(search_key, videos) == false)
                {
                    rsp.status = 500;
                    rsp.body = R"({"result":false,"reason":"查询失败 select_flag:1"})";
                    rsp.set_header("Content-Type", "application/json");
                    return;
                }
            }
            else
            {
                rsp.status = 500;
                rsp.body = R"({"result":false,"reason":"查询失败 select_flag:-1"})";
                rsp.set_header("Content-Type", "application/json");
                return;
            }
            
            rsp.status = 200;
            ns_util::JsonUtil::Serialize(videos,rsp.body);
            rsp.set_header("Content-Type", "application/json");
        }

    public:
        Server(int port) : port(port){}
        // 建立请求与处理函数的映射关系，设置静态资源根目录，启动服务器
        bool RunModule()
        {
            // 初始化目录
            tb_video = new ns_operation::TableVideo();
            ns_util::FileUtil(WWW_ROOT).CreateDirectory();
            std::string video_path = WWW_ROOT + VIDEO_ROOT;
            ns_util::FileUtil(video_path).CreateDirectory();
            std::string image_path = WWW_ROOT + IMAGE_ROOT;
            ns_util::FileUtil(image_path).CreateDirectory();

            // 设置主页
            svr.set_mount_point("/", WWW_ROOT);
            // 设置回调函数
            svr.Post("/video", Insert);
            svr.Delete("/video/(\\d+)", Delete);
            svr.Put("/video/(\\d+)", Update);
            svr.Get("/video/(\\d+)", SelectOne);
            svr.Get("/video", SelectAll);
            // 启动服务器
            svr.listen("0.0.0.0", port);
            return true;
        }
    };
}