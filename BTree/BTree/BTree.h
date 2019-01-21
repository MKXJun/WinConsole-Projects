#ifndef _BTREE_H_
#define _BTREE_H_
#include <utility>
#include <crtdbg.h>
#include <sstream>
#include <string>
#include <vector>


// 键值对
template <class KeyType, class ValueType>
struct KeyValuePair
{
	const KeyType key;
	ValueType value;

	KeyValuePair(const KeyType& key,const ValueType& value)
		: key(key), value(value)
	{
	}

	KeyValuePair()
		: key(), value()
	{
	}
};


// <KeyType关键字类型, ValueType值类型, order阶数>
// 以键值对形式存储的B树，阶数必须不小于3
// 每个非根节点能容纳的键值对数范围为[(order + 1) / 2 - 1, order - 1] 
template <class KeyType, class ValueType, std::size_t order = 3> 
class BTree
{
public:
	typedef KeyValuePair<KeyType, ValueType> KeyValuePair;

private:
	struct BTreeNode
	{
		std::size_t numOfKeys;			// 节点关键字数目
		KeyValuePair* pairs[order];		// 键值对指针数组
		BTreeNode* childs[order + 1];	// 子树指针数组，预留一个多余空位方便分裂
		BTreeNode* parent;				// 父节点

		BTreeNode(const KeyValuePair& pair, BTreeNode* parent = nullptr)
			: numOfKeys(1), pairs{}, childs{}, parent(parent)
		{
			pairs[0] = new KeyValuePair(pair);
		}

		BTreeNode(BTreeNode* parent = nullptr)
			: numOfKeys(0), pairs{}, childs{}, parent(parent)
		{
		}

		~BTreeNode()
		{
			for (std::size_t i = 0; i < numOfKeys; ++i)
				delete pairs[i];
		}
	};


public:
	BTree()
		: root(), count()
	{
		// B树的阶数必须不小于3
		static_assert(order > 2, "The order of B-Tree can't less than 3.");

	}
	
	~BTree() 
	{
		_ReleaseNode(root);
		// 用于检测堆内存泄漏
		_CrtDumpMemoryLeaks();
	}

	// 插入一个键值对
	bool Insert(const KeyValuePair& pair)
	{
		return _Insert(pair).second;
	}
	
	// 插入一个键值对
	bool Insert(const KeyType& key, const ValueType& value)
	{
		return Insert(KeyValuePair(key, value));
	}

