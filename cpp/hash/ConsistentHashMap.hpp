#include <map>
#include <string>
#include <list>
#include <functional> 
#include <algorithm>
#include "MurmurHash.h"


#ifndef __CONSISTENT_HASH_H__
#define __CONSISTENT_HASH_H__

//虚拟节点 
struct VNode {
	VNode(const std::string& n, std::size_t id) : name(n), vnode_id(id){}
	VNode(const char* n, std::size_t id) : name(n), vnode_id(id){}

	std::string to_str() const
	{
		std::string temp = name;
		temp += "%*-:";
		temp += std::to_string(vnode_id);
		return temp;
	}
	std::string name;
	std::size_t vnode_id;
};

//hash算法
struct MurmurHash {
	uint32_t hash(const std::string& key) {
		result_type ret = 0;
		MurmurHash2(key.c_str(), key.size(), &ret);
		return ret;
	}

	uint32_t operator()(const VNode& node) {
		return hash(node.to_str());
	}
	typedef uint32_t result_type;
};


template <typename T,
	typename Hash,
	typename Alloc = std::allocator<std::pair<const typename Hash::result_type, T > > >
class ConsistentHashMap
{
public:

	typedef typename Hash::result_type size_type;
	typedef std::map<size_type, T, std::less<size_type>, Alloc> map_type;
	typedef typename map_type::value_type value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef typename map_type::iterator iterator;
	typedef typename map_type::reverse_iterator reverse_iterator;
	typedef Alloc allocator_type;

public:

	ConsistentHashMap() :vnodecount_(100){}
	~ConsistentHashMap() {}

	//插入节点， 
	//node：节点IP
	void insert(const std::string& node)
	{
		VNode vnode(node, 0);
		for (std::size_t i = 0; i<vnodecount_; ++i)
		{
			vnode.vnode_id = i;
			insert(vnode);
		}
	}

	//根据id返回， host
	iterator find(const std::string& key) 
	{
		return find(hasher_.hash(key));
	}

	void set_vnode_count(std::size_t c){ vnodecount_ = c; }
	
private:
	std::size_t size() const {
		return nodes_.size();
	}

	bool empty() const {
		return nodes_.empty();
	}

	std::pair<iterator, bool> insert(T& node) {
		size_type hash = hasher_(node);
		return nodes_.insert(value_type(hash, node));
	}


	void erase(iterator it) {
		nodes_.erase(it);
	}

	std::size_t erase(const T& node) {
		size_type hash = hasher_(node);
		return nodes_.erase(hash);
	}

	iterator find(size_type hash) {
		if (nodes_.empty()) {
			return nodes_.end();
		}

		iterator it = nodes_.lower_bound(hash);

		if (it == nodes_.end()) {
			it = nodes_.begin();
		}

		return it;
	}
	iterator begin() { return nodes_.begin(); }
	iterator end() { return nodes_.end(); }
	reverse_iterator rbegin() { return nodes_.rbegin(); }
	reverse_iterator rend() { return nodes_.rend(); }

private:

	Hash hasher_;
	map_type nodes_;
	std::size_t vnodecount_;
};

typedef ConsistentHashMap<VNode, MurmurHash> consistent_hash_t;
#endif