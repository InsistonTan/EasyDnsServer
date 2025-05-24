# EasyDnsServer
## 简介
本项目基于QT框架(QT6.x)开发, 实现一个windows的简易dns服务器


---

## 怎么编译本项目
### 依赖
- QT6.x [QT官网](https://www.qt.io/)
- 开发工具 QT Creator 或者其它
### 编译
- clone或下载本项目源码
- QT Creator打开本项目的 EasyDnsServer.pro文件
- 执行qmake
- 编译运行

---

## 已实现的功能
- windows 系统上实现 dns服务, 监听 UDP 53 端口数据, 根据配置的 dns 列表[ip 域名], 当有设备请求解析域名时, 先检查配置列表是否存在该域名的配置, 有则直接返回配置的 ip, 没有则请求系统dns服务获取ip
- ip监测功能, 当本机ip发生变化时, 自动重启dns服务
- 支持设置开机自启动

---

## License
This project is licensed under the [MIT License](LICENSE).