	// 删除一个键值对
	bool Erase(const KeyType& key)
	{
		// 如果根为空则直接返回false
		if (root == nullptr)
			return false;
		// 寻找节点是否存在
		std::pair<BTreeNode*, std::size_t> find_res = _Find_Insert_Pos(key);
		BTreeNode* node = find_res.first;
		std::size_t i = find_res.second;
		// 若未找到存有该关键字的键值对，直接返回
		if (node->numOfKeys == i || node->pairs[i]->key != key)
			return false;


		// 情况1: 该节点是内部节点
		if (node->childs[0] != nullptr)
		{
			// 情况1a: 寻找前驱所在节点，若关键字数目不小于(order + 1) / 2，改为将前驱
			// 移到该节点内找到的键值对位置，删除原来的键值对。
			
			// 寻找前驱
			BTreeNode* predNode = node->childs[i];
			while (predNode->childs[predNode->numOfKeys] != nullptr)
				predNode = predNode->childs[predNode->numOfKeys];
			if (predNode->numOfKeys >= (order + 1) / 2)
			{
				delete node->pairs[i];
				node->pairs[i] = predNode->pairs[--predNode->numOfKeys];
				predNode->pairs[predNode->numOfKeys] = nullptr;
				count--;
				return true;
			}

			// 情况1b: 寻找后继所在节点，若关键字数目不小于(order + 1) / 2，改为将后继
			// 移到该节点内找到的键值对位置，删掉原来的键值对

			// 寻找后继
			BTreeNode* succNode = node->childs[i + 1];
			while (succNode->childs[0] != nullptr)
				succNode = succNode->childs[0];
			if (succNode->numOfKeys >= (order + 1) / 2)
			{
				delete node->pairs[i];
				node->pairs[i] = succNode->pairs[0];
				succNode->numOfKeys--;
				for (i = 0; i < succNode->numOfKeys; ++i)
					succNode->pairs[i] = succNode->pairs[i + 1];
				succNode->pairs[i] = nullptr;
				count--;
				return true;
			}

			// 情况1c->3: 若前驱后继都不满足关键字不小于(order + 1) / 2的条件，则选择用
			//             前驱替代删除内部节点，然后转为情况3

			// 交换两个键值对，修改node指向前驱，i为最后一个关键字的索引
			std::swap(predNode->pairs[predNode->numOfKeys - 1], node->pairs[i]);
			node = predNode;
			i = predNode->numOfKeys - 1;
		}

		// 此时该节点为叶节点 
		// 首先直接删除该键值对
		delete node->pairs[i];
		--count;
		// 后面的键值对全部左移
		for (; i < node->numOfKeys - 1; ++i)
			node->pairs[i] = node->pairs[i + 1];
		node->pairs[--node->numOfKeys] = nullptr;
		
		// 情况2 : 若删除后关键字数目不小于(order + 1) / 2 - 1，
		//         或者该叶节点同时是根节点，直接结束
		if (node->numOfKeys >= (order + 1) / 2 - 1 || node == root)
			return true;

		// 寻找该节点为父亲的第几个节点
		BTreeNode* parent;
		std::size_t childIndex;


		// 以下操作可能会涉及递归
		// 当删除的关键字此时不在树中 且 破坏了B树
		// 性质时，则不断向上调整直至满足性质
		while (node != root && node->numOfKeys < (order + 1) / 2 - 1)
		{
			parent = node->parent;
			for (childIndex = 0; childIndex < parent->numOfKeys + 1; ++childIndex)
				if (parent->childs[childIndex] == node)
					break;
			// 情况3a: 该节点存在左兄弟节点，且左兄弟节点关键字数目不小于
			// (order + 1) / 2，则将父亲节点对应关键字移到该节点，并将
			// 左兄弟节点最大的关键字填补回到父亲节点的位置上。最后结束
			if (childIndex > 0 && parent->childs[childIndex - 1]->numOfKeys >= (order + 1) / 2)
			{
				// 左兄
				BTreeNode* leftBro = parent->childs[childIndex - 1];
				// 该节点的键值对和孩子全部右移
				for (std::size_t pos = node->numOfKeys; pos > 0; --pos)
					node->pairs[pos] = node->pairs[pos - 1];
				for (std::size_t pos = node->numOfKeys + 1; pos > 0; --pos)
					node->childs[pos] = node->childs[pos - 1];

				// 最左边加上父亲节点对应键值对和左兄节点最右边的孩子
				node->pairs[0] = parent->pairs[childIndex - 1];
				node->numOfKeys++;
				node->childs[0] = leftBro->childs[leftBro->numOfKeys];
				// 如果该节点不是叶节点，记得更换父亲到该节点
				if (node->childs[0] != nullptr)
					node->childs[0]->parent = node;
				parent->pairs[childIndex - 1] = leftBro->pairs[--leftBro->numOfKeys];
				leftBro->pairs[leftBro->numOfKeys] = nullptr;
				return true;
				
			}

			// 情况3b: 该节点存在右兄弟节点，且右兄弟节点关键字数目不小于
			// (order + 1) / 2，则将父亲节点对应关键字移到该节点，并将
			// 右兄弟节点最小的关键字填补回到父亲节点的位置上。最后结束
			if (childIndex < parent->numOfKeys && parent->childs[childIndex + 1]->numOfKeys >= (order + 1) / 2)
			{
				// 右兄
				BTreeNode* rightBro = parent->childs[childIndex + 1];
				// 最右边加上父亲节点的对应键值对和右兄节点最左边的孩子
				node->pairs[node->numOfKeys++] = parent->pairs[childIndex];
				node->childs[node->numOfKeys] = rightBro->childs[0];
				// 如果该节点不是叶节点，记得更换父亲到该节点
				if (node->childs[node->numOfKeys] != nullptr)
					node->childs[node->numOfKeys]->parent = node;
				parent->pairs[childIndex] = rightBro->pairs[0];
				for (std::size_t pos = 0; pos < rightBro->numOfKeys - 1; ++pos)
					rightBro->pairs[pos] = rightBro->pairs[pos + 1];
				for (std::size_t pos = 0; pos < rightBro->numOfKeys; ++pos)
					rightBro->childs[pos] = rightBro->childs[pos + 1];
				rightBro->childs[rightBro->numOfKeys] = nullptr;
				rightBro->pairs[--rightBro->numOfKeys] = nullptr;
				return true;
			}



			// 情况3c: 该节点存在左兄弟节点，此时无法向两边借到键值对，需要与左
			//         兄弟节点的键值对，以及对应父亲键值对进行合并操作
			if (childIndex > 0)
			{
				// 使用左兄
				BTreeNode* leftBro = parent->childs[childIndex - 1];
				// 父亲节点对应关键字和该节点首孩子加到左兄
				leftBro->pairs[leftBro->numOfKeys++] = parent->pairs[childIndex - 1];
				leftBro->childs[leftBro->numOfKeys] = node->childs[0];
				// 如果该节点不是叶节点，记得更换父亲到左兄
				if (leftBro->childs[0] != nullptr)
					leftBro->childs[leftBro->numOfKeys]->parent = leftBro;
				// 该节点的关键字和孩子也加到左兄
				for (std::size_t pos = 0; pos < node->numOfKeys; ++pos)
				{
					leftBro->pairs[leftBro->numOfKeys++] = node->pairs[pos];
					leftBro->childs[leftBro->numOfKeys] = node->childs[pos + 1];
					// 如果该节点不是叶节点，记得更换父亲到左兄
					if (leftBro->childs[0] != nullptr)
						leftBro->childs[leftBro->numOfKeys]->parent = leftBro;
					node->pairs[pos] = nullptr;
				}
				// 删除该节点
				delete node;
				parent->childs[childIndex] = nullptr;
				// 关键字和节点左移
				for (std::size_t pos = childIndex - 1; pos < parent->numOfKeys - 1; ++pos)
				{
					parent->pairs[pos] = parent->pairs[pos + 1];
					parent->childs[pos + 1] = parent->childs[pos + 2];
				}
				parent->childs[parent->numOfKeys] = nullptr;
				parent->pairs[--parent->numOfKeys] = nullptr;
				// 若父节点为根节点，且已经没有键值对了，此时删掉父节点，令左兄成为新根结束
				if (root == parent && parent->numOfKeys == 0)
				{
					delete parent;
					root = leftBro;
					root->parent = nullptr;
					return true;
				}
				//向上回溯查看父亲节点是否满足条件
				node = leftBro->parent;
				
			}
			// 情况3d: 该节点存在右兄弟节点，此时无法向两边借到键值对，需要与右
			//         兄弟节点的键值对，以及对应父亲键值对进行合并操作
			else
			{
				// 使用右兄
				BTreeNode* rightBro = parent->childs[childIndex + 1];
				// 父亲节点对应关键字和右兄首孩子加到该节点
				node->pairs[node->numOfKeys++] = parent->pairs[childIndex];
				node->childs[node->numOfKeys] = rightBro->childs[0];
				// 如果该节点不是叶节点，记得更换父亲到该节点
				if (node->childs[0] != nullptr)
					node->childs[node->numOfKeys]->parent = node;
				// 右兄节点关键字叶加到该节点
				for (std::size_t pos = 0; pos < rightBro->numOfKeys; ++pos)
				{
					node->pairs[node->numOfKeys++] = rightBro->pairs[pos];
					node->childs[node->numOfKeys] = rightBro->childs[pos + 1];
					// 如果该节点不是叶节点，记得更换父亲到该节点
					if (node->childs[0] != nullptr)
						node->childs[node->numOfKeys]->parent = node;
					rightBro->pairs[pos] = nullptr;
				}
				// 删除右兄弟节点
				delete rightBro;
				parent->childs[childIndex + 1] = nullptr;
				// 关键字和节点左移
				for (std::size_t pos = childIndex; pos < parent->numOfKeys - 1; ++pos)
				{
					parent->pairs[pos] = parent->pairs[pos + 1];
					parent->childs[pos + 1] = parent->childs[pos + 2];
				}
				parent->childs[parent->numOfKeys] = nullptr;
				parent->pairs[--parent->numOfKeys] = nullptr;
				// 若父节点为根节点，且已经没有键值对了，此时删掉父节点，令该节点成为新根结束
				if (root == parent && parent->numOfKeys == 0)
				{
					delete parent;
					root = node;
					root->parent = nullptr;
					return true;
				}
				//向上回溯查看父亲节点是否满足条件
				node = node->parent;
			}
		}
		return true;
	}

