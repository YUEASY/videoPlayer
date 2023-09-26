#pragma once

#include "util.hpp"
#include <mutex>
#include <cstdlib>
#include <mysql/mysql.h>

namespace ns_operation
{
    class TableVideo
    {
    private:
        MYSQL *mysql;   // 一个对象就是一个客户端，管理一张表
        std::mutex mtx; // 防备操作对象在多线程中使用存在的 线程安全问题

        const char *HOST = "127.0.0.1";
        const char *USER = "root";
        const char *PASSWD = "";
        const char *DB_NAME = "db_video";

        // db_video: tb_video

        // tb_video
        // +-------+--------------+------+-----+---------+----------------+
        // | Field | Type         | Null | Key | Default | Extra          |
        // +-------+--------------+------+-----+---------+----------------+
        // | id    | int(11)      | NO   | PRI | NULL    | auto_increment |
        // | name  | varchar(32)  | YES  |     | NULL    |                |
        // | info  | text         | YES  |     | NULL    |                |
        // | video | varchar(256) | YES  |     | NULL    |                |
        // | image | varchar(256) | YES  |     | NULL    |                |
        // +-------+--------------+------+-----+---------+----------------+

    public:
        // 完成mysql句柄初始化
        TableVideo()
        {
            mysql = ns_util::SQLUtil::MysqlInit(DB_NAME, HOST, USER, PASSWD);
            if (mysql == nullptr)
            {
                exit(-1);
            }
        }
        // 释放msyql操作句柄
        ~TableVideo()
        {
            ns_util::SQLUtil::MysqlDestroy(mysql);
        }
        // 新增-传入视频信息
        bool Insert(const Json::Value &video)
        {
            std::string sql;
            sql.append("insert into tb_video(name, info, video, image) values(");
            sql.append("\'" + video["name"].asString() + "\', ");
            sql.append("\'" + video["info"].asString() + "\', ");
            sql.append("\'" + video["video"].asString() + "\', ");
            sql.append("\'" + video["image"].asString() + "\');");
            return ns_util::SQLUtil::MysqlQuery(mysql, sql);
        }
        // 修改-传入视频id和信息
        bool Update(int video_id, const Json::Value &video)
        {
            std::string sql;
            sql.append("update tb_video set ");

            // 拼接需要更新的字段
            if (!video["name"].isNull())
            {
                sql.append("name=\'");
                sql.append(video["name"].asString());
                sql.append("\', ");
            }
            if (!video["info"].isNull())
            {
                sql.append("info=\'");
                sql.append(video["info"].asString());
                sql.append("\', ");
            }
            if (!video["video"].isNull())
            {
                sql.append("video=\'");
                sql.append(video["video"].asString());
                sql.append("\', ");
            }
            if (!video["image"].isNull())
            {
                sql.append("image=\'");
                sql.append(video["image"].asString());
                sql.append("\', ");
            }

            // 去掉 SQL 语句末尾的逗号和空格
            sql.erase(sql.end() - 2, sql.end());

            // 拼接 WHERE 子句
            sql.append(" where id=");
            sql.append(std::to_string(video_id));
            sql.append(";");

            return ns_util::SQLUtil::MysqlQuery(mysql, sql);
        }
        // 删除-传入视频id
        bool Delete(const int video_id)
        {
            std::string sql;
            sql.append("delete from tb_video where id=");
            sql.append(std::to_string(video_id));
            sql.append(";");

            return ns_util::SQLUtil::MysqlQuery(mysql, sql);
        }
        // 查询所有--输出所有视频信息
        bool SelectAll(Json::Value &videos)
        {
            std::string sql = "select * from tb_video;";
            mtx.lock();
            if (!ns_util::SQLUtil::MysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }
            // 保存结果集
            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                std::cerr << "mysql store result error:" << std::endl;
                std::cerr << mysql_error(mysql) << std::endl;
                mtx.unlock();
                return false;
            }
            mtx.unlock();

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Json::Value video;
                video["id"] = std::stoi(row[0]);
                video["name"] = row[1];
                video["info"] = row[2];
                video["video"] = row[3];
                video["image"] = row[4];
                videos.append(video);
            }
            mysql_free_result(res);
            return true;
        }
        // 查询单个-输入视频id,输出信息
        bool SelectOne(int video_id, Json::Value &video)
        {
            std::string sql;
            sql.append("select * from tb_video where id=");
            sql.append(std::to_string(video_id));
            sql.append(";");
            mtx.lock();
            if (!ns_util::SQLUtil::MysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }
            // 保存结果集
            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                std::cerr << "mysql store result error:" << std::endl;
                std::cerr << mysql_error(mysql) << std::endl;
                mtx.unlock();
                return false;
            }
            mtx.unlock();
            if (mysql_num_rows(res) != 1)
            {
                std::cout << "video_id " << video_id << " does not exist!" << std::endl;
                mysql_free_result(res);
                return false;
            }

            MYSQL_ROW row = mysql_fetch_row(res);
            video["id"] = std::stoi(row[0]);
            video["name"] = row[1];
            video["info"] = row[2];
            video["video"] = row[3];
            video["image"] = row[4];

            mysql_free_result(res);
            return true;
        }
        // 模糊匹配-输入名称关键字，输出视频信息
        bool SelectLike(const std::string &key, Json::Value &videos)
        {
            std::string sql;
            sql.append("select * from tb_video where name like '%");
            sql.append(key);
            sql.append("%';");

            mtx.lock();
            if (!ns_util::SQLUtil::MysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }
            // 保存结果集
            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                std::cerr << "mysql store result error:" << std::endl;
                std::cerr << mysql_error(mysql) << std::endl;
                mtx.unlock();
                return false;
            }
            mtx.unlock();

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Json::Value video;
                video["id"] = std::stoi(row[0]);
                video["name"] = row[1];
                video["info"] = row[2];
                video["video"] = row[3];
                video["image"] = row[4];
                videos.append(video);
            }
            mysql_free_result(res);
            if(videos.empty())
            {
                std::cerr <<"file about "<< key <<" not found!" << std::endl;
                return false;
            }
            return true;
        }
    };
}