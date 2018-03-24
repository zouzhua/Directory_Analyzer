/* 乱码解决办法
 * 需要设置编码为GBK,否则中文输出乱码
 * 设置GBK编码后,输出正常,但调试时查看变量是还是乱码,不过不影响,程序正确运行
 * is_find,is_find_child待解决
 * */
#include <iostream>
#include <io.h>
#include <time.h>
#include <cstring>
#include <stdio.h>
using namespace std;

//存储文件夹下的文件的链表
typedef struct File_Node{
    struct File_Node *next;
    string name;
    long long size;
}File_Node,*File;

//存储文件夹大小的树
typedef struct Node {
    struct Node *child;   //第一个子节点
    struct Node *brother; //第二、三、四...节点
    struct File_Node *file;
    string name;
    long long size;
} Node, *Tree;

//is_find:是否找到该路径   is_find_child:是否找到该路径下的子目录
bool is_find = false, is_find_child = false;

//遍历文件夹(DFS)
void Get_File_Tree(string path, Tree file) {
    long hFile = 0;              //文件句柄
    struct _finddata_t fileinfo; //文件信息结构体
    string p;                    //字符串，存放路径
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
        Tree last;
        file->file=NULL;//防止在只有文件(不含有子文件夹)的文件在进行文件大小排序时指针错误
        do {
            if ((fileinfo.attrib & _A_SUBDIR)) { //如果是目录
                //(.表示当前目录 ..表示当前目录的父目录)忽略这两个
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
                    /*新建和初始化节点*/
                    Node *newNode = new Node;
                    newNode->size = 0;
                    newNode->name=fileinfo.name;
                    newNode->child = newNode->brother = NULL;
                    /*跳过不遍历的文件夹*/
                    if(newNode->name=="System Volume Information")
                        continue;
                    /*新增节点*/
                    file->child==NULL?file->child=newNode:last->brother=newNode;
                    last=newNode;
                    /*遍历下一层文件夹和文件(p.assign(path).append("\\").append(fileinfo.name)是绝对路径)*/
                    Get_File_Tree(p.assign(path).append("\\").append(fileinfo.name), newNode);
                    /*加上path子文件夹(即此文件夹)的大小*/
                    file->size += newNode->size;
                }
            } else {
                /*如果是文件,加上path文件夹下文件的大小*/
                file->size += fileinfo.size;
                /*新建和初始化节点*/
                File_Node *file_temp=new File_Node;
                file_temp->name=fileinfo.name;
                file_temp->size=fileinfo.size;
                /*新增节点*/
                file_temp->next=file->file;
                file->file=file_temp;
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

//查找输入路径下子目录大小
Tree Find_Dir_Addr(Tree file, string path){
    int p;
    string path_dir=path.substr(0,(p=path.find('\\')));
    while(file){ //正常跳出循环条件:1.找到匹配的(file->name=path_dir) 2.file为空
        if(file==NULL)
            return NULL;
        if(file->name==path_dir)
            break;
        file=file->brother;
    }
    /*path="D:123\\435"时满足,但是p!=-1,为避免执行下一层遍历,必需放这里*/
    if(file==NULL)
        return NULL;
    if(p!=-1)
        return Find_Dir_Addr(file->child,path.substr(p+1));
    else if(file->name==path_dir){
        is_find=true;
        return file;
    }
}

//对文件夹和文件进行排序
void Sort_By_Size(Tree tree) {
    /*待加入计算文件夹和文件的数量*/
    /*对文件夹的大小进行排序(冒泡排序)*/
    if (tree->child) {
        Tree tree_temp = new Node, tree_init = tree_temp, sign = new Node;
        tree_temp->brother = tree->child;
        while (tree_temp->brother->brother && tree_temp->brother->brother != sign) {
            while (tree_temp->brother->brother && tree_temp->brother->brother != sign) {
                if (tree_temp->brother->size < tree_temp->brother->brother->size) {
                    Node *node_temp = tree_temp->brother->brother;
                    tree_temp->brother->brother = node_temp->brother;
                    node_temp->brother = tree_temp->brother;
                    tree_temp->brother = node_temp;
                }
                tree_temp = tree_temp->brother;
            }
            sign = tree_temp->brother;
            tree_temp = tree_init;
        }
        tree->child = tree_init->brother;
    }
    /*对文件夹内的文件的大小进行排序(冒泡排序)*/
    if (tree->file) {
        File file_temp = new File_Node, file_init = file_temp, sign = new File_Node;
        file_temp->next = tree->file;
        while (file_temp->next->next && file_temp->next->next != sign) {
            while (file_temp->next->next && file_temp->next->next != sign) {
                if (file_temp->next->size < file_temp->next->next->size) {
                    File_Node *node_temp = file_temp->next->next;
                    file_temp->next->next = node_temp->next;
                    node_temp->next = file_temp->next;
                    file_temp->next = node_temp;
                }
                file_temp = file_temp->next;
            }
            sign = file_temp->next;
            file_temp = file_init;
        }
        tree->file = file_init->next;
    }
}

void Display_Result(Tree tree, string path_now) {
    Sort_By_Size(tree);
    Tree tree_temp = tree->child;
    cout << "[当前路径] " << path_now << endl;
    printf("文件名                             大小         占比      占比状态\n");
    while (tree_temp) {
        printf("[文件夹] %-25s %-12lld %-6.2lf%%", tree_temp->name.c_str(), tree_temp->size,
               tree->size == 0 ? 0.00 : ((tree_temp->size + 0.0) / tree->size * 100));
        int i, j = tree_temp->size * 100 / tree->size;
        printf("   ");
        for (i = 0; i < j; i++)
            printf("-");
        printf("\n");
        tree_temp = tree_temp->brother;
    }
    while (tree->file) {
        printf("%-34s %-12lld %.2lf%%\n", tree->file->name.c_str(), tree->file->size,
               tree->size == 0 ? 0.00 : ((tree->file->size + 0.0) / tree->size * 100));
        tree->file = tree->file->next;
    }
    cout << endl;
}

void Delete_File(Tree tree,string path){

}

int main() {
    string file_path = "D:";

    Tree file = new Node;
    file->child = file->brother = NULL;
    file->name = file_path;
    file->size = 0;

    cout<<"开始运行..."<<endl;
    int start_time=clock();
    Get_File_Tree(file_path, file);
    int end_time=clock();
    cout << "完成搜索( " <<end_time-start_time<<"ms )"<< endl;

    /*测试 输入:一个路径 输出:路径内文件夹的大小*/
    string path_input = "", path_log;
    while (cin >> path_input && path_input != "exit") {
        if (path_input[0] == '.' && path_input[1] == '/')
            path_input = path_log + "\\" + path_input.substr(2);
        Tree temp_node = file;
        temp_node = Find_Dir_Addr(temp_node, path_input);
        if (temp_node != NULL) {
            Display_Result(temp_node, path_input);
            path_log = path_input;
        }
        else
            cout << "未找到" << endl;
    }
    return 0;
}
