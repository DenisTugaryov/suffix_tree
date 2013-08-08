#include <map>
#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <vector>

enum Color {WHITE, GRAY, BLACK};

struct Node
{
  Node* _parent;
  Node* _suffix_ptr;
  int _begin_edge;
  int _end_edge;
  int _substr_length;
  bool _is_leaf;
  Color _color;
  std::map<char, Node*> _child;
  std::vector<int> _mask;

  Node(int size = 0):
    _parent(0),
    _suffix_ptr(0),
    _begin_edge(0),
    _end_edge(-1),
    _substr_length(0),
    _is_leaf(false),
    _color(WHITE),
    _mask(size, 0)
  {}

  ~Node()
  {}

  void set_white_color()
  {
    _color = WHITE;
  }

private:
  Node(const Node&);
  Node& operator=(const Node&);
};

class Suffix_tree
{
public:
  Suffix_tree(const std::string& str, const std::map<int, int>& len_to_suffix_number) :
    _str(str),
    _len_to_suffix_number(len_to_suffix_number),
    _mask_size(len_to_suffix_number.size()),
    _max_suffix_length(0)
  {
    _root = new Node(_mask_size);
    _last = _root;
    _node_set.insert(_root);
    _max_suffix = _root;

    build();
  }

  ~Suffix_tree()
  {
    for (std::set<Node*>::iterator it = _node_set.begin(); it != _node_set.end(); ++it)
      delete *it;
  }

  void get_max_common_suffix()
  {
    max_common_suffix_dfs();
    std::cout << print_max_common_suffix(_max_suffix) << std::endl;
  }

  std::string print_max_common_suffix(Node* node)
  {
    if (node == _root)
      return "";
    return print_max_common_suffix(node->_parent) + _str.substr(node->_begin_edge, node->_end_edge - node->_begin_edge + 1);
  }

private:
  const std::string _str;
  Node* _root;
  Node* _last;
  std::set<Node*> _node_set;
  std::map<int, int> _len_to_suffix_number;
  int _mask_size;
  int _max_suffix_length;
  Node* _max_suffix;

  Suffix_tree(const Suffix_tree&);
  Suffix_tree & operator= (const Suffix_tree&);

  Node* split(Node* node, char ch, int match_length)
  {
    int pos = node->_begin_edge + match_length - 1;
    Node* parent = node->_parent;

    Node* new_node = new Node(_mask_size);

    _node_set.insert(new_node);

    new_node->_parent = parent;
    new_node->_substr_length = parent->_substr_length + match_length;
    char new_edge_ch = _str[pos + 1];
    new_node->_child[new_edge_ch] = node;
    new_node->_begin_edge = node->_begin_edge;
    new_node->_end_edge = pos;

    parent->_child[ch] = new_node;

    node->_begin_edge = pos + 1;
    node->_parent = new_node;

    return new_node;
  }

  Node* fast_find(Node* node, int begin_substr, int end_substr, int& start_pos)
  {
    int length = end_substr - begin_substr + 1;

    if (length == 0 || node->_child.find(_str[begin_substr]) == node->_child.end())
      return node;

    Node* child = node->_child[_str[begin_substr]];
    int child_length = child->_end_edge - child->_begin_edge + 1;

    if (length >= child_length)
    {
      start_pos += child_length;
      return fast_find(child, begin_substr + child_length, end_substr, start_pos);
    }
    else
    {
      Node* new_node = split(child, _str[begin_substr], length);
      start_pos += length;
      return new_node;
    }
  }

  Node* honest_find(Node* node, int begin_substr, int end_substr, int& start_pos)
  {
    int length = end_substr - begin_substr + 1;

    char ch = _str[begin_substr];
    if (length <= 0 || node->_child.find(ch) == node->_child.end())
      return node;

    Node* child = node->_child[_str[begin_substr]];
    int child_length = child->_end_edge - child->_begin_edge + 1;

    int pos = begin_substr;
    int begin_edge = child->_begin_edge;

    for (int i = 1; i < child_length; ++i)
    {
      ++pos;
      ++start_pos;
      if (_str[begin_edge + i] != _str[pos])
      {
        Node* new_node = split(child, _str[begin_substr], i);
        return new_node;
      }
    }
    return honest_find(child, begin_substr + child_length, end_substr, ++start_pos);
  }