	// 通过关键字获取或设置值
	ValueType& operator[](const KeyType& key)
	{
		return (_Insert(KeyValuePair(key, ValueType())).first)->value;
	}

	// 获取元素数目
	std::size_t Count() const
	{
		return count;
	}

	// 清空B树
	void Clear()
	{
		_ReleaseNode(root);
		root = nullptr;
	}

	// 搜索节点是否存在，找到返回键值对指针，否则返回空指针
	KeyValuePair* Find(const KeyType& key)
	{
		if (!count)
			return nullptr;
		std::pair<BTreeNode*, std::size_t> find_res = _Find_Insert_Pos(key);
		BTreeNode* node = find_res.first;
		std::size_t i = find_res.second;
		// 若索引到了末端 或者 关键字不匹配，返回空
		if (node->numOfKeys == i || node->pairs[i]->key != key)
			return nullptr;
		return node->pairs[i];
	}

	// 输出B树
	void Show()
	{
		int r = 0;
		std::vector<std::string> strs;
		_Show(root, strs, r, 0, 0);
		std::cout << "当前" << order << "阶B树元素个数: " << count << std::endl;
		for (const std::string& str : strs)
			std::cout << str << std::endl;
		std::cout << std::endl;
	}

	// 从小到大进行键值对的遍历
	template<class FuncType>
	void Traversal(FuncType func)
	{
		if (root != nullptr)
			_Traversal(root, func);
	}


private:
	// 从小到大进行键值对的遍历
	template<class FuncType>
	void _Traversal(BTreeNode* node, FuncType func)
	{
		for (std::size_t i = 0; i < node->numOfKeys; ++i)
		{
			if (node->childs[i] != nullptr)
				_Traversal(node->childs[i], func);
			func(*(node->pairs[i]));
		}
		if (node->childs[node->numOfKeys] != nullptr)
			_Traversal(node->childs[node->numOfKeys], func);
	}

