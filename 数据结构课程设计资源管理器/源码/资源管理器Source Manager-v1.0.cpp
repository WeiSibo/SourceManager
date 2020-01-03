/*
	数据结构课程设计 A-资源管理器
*/
#include <iostream>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <fstream>
using namespace std;
//资源管理器注意事项:1.只能执行所提供的命令的操作
//                 2.整个系统设定为允许存在重名文件，不允许存在重名文件夹
//                 3.由于系统会真正的去调用CMD命令，所以谨慎使用，不要误操删掉重要文件
//                 4.系统的目录与系统缓存文件中的目录一致，并不是完全把Windows里的整个文件系统搬过来
//                  所以你在系统中只能看到D: E: F:盘符以及你在本系统中在这些盘符下创建的文件夹或文件，
//                  也就是说只能做到本系统的目录结构在winodws系统中完全映射，但不能将windows系统目录映射到本系统
//                 5.**重要 每次退出程序，请输入-1正常退出，如果直接关闭程序，会导致数据丢失

//兄弟孩子树存储文件系统形成的多叉树
typedef struct TreeNode{
  string fileName; //文件名
  int fileType; //文件类型  fileType为1代表文件夹  为0代表文件(.txt)
  int nodeLevel; //结点的层次，注意并不是在树中的深度，因为采用的是孩子兄弟表示法 存这个数主要是为了输出文件系统目录
  struct TreeNode *pre; //前驱
  struct TreeNode *parent; //父亲
  struct TreeNode *firstChild; //第一个孩子
  struct TreeNode *nextSibling; //下一个兄弟
}TreeNode, *Tree;

//该结构体是存储 文件系统的可外存数据（除了指针以外的） 用于 保存 和 读入 文件系统
typedef struct MainData{
  string filename;
  int fileType;
  int nodeLevel;
}MainData;
MainData *preorder; //先序遍历得到的可外存数据序列
MainData *midorder; //中序遍历得到的可外存数据序列
int LEN; //树遍历序列的长度

//存树的链栈的实现
typedef struct Stack{
  Tree data;
  struct Stack *next;
}Stack, *LinkStack;

bool initStack(LinkStack &S);
bool Push(LinkStack &S, Tree T);
bool Pop(LinkStack &S, Tree &T);
bool getTop(LinkStack S, Tree &T);
bool isStackEmpty(LinkStack S);
void createFileSystem(Tree &T);
void createNewTreeNode(Tree &T, string name, int type, int level);
bool initFilePath(Tree &T);
void saveFileSystem(Tree T);
void saveFileSystemAgain(Tree T);
void traverseFilePath(Tree T);
Tree findFileNode(Tree T, string filename);
Tree findFileNode(Tree T, string parentname, string filename);
Tree findFirstNextEmptyRoot(Tree T);
bool checkSameName(Tree T, string filename, int type);
string getFilePath(Tree T, string filename);
string getFilePath(Tree T, string parentname,string filename);
bool createNewFile(Tree &T, string filename, string newfilename, int type);
bool deleteFile(Tree &T,string parentname, string defilename);
bool deleteFile(Tree &T, string defilename);
bool reNameFile(Tree &T, string filename, string newfilename);
bool reNameFile(Tree &T, string parentname,string filename, string newfilename);
bool lsFileChild(Tree T, string filename);
bool moveFile(Tree &T, string mvparentname, string mvfilename, string tofilename);
bool copyFile(Tree &T, string parentname, string cpfilename, string tofilename);
void readFile();
Tree readSystemFromTxt(MainData *preorder, MainData *midorder, int len);
void connectPreParent(Tree &T);
void checkLocalWindows(string cmd);
void menu();
void run(Tree &T);

int main()
{
  Tree fileSystemManager;
  fileSystemManager = new TreeNode;
  //每次启动，先读入外存数据，再建树，再恢复双亲和前驱指针关系
  readFile();
  fileSystemManager = readSystemFromTxt(preorder, midorder, LEN);
  connectPreParent(fileSystemManager);
  //
  system("COLOR e");
  while(true)
  {
    run(fileSystemManager);
  }
}

