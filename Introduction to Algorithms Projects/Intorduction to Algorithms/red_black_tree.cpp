//////////////////////////////////////////////////////////////////////////  
///    COPYRIGHT NOTICE  
///    Copyright (c) 2009
///    All rights reserved.  
///  
/// @file		red_black_tree.cpp
/// @brief		red_black_tree.cpp的简短描述 
///  			
///  			red_black_tree.cpp的详细描述
/// 
/// @author		谭川奇	chuanqi.tan(at)gmail.com 
/// @date		2011/05/30
/// @version 	1.0
///  
///  
///    修订说明：最初版本  
//////////////////////////////////////////////////////////////////////////  

#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <iomanip>
#include <limits>
#include <fstream>
using namespace std;

namespace chapter13
{
	/// 红黑树
	template<typename TKey, typename TValue>
	class RBTree
	{
	public:
		/// 红黑树中结点颜色的枚举
		enum RBTreeNodeColor
		{
			BLACK,			///< 黑色
			RED				///< 红色			
		};

		/// 红黑树中的结点
		struct RBTreeNode
		{
			TKey				Key;
			TValue				Value;
			RBTreeNodeColor		Color;
			RBTreeNode			*Parent;
			RBTreeNode			*Left;
			RBTreeNode			*Right;

			/// 检查是否为非哨兵结点（非nil）
			/// @return			该结点是否为有效结点，即不为nil结点
			/// @retval	true	非nil结点
			/// @retval false	nil结点
			inline bool IsValid() const
			{
				return (this != s_nil);
			}
		};
	
		RBTree()
		{
			if (!s_nil)
			{
				//叶子结点是一个特殊的黑结点
				s_nil = new RBTreeNode();
				s_nil->Color = BLACK;
			}

			_root = s_nil;
		}

		~RBTree()
		{
			_RecursiveReleaseNode(_root);
		}

		bool Insert(TKey key, TValue value)
		{
			if (Search(key)->IsValid())
			{//key重复，添加失败
				return false;
			}
			else
			{
				//新添加的结点为红结点，且Left=Right=s_nil
				RBTreeNode *new_node = new RBTreeNode();
				new_node->Key = key;
				new_node->Value = value;
				new_node->Color = RED;
				new_node->Left = new_node->Right = s_nil;

				_InsertAsNormalBSTree(new_node);
				_InsertFixup(new_node);
				return true;
			}
		}

