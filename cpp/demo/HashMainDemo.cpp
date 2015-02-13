// hash.cpp : 定义控制台应用程序的入口点。
//

#include <stdint.h>
#include <iostream>
#include <string>

#include "ConsistentHashMap.hpp"
#include "MurmurHash.h"

//所有主机的列表 
const char* nodes[] = {
	"192.68.1.100",
	"192.168.1.101",
	"192.268.1.102",
	"192.368.1.103",
	"192.468.1.104",
	"192.468.1.105",
	"192.468.1.106"
};

int main(int argc, char const *argv[])
{
	consistent_hash_t consistent_hash_;

	//指定虚拟节点数量 80
	consistent_hash_.set_vnode_count(80);

	for (std::size_t i = 0; i < 7; ++i) {
		consistent_hash_.insert(nodes[i]);
	}

	std::string key("hello");

	for (size_t i = 0; i < 50; i++)
	{
		std::string temp(key);
		temp += std::to_string(i);
		auto it = consistent_hash_.find(temp);
		std::cout << "node:" << it->second.name << "  vnode:" << it->second.vnode_id << "  hash:" << it->first << std::endl;
	}

	return 0;
}