//初始化链栈
bool initStack(LinkStack &S)
{
  S = NULL;
  return true;
}

//入栈
bool Push(LinkStack &S, Tree T)
{
  Stack *p = new Stack;
  p->data = T;
  p->next = S;
  S = p;
  return true;
}

//出栈
bool Pop(LinkStack &S, Tree &T)
{
  if(S == NULL) return false;
  Stack *p = S;
  T = S->data;
  S = S->next;
  free(p);
  return true;
}

//获得栈顶元素
bool getTop(LinkStack S, Tree &T)
{
  if(S != NULL)
  {
    T = S->data;
    return true;
  }
  else
    return false;
}

//判栈空
bool isStackEmpty(LinkStack S)
{
  if(S == NULL)
    return true;
  else
    return false;
}

//文件系统树的操作
//创建文件系统 即创建根--此电脑
void createFileSystem(Tree &T) //创建整个文件系统，即创建root目录-此电脑
{
  T = new TreeNode;
  T->fileName = "此电脑";
  T->fileType = 1;
  T->pre = NULL;
  T->parent = NULL;
  T->firstChild = NULL;
  T->nextSibling = NULL;
  T->nodeLevel = 1;
}

//用于创建孤立的结点
void createNewTreeNode(Tree &T, string name, int type, int level)
{
  T = new TreeNode;
  T->fileName = name;
  T->fileType = type;
  T->firstChild = NULL;
  T->nextSibling = NULL;
  T->pre = NULL;
  T->parent = NULL;
  T->nodeLevel = level;
}

//初始化系统目录 此目录固定  此电脑 D: E: F: //如果外存数据和windows系统目录不对应了，可以调用这个函数重新初始化文件系统
bool initFilePath(Tree &T) //创建根目录下的主要文件夹 需要根据初始文件目录的图来看
{
  if(T == NULL)
  {
    cerr<<"文件系统根目录不存在,请先建立根目录(此电脑)!"<<endl;
    return false;
  }
  //D盘
  TreeNode *D;
  createNewTreeNode(D, "D:", 1, T->nodeLevel+1);
  T->firstChild = D;
  D->pre = T;
  D->parent = T;
  //E盘
  TreeNode *E;
  createNewTreeNode(E, "E:", 1, T->nodeLevel+1);
  D->nextSibling = E;
  E->pre = D;
  E->parent = T;
  //F盘
  TreeNode *F;
  createNewTreeNode(F, "F:", 1, T->nodeLevel+1);
  E->nextSibling = F;
  F->pre = E;
  F->parent = T;
  return true;
}

//中序遍历 把当前文件系统可外存数据存到txt中
void saveFileSystem(Tree T)
{
  LinkStack S;
  initStack(S);
  TreeNode *p = T;
  TreeNode *q = new TreeNode;
  ofstream output("fileone.txt",ios::app);
  if(!output) cerr<<"文件打开失败！";
  while(p || !isStackEmpty(S))
  {
    if(p)
    {
      Push(S, p);
      p = p->firstChild;
    }
    else
    {
      Pop(S, q);
      output<<q->fileName<<" "<<q->fileType<<" "<<q->nodeLevel<<" "<<"\n";
      p = q->nextSibling;
    }
  }
  output.close();
}

//后序遍历 把当前文件系统可外存数据存到txt中
void saveFileSystemAgain(Tree T)
{
  LinkStack S;
  initStack(S);
  TreeNode *p = T;
  TreeNode *q = new TreeNode;
  ofstream output("filetwo.txt",ios::app);
  if(!output) cerr<<"文件打开失败！";
  while(p || !isStackEmpty(S))
  {
    if(p)
    {
      output<<p->fileName<<" "<<p->fileType<<" "<<p->nodeLevel<<" "<<"\n";
      Push(S, p);
      p = p->firstChild;
    }
    else
    {
      Pop(S, q);
      p = q->nextSibling;
    }
  }
  output.close();
}