	// 递归输出B树
	void _Show(BTreeNode* node, std::vector<std::string>& strs, int& row, int beg, int end)
	{
		if (count == 0)
			return;
		std::ostringstream oss;
		if (end > 0)
		{
			for (int i = 0; i <= beg; ++i)
				oss.put(' ');
			oss.put('|');

			for (int i = beg + 2; i < end; ++i)
				oss.put('-');
		}
		oss << '<' << node->numOfKeys << '|';
		for (size_t i = 0; i < node->numOfKeys; ++i)
			oss << ' ' << node->pairs[i]->key ;
		oss << '>';
		strs.push_back(oss.str());

		if (row > 0)
		{
			std::size_t pos = strs[row - 1].find('|');
			while (pos < strs[row - 1].size() && pos < strs[row].size())
			{
				if (strs[row][pos] == ' ')
					strs[row][pos] = '|';
				pos = strs[row - 1].find('|', pos + 1);
			}
		}
		
		if (node->childs[0] == nullptr)
			return;
		for (size_t i = 0; i <= node->numOfKeys; ++i)
		{
			_Show(node->childs[i], strs, ++row, end, oss.str().size());
		}
			
	}

	// 递归释放节点
	void _ReleaseNode(BTreeNode* node)
	{
		if (node == nullptr)
			return;
		// 判断该节点是否为非叶节点
		if (node->childs[0] != nullptr)
		{
			for (size_t i = 0; i < node->numOfKeys + 1; ++i)
				_ReleaseNode(node->childs[i]);
		}

		count -= node->numOfKeys;
		delete node;
	}

