#ifndef _BTREE_H_
#define _BTREE_H_
#include <utility>
#include <crtdbg.h>
#include <sstream>
#include <string>
#include <vector>


// ��ֵ��
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


// <KeyType�ؼ�������, ValueTypeֵ����, order����>
// �Լ�ֵ����ʽ�洢��B�����������벻С��3
// ÿ���Ǹ��ڵ������ɵļ�ֵ������ΧΪ[(order + 1) / 2 - 1, order - 1] 
template <class KeyType, class ValueType, std::size_t order = 3> 
class BTree
{
public:
	typedef KeyValuePair<KeyType, ValueType> KeyValuePair;

private:
	struct BTreeNode
	{
		std::size_t numOfKeys;			// �ڵ�ؼ�����Ŀ
		KeyValuePair* pairs[order];		// ��ֵ��ָ������
		BTreeNode* childs[order + 1];	// ����ָ�����飬Ԥ��һ�������λ�������
		BTreeNode* parent;				// ���ڵ�

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
		// B���Ľ������벻С��3
		static_assert(order > 2, "The order of B-Tree can't less than 3.");

	}
	
	~BTree() 
	{
		_ReleaseNode(root);
		// ���ڼ����ڴ�й©
		_CrtDumpMemoryLeaks();
	}

	// ����һ����ֵ��
	bool Insert(const KeyValuePair& pair)
	{
		return _Insert(pair).second;
	}
	
	// ����һ����ֵ��
	bool Insert(const KeyType& key, const ValueType& value)
	{
		return Insert(KeyValuePair(key, value));
	}