//非递归遍历树  遍历次序为先序
void traverseFilePath(Tree T)
{
  LinkStack S;
  initStack(S);
  TreeNode *p = T;
  TreeNode *q = new TreeNode;
  while(p || !isStackEmpty(S))
  {
    if(p)
    {
      for(int i=1;i<=p->nodeLevel;i++) //控制层次感
        cout<<"   ";
      if(p->fileType == 1)
      	cout<<p->nodeLevel<<"->"<<p->fileName<<endl;
      else
      	cout<<p->nodeLevel<<"->"<<p->fileName<<".txt"<<endl;
      Push(S, p);
      p = p->firstChild;
    }
    else
    {
      Pop(S, q);
      p = q->nextSibling;
    }
  }
}

//找到T中 T->fileName为filename的结点
//1.如果是个文件夹，直接找到文件名 返回该文件夹指针
Tree findFileNode(Tree T, string filename)
{
  LinkStack S;
  initStack(S);
  TreeNode *p = T;
  TreeNode *q = new TreeNode;
  while(p || !isStackEmpty(S))
  {
    if(p)
    {
      if(p->fileName == filename)
        return p;
      Push(S, p);
      p = p->firstChild;
    }
    else
    {
      Pop(S, q);
      p = q->nextSibling;
    }
  }
  return NULL;
}

//2.如果是个文件，先找到文件夹指针，再找到文件，返回文件指针
Tree findFileNode(Tree T, string parentname, string filename)
{
  LinkStack S;
  initStack(S);
  TreeNode *p = T;
  TreeNode *q = new TreeNode;
  while(p || !isStackEmpty(S))
  {
    if(p)
    {
      if(p->fileName == parentname)
        break;
      Push(S, p);
      p = p->firstChild;
    }
    else
    {
      Pop(S, q);
      p = q->nextSibling;
    }
  }
  if(p == NULL) return NULL;

  if(p->firstChild->fileName == filename)
  {
    return p->firstChild;
  }
  else
  {
    p = p->firstChild;
    while(p->nextSibling != NULL)
    {
      if(p->nextSibling->fileName == filename)
        return p->nextSibling;
      p = p->nextSibling;
    }
  }
  return NULL;
}

//找到第一个nextSibling为空的结点
Tree findFirstNextEmptyRoot(Tree T)
{
  T = T->firstChild;
  while(T->nextSibling != NULL)
  {
    T = T->nextSibling;
  }
  return T;
}

//检查文件重名
bool checkSameName(Tree T, string filename, int type)
{
  bool flag = false;
  if(type == 1) //文件夹不允许重名
  {
    LinkStack S;
    initStack(S);
    TreeNode *p = T;
    TreeNode *q = new TreeNode;
    while(p || !isStackEmpty(S))
    {
      if(p)
      {
        if(p->fileType == 1 && p->fileName == filename)
        {
          flag = true;
          break;
        }
        Push(S, p);
        p = p->firstChild;
      }
      else
      {
        Pop(S, q);
        p = q->nextSibling;
      }
    }
  }
  else
  {
    TreeNode *p = T;
    p = p->firstChild;
    if(p != NULL)
    {
      if(p->fileName == filename)
        flag = true;
      while(p->nextSibling != NULL)
      {
        p = p->nextSibling;
        if(p->fileName == filename)
        {
          flag = true;
          break;
        }
      }
    }
  }
  return flag;
}

//获得名为filename的文件夹的路径
string getFilePath(Tree T, string filename)
{
  TreeNode *p = findFileNode(T, filename);
  if(p == NULL)
  {
    cout<<"文件不存在！"<<endl;
    return "NULL";
  }
  LinkStack S;
  initStack(S);
  Push(S, p);
  while(p->parent != NULL)
  {
    p = p->parent;
    Push(S,p);
  }
  string path;
  while(!isStackEmpty(S))
  {
    TreeNode *t;
    Pop(S, t);
    path += ("\\"+t->fileName);
  }
  return path;
}

