#include <io.h>
#include <iostream>
#include <cstring>
using namespace std;

typedef struct Node {
    struct Node *child;//第一个子节点
    struct Node *brother;//第二、三、四...节点
    string name;
    long long size;
} Node, *Tree;

void GetFiles(string path, Tree file) {
    long hFile = 0;              //文件句柄
    struct _finddata_t fileinfo; //文件信息结构体
    string p;                     //字符串，存放路径
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
        Tree last;
        do {
            if ((fileinfo.attrib & _A_SUBDIR)) { //如果是目录
                //(.表示当前目录 ..表示当前目录的父目录)忽略这两个
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
                    Node *newNode = new Node;                                          //新建节点
                    newNode->size = 0;                                                 //初始化节点的大小
                    newNode->name = p.assign(path).append("\\").append(fileinfo.name); //初始化节点的路径名
                    newNode->child=newNode->brother=NULL;                              //初始化此节点的第一个孩子节点和此节点的兄弟节点
                    if (file->child ==NULL) {                                          //当父节点的第一个孩子节点为空时，表示这个节点是父节点的第一个孩子节点
                        file->child = newNode;                                         //此节点为父节点的第一个孩子节点
                        last = newNode;                                                //此节点存为上一次处理的节点
                    } else {                                                           //父节点的第一个孩子节点不为空，即此节点和上一个处理的节点为兄弟节点，此节点存为上一个处理的节点的兄弟节点
                        last->brother = newNode;                                       //把此节点存为上一个处理的节点的兄弟节点
                        last = newNode;                                                //把此节点存为上一个处理的节点
                    }
                    GetFiles(p.assign(path).append("\\").append(fileinfo.name), newNode); //遍历下一层文件夹和文件
                    file->size += newNode->size;                                           //加上path子文件夹(即此文件夹)的大小
                }
            } else {                         //如果是文件
                file->size += fileinfo.size; //加上path文件夹下文件的大小
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

int main() {
    string file_path = "D:";

    Tree file = new Node;
    file->child = NULL;
    file->brother = NULL;
    file->name = file_path;
    file->size = 0;

    GetFiles(file_path, file);

    Tree temp_node=file;
    cout << temp_node->name << " " << temp_node->size << endl;
    temp_node=temp_node->child;
    cout << temp_node->name << " " << temp_node->size << endl;
    while(temp_node->brother){
        temp_node=temp_node->brother;
        cout << temp_node->name << " " << temp_node->size << endl;
    }
    return 0;
}