  void suspend_leaf(Node* node, int begin_leaf_index)
  {
    int length = _str.length();

    _last = new Node(_mask_size);
    _node_set.insert(_last);
    _last->_is_leaf = true;
    _last->_parent = node;
    _last->_substr_length = node->_substr_length + length - begin_leaf_index;
    _last->_begin_edge = begin_leaf_index;
    _last->_end_edge = length - 1;

    char ch = _str[begin_leaf_index];
    node->_child[ch] = _last;
  }

  void add(int pos)
  {
    Node* node = NULL;
    if (_last == _root || _last->_parent == _root)
    {
      node = honest_find(_root, pos, _str.length() - 1, pos);
    }
    else if (_last->_parent->_parent == _root)
    {
      int length = _last->_parent->_end_edge - _last->_parent->_begin_edge;
      if (length > 0)
      {
        node = fast_find(_root, _last->_parent->_begin_edge + 1, _last->_parent->_end_edge, pos);
        _last->_parent->_suffix_ptr = node;
        node = honest_find(node, pos, _str.length() - 1, pos); 
      }
      else
      {
        _last->_parent->_suffix_ptr = _root;
        node = honest_find(_root, pos, _str.length() - 1, pos);
        _last->_parent->_suffix_ptr = _root;
      }
    }
    else
    {
      pos += _last->_parent->_parent->_suffix_ptr->_substr_length;
      node = fast_find(_last->_parent->_parent->_suffix_ptr, _last->_parent->_begin_edge, _last->_parent->_end_edge, pos);
      _last->_parent->_suffix_ptr = node;
      node = honest_find(node, pos, _str.length() - 1, pos);
    }
    suspend_leaf(node, pos);
  }

  void build()
  {
    int length = _str.length();
    for (int i = 0; i < length; ++i)
      add(i);
  }

  void max_common_suffix(Node* node)
  {
    if (node->_is_leaf)
    {
      int suffix_length = node->_end_edge - node->_begin_edge + 1;
      int suffix_index = _len_to_suffix_number.lower_bound(suffix_length)->second;
      node->_mask[suffix_index] = 1;
      if (node->_parent != NULL)
        node->_parent->_mask[suffix_index] = 1;
    }
    else if (node != _root)
    {
      int size = node->_mask.size();
      bool is_common = true;
      for (int i = 0; i < size; ++i)
      {
        if (node->_mask[i] > node->_parent->_mask[i])
          node->_parent->_mask[i] = node->_mask[i];
        is_common *= node->_mask[i];
      }

      if (is_common && node->_substr_length > _max_suffix_length)
      {
        _max_suffix_length = node->_substr_length;
        _max_suffix = node;
      }
    }  
  }

  void max_common_suffix_dfs()
  {
    for (std::set<Node*>::iterator it = _node_set.begin(); it != _node_set.end(); ++it)
      (*it)->set_white_color();
    dfs_visit(_root);
  }

  void dfs_visit(Node* node)
  {
    node->_color = GRAY;
    for(std::map<char, Node*>::iterator it = node->_child.begin(); it != node->_child.end(); ++it)
      if (it->second->_color == WHITE)
        dfs_visit(it->second);
    node->_color = BLACK;
    max_common_suffix(node);
  }
};


int main()
{
  int str_number = 0;
  std::cin >> str_number;

  std::vector<std::string> vstr;
  std::string str_tmp;
  for (int i = 0; i < str_number; ++i)
  {
    std::cin >> str_tmp;
    vstr.push_back(str_tmp);
  }

  std::map<int, int> len_to_suffix_number;
  std::string str = "";
  int tmp = 0;
  const size_t FIRST_SEPARATE_SYMBOL_INDEX = 48;
  for (int i = str_number - 1; i >= 0; --i)
  {
    tmp += vstr[i].length() + 1;
    len_to_suffix_number[tmp] = i;
    str += vstr[str_number - i - 1] + char(str_number - i - 1 + FIRST_SEPARATE_SYMBOL_INDEX);
  }

  Suffix_tree tree(str, len_to_suffix_number);

  tree.get_max_common_suffix();

  return 0;
}