//获得文件夹名为parentname下名为filename的文件的路径
string getFilePath(Tree T, string parentname,string filename)
{
  TreeNode *p = findFileNode(T, parentname,filename);
  if(p == NULL)
  {
    cout<<"文件不存在！"<<endl;
    return "NULL";
  }
  LinkStack S;
  initStack(S);
  Push(S, p);
  while(p->parent != NULL)
  {
    p = p->parent;
    Push(S,p);
  }
  string path;
  while(!isStackEmpty(S))
  {
    TreeNode *t;
    Pop(S, t);
    path += ("\\"+t->fileName);
  }
  return path;
}

//创建新文件/文件夹
bool createNewFile(Tree &T, string filename, string newfilename, int type) // 参数分别为 文件系统  要创立的文件所在的文件夹(传入的也是个名字，如果这个文件不是文件夹，则不能创建文件，返回false) 要创立文件的名字 要创立文件的类型
{
  TreeNode *p = findFileNode(T, filename);
  if(type == 1 && checkSameName(T, newfilename, type))
  {
      cout<<"Warning:"<<endl;
      cout<<"文件系统中已存在名为 "<<newfilename<<" 的文件夹!"<<endl;
      cout<<endl;
      return false;
  }
  else if(type == 0 && checkSameName(p, newfilename, type))
  {
      cout<<"Warning:"<<endl;
      cout<<"文件夹中已存在名为 "<<newfilename<<" 的文件!"<<endl;
      cout<<endl;
      return false;
  }
  if(p == NULL)
  {
    cout<<"文件夹不存在！"<<endl;
    return false;
  }
  else if(p->fileType == 0)
  {
    cout<<"你不能在一个文件下创建文件，请选择一个文件夹!"<<endl;
    return false;
  }
  int level = p->nodeLevel+1;
  if(p->firstChild == NULL)
  {
    createNewTreeNode(p->firstChild, newfilename, type, level);
    p->firstChild->pre = p;
    p->firstChild->parent = p;
  }
  else
  {
    TreeNode *q = findFirstNextEmptyRoot(p);
    createNewTreeNode(q->nextSibling, newfilename, type, level);
    q->nextSibling->pre = q;
    q->nextSibling->parent = p;
  }
  string path = getFilePath(T, filename,newfilename);
  string cdcmd = path.substr(8, 1)+":";
  system(cdcmd.c_str());
  if(type == 1) //创建文件夹
  {
    string cmd = "mkdir "+path.substr(8, path.length()-8);
    //cout<<"命令测试: "<<cmd<<endl;
    system(cmd.c_str());
  }
  else if(type == 0) //创建文本文件
  {
    string cmd = "type nul>"+path.substr(8, path.length()-8)+".txt"; //这里把文件类型写死，TXT就代表所有除文件夹外的文件
    //cout<<"命令测试: "<<cmd<<endl;
    system(cmd.c_str());
  }
  return true;
}

//删除文件
bool deleteFile(Tree &T,string parentname, string defilename)
{
  TreeNode *p = findFileNode(T, parentname,defilename);
  if(p == NULL)
  {
    cout<<"要删除的文件不存在!"<<endl;
    return false;
  }
  string path = getFilePath(T, parentname,defilename);
  string cdcmd = path.substr(8, 1)+":";
  system(cdcmd.c_str());
  string cmd = "del "+path.substr(8, path.length()-8)+".txt";
  //cout<<"命令测试: "<<cmd<<endl;
  system(cmd.c_str());
  if(p->pre->firstChild == p)
  {
    p->pre->firstChild = p->nextSibling;
    if(p->nextSibling != NULL)
      p->nextSibling->pre = p->pre;
    delete(p);
  }
  else
  {
    p->pre->nextSibling = p->nextSibling;
    if(p->nextSibling != NULL)
      p->nextSibling->pre = p->pre;
    delete(p);
  }
  return true;
}


