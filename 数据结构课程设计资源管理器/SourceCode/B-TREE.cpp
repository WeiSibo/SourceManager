#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

struct Index{
  string id;
  int loc;
};

//B-Tree 结点
class BTreeNode{
protected:
  //string *keys;//存储关键字的数据
  Index *keys;
  int t; //最小度 (决定了key的数量范围)
  BTreeNode **C; //存储孩子结点的数组
  int n; //现在key的个数
  bool leaf; //如果这个结点是一个叶子 则为TRUE , 否则 为FALSE
public:
  BTreeNode(int _t, bool _leaf);
  void traverse(); //遍历以该结点为根的子树中所有结点
  BTreeNode *search(Index k);//在以该节点为根的子树中查找键为k的结点

  void insertNonFull(Index k);
  void splitChild(int i, BTreeNode *y);

friend class BTree;
};

class BTree{
protected:
  BTreeNode *root; //根节点的指针
  int t; //最小度
public:
  BTree(int _t)
  { root = NULL; t = _t;}
  void traverse()
  { if(root != NULL) root->traverse(); }
  BTreeNode *search(Index k)
  {
    if(root == NULL) return NULL;
    else return root->search(k);
  }
  void insert(Index _k);
};

BTreeNode::BTreeNode(int _t, bool _leaf)
{
  t = _t;
  leaf = _leaf;
  keys = new Index[2*t-1]; //一个结点key最多有2*t-1个
  C = new BTreeNode *[2*t]; //一个结点孩子最多有2*t个
  n = 0; //新建结点当前key的数量为0
}

void BTreeNode::traverse()
{
  // There are n keys and n+1 children, travers through n keys
  // and first n children
  int i;
  for(i=0;i<n;++i)
  {
    // If this is not leaf, then before printing key[i],
    // traverse the subtree rooted with child C[i].
    if(leaf == false)
      C[i]->traverse();
    cout<<" "<<keys[i].id;
  }
  // Print the subtree rooted with last child
  if (leaf == false)
    C[i]->traverse();
}

BTreeNode *BTreeNode::search(Index k)
{
  // Find the first key greater than or equal to k
  int i = 0;
  while(i < n && k.id > keys[i].id)
   ++i;

  //If the found key is equal to k, return this node
  if(keys[i].id == k.id)
    return this;
  // If the key is not found here and this is a leaf node
  if(leaf == true)
    return NULL;
  // Go to the appropriate child
  return C[i]->search(k);
}

void BTree::insert(Index k)
{
  //如果树为空
  if(root == NULL)
  {
    root = new BTreeNode(t, true);
    root->keys[0] = k; //插入 key
    root->n = 1; //更新当前root结点的key的个数为1
  }
  else //树不为空
  {
    // If root is full, then tree grows in height
    if(root->n == 2*t-1)
    {
      BTreeNode *s = new BTreeNode(t, false);
      // Make old root as child of new root
      s->C[0] = root;
      // Split the old root and move 1 key to the new root
      s->splitChild(0, root);
      // New root has two children now.  Decide which of the
      // two children is going to have new key
      int i = 0;
      if(s->keys[0].id < k.id)
        ++i;
      s->C[i]->insertNonFull(k);
      //change root
      root = s;
    }
    else
      root->insertNonFull(k);
  }
}

// A utility function to insert a new key in this node
// The assumption is, the node must be non-full when this
// function is called
void BTreeNode::insertNonFull(Index k)
{
    // Initialize index as index of rightmost element
    int i = n-1;

    // If this is a leaf node
    if(leaf == true)
    {
      // The following loop does two things
      // a) Finds the location of new key to be inserted
      // b) Moves all greater keys to one place ahead
      while(i >= 0 && keys[i].id > k.id)
      {
        keys[i+1] = keys[i];
        --i;
      }
      //Insert the new key at found location
      keys[i+1] = k;
      n = n+1;
    }
    else //If this node is not leaf
    {
      //Find the child which is going to have the new key
      while(i >= 0 && keys[i].id > k.id)
        --i;

      //See if the found child is full
      if(C[i+1]->n == 2*t-1)
      {
        // If the child is full, then split it
        splitChild(i+1, C[i+1]);

        // After split, the middle key of C[i] goes up and
        // C[i] is splitted into two.  See which of the two
        // is going to have the new key
        if(keys[i+1].id < k.id)
          ++i;
      }
      C[i+1]->insertNonFull(k);
    }
}

// A utility function to split the child y of this node
// Note that y must be full when this function is called
void BTreeNode::splitChild(int i, BTreeNode *y)
{
    //Create a new node which is going to storee (t-1) keys
    //of y
    BTreeNode *z = new BTreeNode(y->t, y->leaf);
    z->n = t-1;

    //Copy the last (t-1) keys of y to z
    for(int j=0;j<t-1;j++)
      z->keys[j] = y->keys[j+t];

    //Copy the last t children of y to z
    if(y->leaf == false)
    {
      for(int j=0;j<t;++j)
        z->C[j] = y->C[j+t];
    }

    //Reduce the number of keys in y
    y->n = t-1;

    // Since this node is going to have a new child,
    // create space of new child
    for(int j=n;j>=i+1;--j)
      C[j+1] = C[j];

    //Link the new child to this node
    C[i+1] = z;

    // A key of y will move to this node. Find the location of
    // new key and move all greater keys one space ahead
    for(int j=n-1;j>=i;--j)
      keys[j+1] = keys[j];

    // Copy the middle key of y to this node
    keys[i] = y->keys[t-1];

    //Increment count of keys in this node
    n = n+1;
}

int main()
{
  BTree t(3);
  t.insert({"10",1});
  t.insert({"20",2});
  t.insert({"5",3});
  t.insert({"6",4});
  t.insert({"12",5});
  t.insert({"30",6});
  t.insert({"7",7});
  t.insert({"17",8});

  t.traverse();
  cout<<endl;
  Index k = {"20",2};
  if(t.search(k) != NULL)
    cout<<"exist"<<endl;
  else
    cout<<"not exist"<<endl;
}
