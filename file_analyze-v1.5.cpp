/* 乱码解决办法
 * 需要设置编码为GBK,否则中文输出乱码
 * 设置GBK编码后,输出正常,但调试时查看变量是还是乱码,不过不影响,程序正确运行
 * is_find,is_find_child待解决
 * 输入D:后有BUG
 * */
#include <iostream>
#include <io.h>
#include <time.h>
#include <cstring>
#include <stdio.h>
using namespace std;

//存储文件夹下的文件的链表
typedef struct File_Node{
    struct File_Node *front;
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
//此函数自动对文件夹树下的file->next赋值NULL
void GetFiles(string path, Tree file) {
    long hFile = 0;              //文件句柄
    struct _finddata_t fileinfo; //文件信息结构体
    string p;                    //字符串，存放路径
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
        Tree last;
        file->file=new File_Node; //未初始化的file->file在使用file->file->next时出错
        file->file->next=NULL;    //初始化后的file->file->next不为空
        do {
            if ((fileinfo.attrib & _A_SUBDIR)) { //如果是目录
                //(.表示当前目录 ..表示当前目录的父目录)忽略这两个
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
                    Node *newNode = new Node;                                          //新建节点
                    newNode->size = 0;                                                 //初始化节点的大小
                    newNode->name=fileinfo.name;                                       //初始化节点的路径名
                    if(newNode->name=="System Volume Information")
                        continue;
                    newNode->child = newNode->brother = NULL;                          //初始化此节点的第一个孩子节点和此节点的兄弟节点
                    if (file->child == NULL) {                                         //当父节点的第一个孩子节点为空时，表示这个节点是父节点的第一个孩子节点
                        file->child = newNode;                                         //此节点为父节点的第一个孩子节点
                        last = newNode;                                                //此节点存为上一次处理的节点
                    } else {                                                           //父节点的第一个孩子节点不为空，即此节点和上一个处理的节点为兄弟节点，此节点存为上一个处理的节点的兄弟节点
                        last->brother = newNode;                                       //把此节点存为上一个处理的节点的兄弟节点
                        last = newNode;                                                //把此节点存为上一个处理的节点
                    }
                    GetFiles(p.assign(path).append("\\").append(fileinfo.name), newNode); //遍历下一层文件夹和文件(p.assign(path).append("\\").append(fileinfo.name)是绝对路径)
                    file->size += newNode->size;                                          //加上path子文件夹(即此文件夹)的大小
                }
            } else {                         //如果是文件
                file->size += fileinfo.size; //加上path文件夹下文件的大小

                File_Node *newFile=new File_Node;
                newFile->name=fileinfo.name;
                newFile->size=fileinfo.size;

                File file_temp = file->file;
                while (file_temp->next) {
                    if (file_temp->next == NULL || newFile->size >= file_temp->next->size)
                        break;
                    file_temp = file_temp->next;
                }
                newFile->next = file_temp->next;
                file_temp->next = newFile;
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
    if(file==NULL) //path="D:123\\435"时满足,但是p!=-1,为避免执行下一层遍历,必需放这里
        return NULL;
    if(p!=-1)
        return Find_Dir_Addr(file->child,path.substr(p+1));
    else if(file->name==path_dir){
        is_find=true;
        return file;
    }
}

void Display_Result(Tree tree,long long size_max,long long size_min){
    Tree tree_temp = tree->child;
    cout<<"max="<<size_max<<" min="<<size_min<<endl<<endl;
    printf("文件名                             大小         占比      占比状态\n");
    while (tree_temp) {
        printf("[文件夹] %-25s %-12lld %-6.2lf%%",tree_temp->name.c_str(),tree_temp->size,tree->size==0?0.00:((tree_temp->size+0.0)/tree->size*100));
        int i,j=tree_temp->size*100/tree->size;
        printf("   ");
        for(i=0;i<j;i++)
            printf("-");
        printf("\n");
        tree_temp = tree_temp->brother;
    }
    while (tree->file) {
        printf("%-25s %-12lld %.2lf%%\n",tree->file->name.c_str(),tree->file->size,tree->size==0?0.00:((tree->file->size+0.0)/tree->size*100));
        tree->file = tree->file->front;
    }
    cout<<endl;
}

//对文件夹和文件进行排序
void Sort_By_Size(Tree tree){
    /*待加入计算文件夹和文件的数量*/
    long long size_max=0,size_min=tree->size;
    //对文件夹的大小进行排序
    Tree tree_temp=tree->child;
    while(tree_temp){
        Tree tree_temp2=tree_temp->brother;
        while(tree_temp2){
            if(tree_temp2->size>tree_temp->size){
                //交换除brother以外的全部成员
                Tree child_temp=tree_temp2->child;
                tree_temp2->child=tree_temp->child;
                tree_temp->child=child_temp;
                File file_temp=tree_temp2->file;
                tree_temp2->file=tree_temp->file;
                tree_temp->file=file_temp;
                long long size_temp=tree_temp2->size;
                tree_temp2->size=tree_temp->size;
                tree_temp->size=size_temp;
                string name_temp=tree_temp2->name;
                tree_temp2->name=tree_temp->name;
                tree_temp->name=name_temp;
            }
            tree_temp2=tree_temp2->brother;
        }
        if(tree_temp->size>size_max)
            size_max=tree_temp->size;
        if(tree_temp->size<size_min)
            size_min=tree_temp->size;
        tree_temp=tree_temp->brother;
    }
    //对文件夹内的文件的大小进行排序
    File file_temp=tree->file;
    while(file_temp){
        File file_temp2=file_temp->front;
        while(file_temp2){
            if(file_temp2->size>file_temp->size){
                long long size_temp=file_temp2->size;
                file_temp2->size=file_temp->size;
                file_temp->size=size_temp;
                string name_temp=file_temp2->name;
                file_temp2->name=file_temp->name;
                file_temp->name=name_temp;
            }
            file_temp2=file_temp2->front;
        }
        if(file_temp->size>size_max)
            size_max=file_temp->size;
        if(file_temp->size<size_min)
            size_min=file_temp->size;
        file_temp=file_temp->front;
    }
    Display_Result(tree,size_max,size_min);
}

void Display_Result2(Tree tree){
    Tree tree_temp = tree->child;
    printf("文件名                             大小         占比      占比状态\n");
    while (tree_temp) {
        printf("[文件夹] %-25s %-12lld %-6.2lf%%",tree_temp->name.c_str(),tree_temp->size,tree->size==0?0.00:((tree_temp->size+0.0)/tree->size*100));
        int i,j=tree_temp->size*100/tree->size;
        printf("   ");
        for(i=0;i<j;i++)
            printf("-");
        printf("\n");
        tree_temp = tree_temp->brother;
    }
    tree->file=tree->file->next;
    while (tree->file) {
        printf("%-25s %-12lld %.2lf%%\n",tree->file->name.c_str(),tree->file->size,tree->size==0?0.00:((tree->file->size+0.0)/tree->size*100));
        tree->file = tree->file->next;
    }
    cout<<endl;
}

int main() {
    string file_path = "D:";

    Tree file = new Node;
    file->child = file->brother = NULL;
    file->name = file_path;
    file->size = 0;

    cout<<"开始运行..."<<endl;
    int start_time=clock();
    GetFiles(file_path, file);
    int end_time=clock();
    cout << "完成搜索( " <<end_time-start_time<<"ms )"<< endl;

    /*测试 输入:一个路径 输出:路径内文件夹的大小*/
    string path_input;
    while (cin >> path_input && path_input != "exit") {
        Tree temp_node = file;
        temp_node=Find_Dir_Addr(temp_node, path_input);
        if(temp_node!=NULL)
            Display_Result2(temp_node);
        else
            cout<<"未找到"<<endl;
    }
    return 0;
}
