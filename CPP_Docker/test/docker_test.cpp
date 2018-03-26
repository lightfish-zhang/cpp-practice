#include "../src/docker.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "container start" << std::endl;
    docker::container_config config;

    // 配置容器
    config.root_dir = "~/tmp/docker-image";
    config.host_name = "test-image";

    docker::container container(config);// 根据 config 构造容器
    int status_code = container.start();                  // 启动容器
    std::cout << "container exit:" << status_code << std::endl;
    return 0;
}