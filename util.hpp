#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <jsoncpp/json/json.h>
// #include <mysql-cppconn/mysqlx/xdevapi.h>
#include <mysql/mysql.h>

namespace ns_util
{

    class FileUtil
    {
    private:
        std::string file_path; // 文件路径名称

    public:
        FileUtil(const std::string &path) : file_path(path) {}
        // 判断文件是否存在
        bool Exists()
        {
            // access()的F_OK用于检测文件是否存在，存在则返回0
            int ret = access(file_path.c_str(), F_OK);
            if (ret != 0)
            {
                std::cerr << "file is not exists: " << file_path << std::endl;
                return false;
            }
            return true;
        }
        // 获取文件大小
        size_t FileSize()
        {
            if (Exists() == false)
            {
                return 0;
            }
            struct stat st;
            // stat()填充文件属性st，成功则返回0
            if (stat(file_path.c_str(), &st) != 0)
            {
                std::cerr << "get file stat failed: " << file_path << std::endl;
                return 0;
            }
            return st.st_size;
        }
        // 获取文件数据，填充至content
        bool GetContent(std::string &content)
        {
            std::ifstream ifs;
            ifs.open(file_path, std::ios::binary);
            if (!ifs.is_open())
            {
                std::cerr << "open file failed: " << file_path << std::endl;
                return false;
            }
            // content.resize(FileSize());
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            content = buffer.str();

            if (!buffer.good())
            {
                std::cerr << "read file content failed: " << file_path << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }
        // 向文件写入数据
        bool SetContent(const std::string &content)
        {
            std::ofstream ofs;
            ofs.open(file_path, std::ios::binary);
            if (!ofs.is_open())
            {
                std::cerr << "open file failed: " << file_path << std::endl;
                return false;
            }
            ofs << content;
            if (!ofs.good())
            {
                std::cerr << "write file content failed: " << file_path << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }
        // 创建目录
        bool CreateDirectory()
        {
            if (Exists())
                return true;
            if (mkdir(file_path.c_str(), 0777) == -1)
            {
                std::cerr << "mkdir failed: " << file_path << std::endl;
            }
            return true;
        }
    };

    class JsonUtil
    {
    public:
        static bool Serialize(const Json::Value &root, std::string &str)
        {
            Json::StreamWriterBuilder swb;
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());

            std::stringstream ss;
            if (sw->write(root, &ss) != 0)
            {
                std::cerr << "Serialize failed! " << std::endl;
                return false;
            }
            str = ss.str();
            return true;
        }

        static bool UnSerialize(const std::string &str, Json::Value &root)
        {
            Json::CharReaderBuilder crb;
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());

            std::string errs;
            if (cr->parse(str.c_str(), str.c_str() + str.size(), &root, &errs) == false)
            {
                std::cerr << "UnSerialize failed! " << std::endl;
                return false;
            }
            return true;
        }
    };

    class SQLUtil
    {
    public:
        static MYSQL *MysqlInit(const char *db, const char *host = "127.0.0.1", const char *user = "root", const char *passwd = "", unsigned int port = 0, const char *unix_socket = nullptr, unsigned long client_flag = 0)
        {
            MYSQL *mysql = mysql_init(NULL);
            if (mysql == nullptr)
            {
                std::cerr << "init mysql instance failed!" << std::endl;
                return nullptr;
            }
            if (mysql_real_connect(mysql, host, user, passwd, db, port, unix_socket, client_flag) == nullptr)
            {
                std::cerr << "connect mysql sever failed!" << std::endl;
                mysql_close(mysql);
                return nullptr;
            }
            mysql_set_character_set(mysql, "utf8");
            return mysql;
        }

        static void MysqlDestroy(MYSQL *mysql)
        {
            if (mysql != nullptr)
            {
                mysql_close(mysql);
            }
        }

        static bool MysqlQuery(MYSQL *mysql, const std::string &sql)
        {
            if(mysql_query(mysql,sql.c_str()) != 0)
            {
                std::cerr << "mysql query error:" << std::endl;
                std::cerr << sql << std::endl;
                std::cerr << mysql_error(mysql) << std::endl;
                return false;
            }
            return true;
        }
    };
}