		bool Delete(TKey key)
		{
			if (Search(key)->IsValid())
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		RBTreeNode * Search(TValue const &value)
		{
			RBTreeNode *node = _root;
			while (node != s_nil && node->Value != value)
			{
				node = (value < node->Value ? node->Left : node->Right); 
			}
			return node;
		}

		bool Empty()
		{
			return !(_root->IsValid());
		}

		/// 显示当前二叉查找树的状态
		/// 
		/// 使用dot描述当前红黑树
		void Display() const
		{
			ofstream dot_file("c:\\red_back_tree.dot");

			dot_file << "digraph graphname" << (rand() % 1000) << "{" << endl
				<< "    node [shape = record,height = .1];" << endl;
			_Display(dot_file, _root);
			dot_file << "}" << endl;

			dot_file.close();
			system("dot -Tpng c:\\red_back_tree.dot -o c:\\red_back_tree.png");
			system("c:\\red_back_tree.png");
		}

	private:
		void _RecursiveReleaseNode(RBTreeNode *node)
		{
			if (node->IsValid())
			{			
				_RecursiveReleaseNode(node->Left);
				_RecursiveReleaseNode(node->Right);
				delete node;
			}
		}

		void _Display(ofstream &dot_file, RBTreeNode *node) const
		{
			if (node->IsValid())
			{
				dot_file << "    node" << node->Value << "[label = \"<f0>|<f1>" << node->Value << "|<f2>\", color = " << (node->Color == RED ? "red" : "black") << "];" << endl;

				if (node->Left->IsValid())
				{
					dot_file << "    \"node" << node->Value << "\":f0 -> \"node" << node->Left->Value << "\":f1;" << endl;
					_Display(dot_file, node->Left);
				}

				if (node->Right->IsValid())
				{
					dot_file << "    \"node" << node->Value << "\":f2 -> \"node" << node->Right->Value << "\":f1;" << endl;
					_Display(dot_file, node->Right);
				}
			}
		}

		/// 将一个结点简单地加入红黑树
		/// 
		/// 视该红黑树为普通的二叉查找树简单的进行插入操作，需要在此之后调整以满足红黑树的性质
		/// @notes		一定要保证node->Key一定是一个新的值，否则会无限循环，在这里不检查
		void _InsertAsNormalBSTree( RBTreeNode *node ) 
		{
			if (!_root->IsValid())
			{//插入的是第1个节点
				_root = node;
				_root->Left = _root->Right = _root->Parent = s_nil;
				_root->Color = BLACK;
				return;
			}

			//非第1个节点
			RBTreeNode *current_node = _root;

			while (true)
			{
				RBTreeNode *&next_node_pointer = (node->Key > current_node->Key ? current_node->Right : current_node->Left);
				if (next_node_pointer->IsValid())
				{
					current_node = next_node_pointer;
				}
				else
				{//进行真正的插入操作
					node->Parent = current_node;
					next_node_pointer = node;
					break;
				}
			}
		}

		/// 对插入操作的修正
		/// 
		/// 由于对红黑树的插入操作破坏了红黑树的性质，所以需要对它进行修正
		/// @notes		node的结点是需要处理的结点，由于它破坏了红黑性质，它一定是红结点
		void _InsertFixup( RBTreeNode * node ) 
		{
			while (node->Parent->Color == RED)
			{
				//标识node的父结点是否为node的祖父结点的左孩子
				bool parent_is_left_child_flag = (node->Parent == node->Parent->Parent->Left);		
				//叔叔结点
				RBTreeNode *uncle = parent_is_left_child_flag ? node->Parent->Parent->Right : node->Parent->Parent->Left;
				if (uncle->Color == RED)
				{//case1
					node->Parent->Color = BLACK;
					uncle->Color = BLACK;
					node->Parent->Parent->Color = RED;
					node = node->Parent->Parent;
				}
				else 
				{
					if (node == (parent_is_left_child_flag ? node->Parent->Right : node->Parent->Left))
					{//case2
						node = node->Parent;
						parent_is_left_child_flag ? _LeftRotate(node) : _RightRotate(node);
					}

					//case3
					node->Parent->Color = BLACK;
					node->Parent->Parent->Color = RED;
					parent_is_left_child_flag ? _RightRotate(node->Parent->Parent) : _LeftRotate(node->Parent->Parent);
				}				
			}

			//处理性质2被破坏只需要简简单单一句话
			_root->Color = BLACK;
		}

		/// 左旋
		void _LeftRotate( RBTreeNode * node ) 
		{
			if (!(node->IsValid() && node->Right->IsValid()))
			{//左旋操作要求对非哨兵进行操作，并且要求右孩子也不是哨兵
				throw std::exception("左旋操作要求对非哨兵进行操作，并且要求右孩子也不是哨兵");
			}
			else
			{
				//node的右孩子
				RBTreeNode *right_son = node->Right;

				node->Right = right_son->Left;
				if (right_son->Left->IsValid())
				{
					right_son->Left->Parent = node;
				}
				right_son->Parent = node->Parent;
				if (!(node->Parent->IsValid()))
				{
					_root = right_son;
				}
				else
				{
					if (node == node->Parent->Left)
					{
						node->Parent->Left = right_son;
					}
					else
					{
						node->Parent->Right = right_son;
					}
				}
				right_son->Left = node;
				node->Parent = right_son;
			}
		}

		/// 右旋
		void _RightRotate( RBTreeNode * node ) 
		{
			if (!(node->IsValid() && node->Left->IsValid()))
			{//右旋操作要求对非哨兵进行操作，并且要求左孩子也不是哨兵
				throw std::exception("右旋操作要求对非哨兵进行操作，并且要求左孩子也不是哨兵");
			}
			else
			{
				//node的左孩子
				RBTreeNode *left_son = node->Left;

				node->Left = left_son->Right;
				if (left_son->Right->IsValid())
				{
					left_son->Right->Parent = node;
				}
				left_son->Parent = node->Parent;
				if (!(node->Parent->IsValid()))
				{
					_root = left_son;
				}
				else
				{
					if (node == node->Parent->Left)
					{
						node->Parent->Left = left_son;
					}
					else
					{
						node->Parent->Right = left_son;
					}
				}
				left_son->Right = node;
				node->Parent = left_son;
			}
		}





		RBTreeNode *_root;				///< 根结点

		static RBTreeNode *s_nil;		///< 红黑树的叶子结点（哨兵）
	};


	template<typename TKey, typename TValue>
	typename RBTree<TKey, TValue>::RBTreeNode * RBTree<TKey, TValue>::s_nil = NULL;

	int test()
	{
		int init[] = {11, 2, 14, 1, 7, 15, 5, 8};
		RBTree<int, int> bst;
		for (int i = 0; i < sizeof(init) / sizeof(init[0]); ++i)
		{
			bst.Insert(init[i], init[i]);
		}
		bst.Insert(4, 4);
		bst.Display();

		//用随机值生成一棵二叉查找树
		for (int i = 0; i < 100; ++i)
		{
			int v = rand() % 100;
			bst.Insert(v, v);						
		}
		//bst.Insert(99, 99);
		bst.Display();

		return 0;
	}
}