//删除文件夹
bool deleteFile(Tree &T, string defilename) //文件系统      删除的文件夹名
{
  if(defilename == "D:" || defilename == "E:" || defilename == "F")
  {
    cout<<"禁止删除的文件夹!!"<<endl;
    return false;
  }
  TreeNode *p = findFileNode(T, defilename);
  if(p == NULL)
  {
    cout<<"要删除的文件不存在!"<<endl;
    return false;
  }

  string ensure;
  cout<<"你选择删除的是一个文件夹，你确定要这么做吗？如果这么做的话，该文件夹下的文件和文件夹将都被删除!"<<endl;
  cout<<"如果你依然要这么做，请输入yes，如果你要放弃操作，请输入no:";
  cin>>ensure;
  if(ensure == "no") return false;

  string path = getFilePath(T, defilename);
  string cdcmd = path.substr(8, 1)+":";
  system(cdcmd.c_str());
  LinkStack S;
  initStack(S);
  Tree rootDel = p;
  Tree tmp;
  if(p->firstChild != NULL)
  {
    Push(S, p->firstChild);
    p = p->firstChild;
    while(p->nextSibling != NULL)
    {
      Push(S, p->nextSibling);
      p = p->nextSibling;
    }
  }
  while(!isStackEmpty(S))
  {
    Pop(S, tmp);
    if(tmp->fileType == 1)
      deleteFile(T, tmp->fileName);
    else
      deleteFile(T, rootDel->fileName,tmp->fileName);
  }
  if(rootDel->pre->firstChild == rootDel)
  {
    rootDel->pre->firstChild = rootDel->nextSibling;
    if(rootDel->nextSibling != NULL)
      rootDel->nextSibling->pre = rootDel->pre;
    delete(rootDel);
  }
  else //tmp->pre->nextSibling == p
  {
    rootDel->pre->nextSibling = rootDel->nextSibling;
    if(rootDel->nextSibling !=NULL)
      rootDel->nextSibling->pre = rootDel->pre;
    delete(rootDel);
  }
  string cmd = "rmdir /s "+path.substr(8, path.length()-8);
  system(cmd.c_str());
  //cout<<"命令测试: "<<cmd<<endl;
  return true;
}

//重命名文件夹
bool reNameFile(Tree &T, string filename, string newfilename)
{
  TreeNode *p = findFileNode(T, filename);
  if(checkSameName(T, newfilename, 1))
  {
    cout<<"Warning:"<<endl;
    cout<<"文件系统中已经存在名为 "<<newfilename<<" 的文件夹"<<endl;
    cout<<endl;
    return false;
  }
  if(p == NULL)
  {
    cout<<"要重命名的文件不存在!"<<endl;
    return false;
  }
  string path = getFilePath(T, filename);
  string cdcmd = path.substr(8, 1)+":";
  system(cdcmd.c_str());

  string cmd = "rename "+path.substr(8, path.length()-8)+" "+newfilename;
  //cout<<"命令测试: "<<cmd<<endl;
  system(cmd.c_str());
  p->fileName = newfilename;
  return true;
}

//重命名文件
bool reNameFile(Tree &T, string parentname,string filename, string newfilename)
{
  TreeNode *p = findFileNode(T, parentname, filename);
  if(checkSameName(p, newfilename, 0))
  {
    cout<<"Warning:"<<endl;
    cout<<"文件夹中已经存在名为 "<<newfilename<<" 的文件"<<endl;
    cout<<endl;
    return false;
  }
  if(p == NULL)
  {
    cout<<"要重命名的文件不存在!"<<endl;
    return false;
  }
  string path = getFilePath(T, parentname,filename);
  string cdcmd = path.substr(8, 1)+":";
  system(cdcmd.c_str());

  string cmd = "rename "+path.substr(8, path.length()-8)+".txt"+" "+newfilename+".txt";
  //cout<<"命令测试: "<<cmd<<endl;
  system(cmd.c_str());
  p->fileName = newfilename;
  return true;
}