	// 实际插入函数，返回键值对指针和插入结果
	std::pair<KeyValuePair*, bool> _Insert(const KeyValuePair& pair)
	{
		std::pair<KeyValuePair*, bool> res;
		// 情况1: 根为空时需要新建根节点，并把键值对加到根节点上
		if (root == nullptr)
		{
			root = new BTreeNode(pair);
			count++;
			
			res.first = root->pairs[0];
			res.second = true;
			return res;
		}

		// 情况2: 寻找适用于插入的叶节点以及索引
		//         若插入后numOfKeys < order则结束
		std::pair<BTreeNode*, std::size_t> find_res = _Find_Insert_Pos(pair.key);
		BTreeNode* node = find_res.first;
		std::size_t i = find_res.second;
		// 如果已经存在相同的键值对，则直接返回存在的键值对
		if (i < node->numOfKeys && node->pairs[i]->key == pair.key)
		{
			res.first = node->pairs[i];
			res.second = false;
			return res;
		}

		// 将i后面的键值对全部往后挪供插入
		for (std::size_t j = node->numOfKeys; j > i; j--)
			node->pairs[j] = node->pairs[j - 1];
		// 插入键值对，保存结果
		res.first = node->pairs[i] = new KeyValuePair(pair);
		res.second = true;
		// 计数增加
		node->numOfKeys++;
		count++;
		// 情况3：若插入后当前节点 numOfKeys == order 则进行分裂操作
		//        向上检查直到节点不满足 numOfKeys == order
		while (node != nullptr && node->numOfKeys == order)
		{
			// 分裂点索引
			size_t midIndex = order / 2;


			BTreeNode* parent;
			BTreeNode* brother;
			// 若存在父亲节点，则添加一个兄弟节点，然后比索引mid
			// 小的留在这里，比索引mid大的放入新的兄弟节点，索引
			// mid的放入父亲中
			if (node->parent != nullptr)
			{
				parent = node->parent;
				// 寻找该节点为父亲的第几个孩子
				std::size_t nodeIndex = 0;
				for (nodeIndex = 0; nodeIndex < parent->numOfKeys + 1; ++nodeIndex)
					if (node == parent->childs[nodeIndex])
						break;
				// 往该节点后面插入新兄弟
				for (i = parent->numOfKeys + 1; i > nodeIndex + 1; --i)
					parent->childs[i] = parent->childs[i - 1];
				brother = parent->childs[nodeIndex + 1] = new BTreeNode(parent);
				// 在父亲节点中插入新键值对
				for (i = parent->numOfKeys; i > nodeIndex; --i)
					parent->pairs[i] = parent->pairs[i - 1];
				parent->pairs[nodeIndex] = node->pairs[midIndex];
				node->pairs[midIndex] = nullptr;
			}
			// 若不存在父亲节点，则新建根节点，给根节点创建右孩子，左孩子绑定node
			else
			{
				root = parent = new BTreeNode();
				parent->pairs[0] = node->pairs[midIndex];
				parent->childs[0] = node;
				parent->childs[0]->parent = parent;
				// 创建兄弟
				brother = parent->childs[1] = new BTreeNode(parent);
				node->pairs[midIndex] = nullptr;
			}

			parent->numOfKeys++;
			// 将mid往后的键值对全都搬运到兄弟节点中
			for (i = midIndex + 1; i < order; ++i)
			{
				brother->pairs[i - midIndex - 1] = node->pairs[i];
				node->pairs[i] = nullptr;
			}
			node->numOfKeys = midIndex;
			brother->numOfKeys = order - midIndex - 1;
			// 将节点右半部分的所有孩子均分给兄弟
			for (i = midIndex + 1; i < order + 1; ++i)
			{
				brother->childs[i - midIndex - 1] = node->childs[i];
				// 记得将孩子的父亲给兄弟
				if (brother->childs[i - midIndex - 1] != nullptr)
					brother->childs[i - midIndex - 1]->parent = brother;
				node->childs[i] = nullptr;
			}
			node = node->parent;
		}
		return res;
	}

	// 寻找合适的插入位置
	std::pair<BTreeNode*, std::size_t> _Find_Insert_Pos(const KeyType& key)
	{
		BTreeNode* node = root;
		std::size_t i;
		while (true)
		{
			// 寻找关键字所在的区间
			// i [ 0  |   1  |  2   |  3  | ... |  N-1  |  N  ] 
			//       key0   key1  key2     ...        keyN-1
			for (i = 0; i < node->numOfKeys; ++i)
			{
				// 若已经存在相同关键字的节点，直接返回已经存在的键值对
				if (key == node->pairs[i]->key)
				{
					return std::make_pair(node, i);
				}
				if (key < node->pairs[i]->key)
					break;
			}
			// 若存在指定的子节点，跳入子节点
			if (node->childs[i] != nullptr)
				node = node->childs[i];
			else
				break;
		}
		return std::make_pair(node, i);
	}

private:
	BTreeNode* root;		// B树根节点
	std::size_t count;		// 关键字数目
};

#endif