	// ɾ��һ����ֵ��
	bool Erase(const KeyType& key)
	{
		// �����Ϊ����ֱ�ӷ���false
		if (root == nullptr)
			return false;
		// Ѱ�ҽڵ��Ƿ����
		std::pair<BTreeNode*, std::size_t> find_res = _Find_Insert_Pos(key);
		BTreeNode* node = find_res.first;
		std::size_t i = find_res.second;
		// ��δ�ҵ����иùؼ��ֵļ�ֵ�ԣ�ֱ�ӷ���
		if (node->numOfKeys == i || node->pairs[i]->key != key)
			return false;


		// ���1: �ýڵ����ڲ��ڵ�
		if (node->childs[0] != nullptr)
		{
			// ���1a: Ѱ��ǰ�����ڽڵ㣬���ؼ�����Ŀ��С��(order + 1) / 2����Ϊ��ǰ��
			// �Ƶ��ýڵ����ҵ��ļ�ֵ��λ�ã�ɾ��ԭ���ļ�ֵ�ԡ�
			
			// Ѱ��ǰ��
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

			// ���1b: Ѱ�Һ�����ڽڵ㣬���ؼ�����Ŀ��С��(order + 1) / 2����Ϊ�����
			// �Ƶ��ýڵ����ҵ��ļ�ֵ��λ�ã�ɾ��ԭ���ļ�ֵ��

			// Ѱ�Һ��
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

			// ���1c->3: ��ǰ����̶�������ؼ��ֲ�С��(order + 1) / 2����������ѡ����
			//             ǰ�����ɾ���ڲ��ڵ㣬Ȼ��תΪ���3

			// ����������ֵ�ԣ��޸�nodeָ��ǰ����iΪ���һ���ؼ��ֵ�����
			std::swap(predNode->pairs[predNode->numOfKeys - 1], node->pairs[i]);
			node = predNode;
			i = predNode->numOfKeys - 1;
		}

		// ��ʱ�ýڵ�ΪҶ�ڵ� 
		// ����ֱ��ɾ���ü�ֵ��
		delete node->pairs[i];
		--count;
		// ����ļ�ֵ��ȫ������
		for (; i < node->numOfKeys - 1; ++i)
			node->pairs[i] = node->pairs[i + 1];
		node->pairs[--node->numOfKeys] = nullptr;
		
		// ���2 : ��ɾ����ؼ�����Ŀ��С��(order + 1) / 2 - 1��
		//         ���߸�Ҷ�ڵ�ͬʱ�Ǹ��ڵ㣬ֱ�ӽ���
		if (node->numOfKeys >= (order + 1) / 2 - 1 || node == root)
			return true;

		// Ѱ�Ҹýڵ�Ϊ���׵ĵڼ����ڵ�
		BTreeNode* parent;
		std::size_t childIndex;


		// ���²������ܻ��漰�ݹ�
		// ��ɾ���Ĺؼ��ִ�ʱ�������� �� �ƻ���B��
		// ����ʱ���򲻶����ϵ���ֱ����������
		while (node != root && node->numOfKeys < (order + 1) / 2 - 1)
		{
			parent = node->parent;
			for (childIndex = 0; childIndex < parent->numOfKeys + 1; ++childIndex)
				if (parent->childs[childIndex] == node)
					break;
			// ���3a: �ýڵ�������ֵܽڵ㣬�����ֵܽڵ�ؼ�����Ŀ��С��
			// (order + 1) / 2���򽫸��׽ڵ��Ӧ�ؼ����Ƶ��ýڵ㣬����
			// ���ֵܽڵ����Ĺؼ�����ص����׽ڵ��λ���ϡ�������
			if (childIndex > 0 && parent->childs[childIndex - 1]->numOfKeys >= (order + 1) / 2)
			{
				// ����
				BTreeNode* leftBro = parent->childs[childIndex - 1];
				// �ýڵ�ļ�ֵ�Ժͺ���ȫ������
				for (std::size_t pos = node->numOfKeys; pos > 0; --pos)
					node->pairs[pos] = node->pairs[pos - 1];
				for (std::size_t pos = node->numOfKeys + 1; pos > 0; --pos)
					node->childs[pos] = node->childs[pos - 1];

				// ����߼��ϸ��׽ڵ��Ӧ��ֵ�Ժ����ֽڵ����ұߵĺ���
				node->pairs[0] = parent->pairs[childIndex - 1];
				node->numOfKeys++;
				node->childs[0] = leftBro->childs[leftBro->numOfKeys];
				// ����ýڵ㲻��Ҷ�ڵ㣬�ǵø������׵��ýڵ�
				if (node->childs[0] != nullptr)
					node->childs[0]->parent = node;
				parent->pairs[childIndex - 1] = leftBro->pairs[--leftBro->numOfKeys];
				leftBro->pairs[leftBro->numOfKeys] = nullptr;
				return true;
				
			}

			// ���3b: �ýڵ�������ֵܽڵ㣬�����ֵܽڵ�ؼ�����Ŀ��С��
			// (order + 1) / 2���򽫸��׽ڵ��Ӧ�ؼ����Ƶ��ýڵ㣬����
			// ���ֵܽڵ���С�Ĺؼ�����ص����׽ڵ��λ���ϡ�������
			if (childIndex < parent->numOfKeys && parent->childs[childIndex + 1]->numOfKeys >= (order + 1) / 2)
			{
				// ����
				BTreeNode* rightBro = parent->childs[childIndex + 1];
				// ���ұ߼��ϸ��׽ڵ�Ķ�Ӧ��ֵ�Ժ����ֽڵ�����ߵĺ���
				node->pairs[node->numOfKeys++] = parent->pairs[childIndex];
				node->childs[node->numOfKeys] = rightBro->childs[0];
				// ����ýڵ㲻��Ҷ�ڵ㣬�ǵø������׵��ýڵ�
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



			// ���3c: �ýڵ�������ֵܽڵ㣬��ʱ�޷������߽赽��ֵ�ԣ���Ҫ����
			//         �ֵܽڵ�ļ�ֵ�ԣ��Լ���Ӧ���׼�ֵ�Խ��кϲ�����
			if (childIndex > 0)
			{
				// ʹ������
				BTreeNode* leftBro = parent->childs[childIndex - 1];
				// ���׽ڵ��Ӧ�ؼ��ֺ͸ýڵ��׺��Ӽӵ�����
				leftBro->pairs[leftBro->numOfKeys++] = parent->pairs[childIndex - 1];
				leftBro->childs[leftBro->numOfKeys] = node->childs[0];
				// ����ýڵ㲻��Ҷ�ڵ㣬�ǵø������׵�����
				if (leftBro->childs[0] != nullptr)
					leftBro->childs[leftBro->numOfKeys]->parent = leftBro;
				// �ýڵ�Ĺؼ��ֺͺ���Ҳ�ӵ�����
				for (std::size_t pos = 0; pos < node->numOfKeys; ++pos)
				{
					leftBro->pairs[leftBro->numOfKeys++] = node->pairs[pos];
					leftBro->childs[leftBro->numOfKeys] = node->childs[pos + 1];
					// ����ýڵ㲻��Ҷ�ڵ㣬�ǵø������׵�����
					if (leftBro->childs[0] != nullptr)
						leftBro->childs[leftBro->numOfKeys]->parent = leftBro;
					node->pairs[pos] = nullptr;
				}
				// ɾ���ýڵ�
				delete node;
				parent->childs[childIndex] = nullptr;
				// �ؼ��ֺͽڵ�����
				for (std::size_t pos = childIndex - 1; pos < parent->numOfKeys - 1; ++pos)
				{
					parent->pairs[pos] = parent->pairs[pos + 1];
					parent->childs[pos + 1] = parent->childs[pos + 2];
				}
				parent->childs[parent->numOfKeys] = nullptr;
				parent->pairs[--parent->numOfKeys] = nullptr;
				// �����ڵ�Ϊ���ڵ㣬���Ѿ�û�м�ֵ���ˣ���ʱɾ�����ڵ㣬�����ֳ�Ϊ�¸�����
				if (root == parent && parent->numOfKeys == 0)
				{
					delete parent;
					root = leftBro;
					root->parent = nullptr;
					return true;
				}
				//���ϻ��ݲ鿴���׽ڵ��Ƿ���������
				node = leftBro->parent;
				
			}
			// ���3d: �ýڵ�������ֵܽڵ㣬��ʱ�޷������߽赽��ֵ�ԣ���Ҫ����
			//         �ֵܽڵ�ļ�ֵ�ԣ��Լ���Ӧ���׼�ֵ�Խ��кϲ�����
			else
			{
				// ʹ������
				BTreeNode* rightBro = parent->childs[childIndex + 1];
				// ���׽ڵ��Ӧ�ؼ��ֺ������׺��Ӽӵ��ýڵ�
				node->pairs[node->numOfKeys++] = parent->pairs[childIndex];
				node->childs[node->numOfKeys] = rightBro->childs[0];
				// ����ýڵ㲻��Ҷ�ڵ㣬�ǵø������׵��ýڵ�
				if (node->childs[0] != nullptr)
					node->childs[node->numOfKeys]->parent = node;
				// ���ֽڵ�ؼ���Ҷ�ӵ��ýڵ�
				for (std::size_t pos = 0; pos < rightBro->numOfKeys; ++pos)
				{
					node->pairs[node->numOfKeys++] = rightBro->pairs[pos];
					node->childs[node->numOfKeys] = rightBro->childs[pos + 1];
					// ����ýڵ㲻��Ҷ�ڵ㣬�ǵø������׵��ýڵ�
					if (node->childs[0] != nullptr)
						node->childs[node->numOfKeys]->parent = node;
					rightBro->pairs[pos] = nullptr;
				}
				// ɾ�����ֵܽڵ�
				delete rightBro;
				parent->childs[childIndex + 1] = nullptr;
				// �ؼ��ֺͽڵ�����
				for (std::size_t pos = childIndex; pos < parent->numOfKeys - 1; ++pos)
				{
					parent->pairs[pos] = parent->pairs[pos + 1];
					parent->childs[pos + 1] = parent->childs[pos + 2];
				}
				parent->childs[parent->numOfKeys] = nullptr;
				parent->pairs[--parent->numOfKeys] = nullptr;
				// �����ڵ�Ϊ���ڵ㣬���Ѿ�û�м�ֵ���ˣ���ʱɾ�����ڵ㣬��ýڵ��Ϊ�¸�����
				if (root == parent && parent->numOfKeys == 0)
				{
					delete parent;
					root = node;
					root->parent = nullptr;
					return true;
				}
				//���ϻ��ݲ鿴���׽ڵ��Ƿ���������
				node = node->parent;
			}
		}
		return true;
	}

	// ͨ���ؼ��ֻ�ȡ������ֵ
	ValueType& operator[](const KeyType& key)
	{
		return (_Insert(KeyValuePair(key, ValueType())).first)->value;
	}

	// ��ȡԪ����Ŀ
	std::size_t Count() const
	{
		return count;
	}

	// ���B��
	void Clear()
	{
		_ReleaseNode(root);
		root = nullptr;
	}

	// �����ڵ��Ƿ���ڣ��ҵ����ؼ�ֵ��ָ�룬���򷵻ؿ�ָ��
	KeyValuePair* Find(const KeyType& key)
	{
		if (!count)
			return nullptr;
		std::pair<BTreeNode*, std::size_t> find_res = _Find_Insert_Pos(key);
		BTreeNode* node = find_res.first;
		std::size_t i = find_res.second;
		// ����������ĩ�� ���� �ؼ��ֲ�ƥ�䣬���ؿ�
		if (node->numOfKeys == i || node->pairs[i]->key != key)
			return nullptr;
		return node->pairs[i];
	}

	// ���B��
	void Show()
	{
		int r = 0;
		std::vector<std::string> strs;
		_Show(root, strs, r, 0, 0);
		std::cout << "��ǰ" << order << "��B��Ԫ�ظ���: " << count << std::endl;
		for (const std::string& str : strs)
			std::cout << str << std::endl;
		std::cout << std::endl;
	}

	// ��С������м�ֵ�Եı���
	template<class FuncType>
	void Traversal(FuncType func)
	{
		if (root != nullptr)
			_Traversal(root, func);
	}


private:
	// ��С������м�ֵ�Եı���
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

	// �ݹ����B��
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

	// �ݹ��ͷŽڵ�
	void _ReleaseNode(BTreeNode* node)
	{
		if (node == nullptr)
			return;
		// �жϸýڵ��Ƿ�Ϊ��Ҷ�ڵ�
		if (node->childs[0] != nullptr)
		{
			for (size_t i = 0; i < node->numOfKeys + 1; ++i)
				_ReleaseNode(node->childs[i]);
		}

		count -= node->numOfKeys;
		delete node;
	}

	// ʵ�ʲ��뺯�������ؼ�ֵ��ָ��Ͳ�����
	std::pair<KeyValuePair*, bool> _Insert(const KeyValuePair& pair)
	{
		std::pair<KeyValuePair*, bool> res;
		// ���1: ��Ϊ��ʱ��Ҫ�½����ڵ㣬���Ѽ�ֵ�Լӵ����ڵ���
		if (root == nullptr)
		{
			root = new BTreeNode(pair);
			count++;
			
			res.first = root->pairs[0];
			res.second = true;
			return res;
		}

		// ���2: Ѱ�������ڲ����Ҷ�ڵ��Լ�����
		//         �������numOfKeys < order�����
		std::pair<BTreeNode*, std::size_t> find_res = _Find_Insert_Pos(pair.key);
		BTreeNode* node = find_res.first;
		std::size_t i = find_res.second;
		// ����Ѿ�������ͬ�ļ�ֵ�ԣ���ֱ�ӷ��ش��ڵļ�ֵ��
		if (i < node->numOfKeys && node->pairs[i]->key == pair.key)
		{
			res.first = node->pairs[i];
			res.second = false;
			return res;
		}

		// ��i����ļ�ֵ��ȫ������Ų������
		for (std::size_t j = node->numOfKeys; j > i; j--)
			node->pairs[j] = node->pairs[j - 1];
		// �����ֵ�ԣ�������
		res.first = node->pairs[i] = new KeyValuePair(pair);
		res.second = true;
		// ��������
		node->numOfKeys++;
		count++;
		// ���3���������ǰ�ڵ� numOfKeys == order ����з��Ѳ���
		//        ���ϼ��ֱ���ڵ㲻���� numOfKeys == order
		while (node != nullptr && node->numOfKeys == order)
		{
			// ���ѵ�����
			size_t midIndex = order / 2;


			BTreeNode* parent;
			BTreeNode* brother;
			// �����ڸ��׽ڵ㣬�����һ���ֵܽڵ㣬Ȼ�������mid
			// С���������������mid��ķ����µ��ֵܽڵ㣬����
			// mid�ķ��븸����
			if (node->parent != nullptr)
			{
				parent = node->parent;
				// Ѱ�Ҹýڵ�Ϊ���׵ĵڼ�������
				std::size_t nodeIndex = 0;
				for (nodeIndex = 0; nodeIndex < parent->numOfKeys + 1; ++nodeIndex)
					if (node == parent->childs[nodeIndex])
						break;
				// ���ýڵ����������ֵ�
				for (i = parent->numOfKeys + 1; i > nodeIndex + 1; --i)
					parent->childs[i] = parent->childs[i - 1];
				brother = parent->childs[nodeIndex + 1] = new BTreeNode(parent);
				// �ڸ��׽ڵ��в����¼�ֵ��
				for (i = parent->numOfKeys; i > nodeIndex; --i)
					parent->pairs[i] = parent->pairs[i - 1];
				parent->pairs[nodeIndex] = node->pairs[midIndex];
				node->pairs[midIndex] = nullptr;
			}
			// �������ڸ��׽ڵ㣬���½����ڵ㣬�����ڵ㴴���Һ��ӣ����Ӱ�node
			else
			{
				root = parent = new BTreeNode();
				parent->pairs[0] = node->pairs[midIndex];
				parent->childs[0] = node;
				parent->childs[0]->parent = parent;
				// �����ֵ�
				brother = parent->childs[1] = new BTreeNode(parent);
				node->pairs[midIndex] = nullptr;
			}

			parent->numOfKeys++;
			// ��mid����ļ�ֵ��ȫ�����˵��ֵܽڵ���
			for (i = midIndex + 1; i < order; ++i)
			{
				brother->pairs[i - midIndex - 1] = node->pairs[i];
				node->pairs[i] = nullptr;
			}
			node->numOfKeys = midIndex;
			brother->numOfKeys = order - midIndex - 1;
			// ���ڵ��Ұ벿�ֵ����к��Ӿ��ָ��ֵ�
			for (i = midIndex + 1; i < order + 1; ++i)
			{
				brother->childs[i - midIndex - 1] = node->childs[i];
				// �ǵý����ӵĸ��׸��ֵ�
				if (brother->childs[i - midIndex - 1] != nullptr)
					brother->childs[i - midIndex - 1]->parent = brother;
				node->childs[i] = nullptr;
			}
			node = node->parent;
		}
		return res;
	}

	// Ѱ�Һ��ʵĲ���λ��
	std::pair<BTreeNode*, std::size_t> _Find_Insert_Pos(const KeyType& key)
	{
		BTreeNode* node = root;
		std::size_t i;
		while (true)
		{
			// Ѱ�ҹؼ������ڵ�����
			// i [ 0  |   1  |  2   |  3  | ... |  N-1  |  N  ] 
			//       key0   key1  key2     ...        keyN-1
			for (i = 0; i < node->numOfKeys; ++i)
			{
				// ���Ѿ�������ͬ�ؼ��ֵĽڵ㣬ֱ�ӷ����Ѿ����ڵļ�ֵ��
				if (key == node->pairs[i]->key)
				{
					return std::make_pair(node, i);
				}
				if (key < node->pairs[i]->key)
					break;
			}
			// ������ָ�����ӽڵ㣬�����ӽڵ�
			if (node->childs[i] != nullptr)
				node = node->childs[i];
			else
				break;
		}
		return std::make_pair(node, i);
	}

private:
	BTreeNode* root;		// B�����ڵ�
	std::size_t count;		// �ؼ�����Ŀ
};

#endif