//列出名为filename的文件夹下的所有文件/文件夹
bool lsFileChild(Tree T, string filename)
{
  TreeNode *p = findFileNode(T, filename);
  if(p == NULL)
  {
    cout<<"文件夹不存在!"<<endl;
    return false;
  }
  else if(p->firstChild == NULL)
  {
    cout<<"该文件夹是空的！"<<endl;
    return true;
  }
  cout<<p->fileName<<"中的文件有:"<<endl;
  p = p->firstChild;
  cout<<"  -->"<<p->fileName<<endl;
  while(p->nextSibling != NULL)
  {
    p = p->nextSibling;
    cout<<"  -->"<<p->fileName<<endl;
  }
  return true;
}

//移动文件
bool moveFile(Tree &T, string mvparentname, string mvfilename, string tofilename)
{
  TreeNode *p = findFileNode(T, mvparentname,mvfilename);
  TreeNode *q = findFileNode(T, tofilename);
  if(checkSameName(q, mvfilename, 0))
  {
    cout<<"Warning:"<<endl;
    cout<<"要移动到的文件夹中已经存在名为 "<<mvfilename<<" 的文件夹"<<endl;
    cout<<endl;
    return false;
  }
  if(p == NULL || q == NULL)
  {
    cout<<"被移动文件夹或文件或移动到的文件夹不存在!"<<endl;
    return false;
  }

  //只能移动文件
  if(p->fileType == 0)
  {
    string tmpfilename = q->fileName;
    string tmpnewfilename = p->fileName;
    deleteFile(T, mvparentname,p->fileName);
    createNewFile(T, tmpfilename, tmpnewfilename, 0);
  }
  else
    cout<<"不支持移动文件夹!"<<endl;
  return true;
}

//复制文件
bool copyFile(Tree &T, string parentname, string cpfilename, string tofilename)
{
  TreeNode *p = findFileNode(T, parentname, cpfilename);
  TreeNode *q = findFileNode(T, tofilename);
  if(checkSameName(q, cpfilename, 0))
  {
    cout<<"Warning:"<<endl;
    cout<<"要移动到的文件夹中已经存在名为 "<<cpfilename<<" 的文件夹"<<endl;
    cout<<endl;
    return false;
  }
  if(p == NULL || q == NULL)
  {
    cout<<"所要复制的文件或文件夹或要移动到的文件夹不存在!"<<endl;
    return false;
  }

  //只能复制文件
  if(p->fileType == 0)
  {
    createNewFile(T, q->fileName, p->fileName, p->fileType);
  }
  else
    cout<<"不支持复制文件夹!"<<endl;
  return true;
}

//从外存中读入文件系统树数据
void readFile()
{
  preorder = new MainData[100];
  midorder = new MainData[100];
  ifstream infile("fileone.txt");
  if(!infile) cerr<<"文件打开失败！";
  int k = 0;
  while(!infile.eof())
  {
    infile>>midorder[k].filename>>midorder[k].fileType>>midorder[k].nodeLevel;
    ++k;
  }
  infile.close();
  ifstream input("filetwo.txt");
  if(!input) cerr<<"文件打开失败！";
  int j = 0;
  while(!input.eof())
  {
    input>>preorder[j].filename>>preorder[j].fileType>>preorder[j].nodeLevel;
    ++j;
  }
  input.close();
  LEN = k-1;
}

//利用读入的数据重建树
Tree readSystemFromTxt(MainData *preorder, MainData *midorder, int len)
{
  MainData rootKey = preorder[0];
  Tree root = new TreeNode;
  root->fileName = rootKey.filename;
  root->fileType = rootKey.fileType;
  root->nodeLevel = rootKey.nodeLevel;
  root->firstChild = NULL;
  root->nextSibling = NULL;
  root->parent = NULL;
  root->pre = NULL;
  if(len == 1 && preorder->filename == midorder->filename)
    return root;

  MainData *rootMidOrder = midorder;
  int leftLen = 0;
  while(rootMidOrder->filename != rootKey.filename && rootMidOrder <= (midorder+len-1))
  {
    ++rootMidOrder;
    ++leftLen;
  }
  //if(rootMidOrder->filename != rootKey.filename ) return NULL; //error

  if(leftLen > 0)
  {
    root->firstChild = readSystemFromTxt(preorder+1, midorder, leftLen);
  }
  if(len-leftLen-1 >0)
  {
    root->nextSibling = readSystemFromTxt(preorder+leftLen+1, rootMidOrder+1, len-leftLen-1);
  }
  return root;
}

//将重建后的树的双亲和前驱还原
void connectPreParent(Tree &T)
{
  LinkStack S;
  initStack(S);
  TreeNode *par;
  TreeNode *now;
  TreeNode *prenow;
  TreeNode *p = T;
  TreeNode *q = new TreeNode;
  while(p || !isStackEmpty(S))
  {
    if(p)
    {

      Push(S, p);
      par = p;
      if(p->firstChild != NULL)
      {
        p->firstChild->parent = par;
        p->firstChild->pre = p;
        now = p->firstChild;
        while(now->nextSibling != NULL)
        {
          prenow = now;
          now = now->nextSibling;
          now->parent = par;
          now->pre = prenow;
        }
      }
      p = p->firstChild;
    }
    else
    {
      Pop(S, q);
      p = q->nextSibling;
    }
  }
}

void checkLocalWindows()
{
  string cmd;
  getline(cin, cmd);
  while(cmd != "exit")
  {
    cout<<"请输入命令(Command):"<<endl;
    getline(cin, cmd);
    system(cmd.c_str());
  }
}

void menu()
{
  cout<<"              |                             |                "<<endl;
  cout<<"               模拟资源管理器Resource Manager                 "<<endl;
  cout<<"              |                             |                "<<endl;
  cout<<"|                  {}可执行命令列表{}                        "<<endl;
  cout<<"|          []常用命令-.-                                     "<<endl;
  cout<<"|                  check->查看文件系统                       "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  new->创建新文件                           "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  delete->删除文件                          "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  cp->复制文件到另一个文件夹                 "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  mv->移动文件到另一个文件夹                 "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  path->获取文件路径                        "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  ls->查看文件夹下的所有文件                  "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  rename->重命名文件                         "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  cmd->进入命令行                            "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|          ()备用命令0.0                                      "<<endl;
  cout<<"|                  init->初始化文件系统                       "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  create->建立文件系统                      "<<endl;
  cout<<"|                                                            "<<endl;
  cout<<"|                  -1->退出系统                               "<<endl;
  cout<<"                                @windows =>命令输入区:";
}

void run(Tree &T)
{
  menu();
  string cmd;
  cin>>cmd;
  if(cmd == "create")
  {
    system("CLS");
    cout<<"正在建立文件系统"<<endl;
    Sleep(1000);
    createFileSystem(T);
    cout<<"执行完毕！"<<endl;
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "init")
  {
    system("CLS");
    cout<<"正在初始化文件系统"<<endl;
    Sleep(1000);
    initFilePath(T);
    cout<<"执行完毕！初始化后的文件系统:"<<endl;
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "cmd")
  {
    system("CLS");
    checkLocalWindows();
    cout<<"执行完毕！"<<endl;
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "new")
  {
    system("CLS");
    cout<<"正在执行创建文件操作"<<endl;
    cout<<"请输入存放创建文件的文件夹名及要创建的文件名字和类型(1为文件夹 0为文件):";
    int type;
    string filename, newfilename;
    cout<<"请输入类型:";
    cin>>type;
    cout<<"请输入源文件夹名及新建文件名:";
    cin>>filename>>newfilename;
    createNewFile(T, filename, newfilename, type);
    cout<<"执行完毕！创建新文件后的文件系统:"<<endl;
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "delete")
  {
    system("CLS");
    cout<<"正在执行删除文件操作"<<endl;
    cout<<"请输入要删除的文件或文件夹的名:";
    int type;
    cout<<"请输入删除文件的类型:";
    cin>>type;
    if(type == 1)
    {
      string defilename;
      cout<<"请输入要删除的文件夹名:";
      cin>>defilename;
      deleteFile(T, defilename);
    }
    else
    {
      string defilename, delparname;
      cout<<"请输入要删除文件所在的文件夹及要删除的文件名:";
      cin>>delparname>>defilename;
      deleteFile(T, delparname, defilename);
    }
    cout<<"执行完毕！删除文件后的文件系统:"<<endl;
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "path")
  {
    system("CLS");
    cout<<"正在执行获取文件路径操作"<<endl;
    int type;
    cout<<"请输入文件类型:";
    cin>>type;
    if(type == 1)
    {
      cout<<"请输入要获取路径的文件夹名:";
      string filename;
      cin>>filename;
      string path = getFilePath(T, filename);
      cout<<"文件夹"<<filename<<"的路径为:";
      cout<<path<<endl;
    }
    else
    {
      cout<<"请输入要获取路径的文件所在的文件夹名及文件名:";
      string parname,filename;
      cin>>parname>>filename;
      string path = getFilePath(T, parname,filename);
      cout<<"文件"<<filename<<"的路径为:";
      cout<<path<<endl;
    }
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "ls")
  {
    system("CLS");
    cout<<"正在执行查看文件夹下的所有文件操作"<<endl;
    cout<<"请输入要查看的文件夹名:";
    string filename;
    cin>>filename;
    cout<<"文件 "<<filename<<" 下的文件有:"<<endl;
    lsFileChild(T, filename);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "rename")
  {
    system("CLS");
    cout<<"正在执行文件重命名操作"<<endl;
    int type;
    cout<<"请输入要重命名文件的类型:";
    cin>>type;
    if(type == 1)
    {
      string filename, newfilename;
      cout<<"请输入文件夹原名及新的文件夹名:";
      cin>>filename>>newfilename;
      reNameFile(T, filename, newfilename);
    }
    else
    {
      string parname, filename, newfilename;
      cout<<"请输入文件所在的文件夹名,文件原名及新的文件名:";
      cin>>parname>>filename>>newfilename;
      reNameFile(T, parname, filename, newfilename);
    }
    cout<<"执行完毕！文件重命名后的文件系统:"<<endl;
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "mv")
  {
    system("CLS");
    cout<<"正在执行文件移动操作"<<endl;
    cout<<"请输入起始文件所在的文件夹名以及文件名 末文件夹名:";
    string parname, filename, newfilename;
    cin>>parname>>filename>>newfilename;
    moveFile(T, parname,filename, newfilename);
    cout<<"执行完毕！文件移动后的文件系统:"<<endl;
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "cp")
  {
    system("CLS");
    cout<<"正在执行文件复制操作"<<endl;
    cout<<"请输入起始文件所在的文件夹名以及文件名 末文件夹名:";
    string parname, filename, tofilename;
    cin>>parname>>filename>>tofilename;
    copyFile(T, parname, filename, tofilename);
    cout<<"执行完毕！文件移动后的文件系统:"<<endl;
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "check")
  {
    system("CLS");
    traverseFilePath(T);
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
  else if(cmd == "-1")
  {
    fstream output1("fileone.txt", ios::out|ios::trunc);
    if(!output1)
    {
      cerr<<"文件打开失败!"<<endl;
      exit(0);
    }
    fstream output2("filetwo.txt", ios::out|ios::trunc);
    if(!output2)
    {
      cerr<<"文件打开失败!"<<endl;
      exit(0);
    }
    output1.close();
    output2.close();
    saveFileSystem(T);
    saveFileSystemAgain(T);
    cout<<"欢迎下次使用~"<<endl;
    exit(1);
  }
  else
  {
    system("CLS");
    cout<<"命令错误，请检查命令格式或拼写!"<<endl;
    cout<<"输入任意字符回车返回上层:";
    string s;
    cin>>s;
    system("CLS");
  }
}
