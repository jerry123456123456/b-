#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define M 3 // B树的阶数/2+1

typedef struct btree_node {
    int num;
    int leaf;   //叶子结点是1
  	int keys[2*M-1]; 
  	struct btree_node *childrens[2*M];
} btree_node;

typedef struct b_tree {
  	struct btree_node *root;
  	int t;    //由这个关键字*2-1可以得到这个b树的阶数
} b_tree;

// 创建一个空的B树
b_tree* create_b_tree() {
    b_tree* tree = (b_tree*)malloc(sizeof(b_tree));
    tree->root = NULL;
    tree->t = M;
    return tree;
}

// 在B树中查找关键字key所在节点以及索引位置
void search_key(btree_node* x, int key, int* index) {    //第一个参数时传入的根节点，第二个参数传入的是查找的元素，最后一个是引用方式传递的结果作为传出参数
  int i=0;

	while(i < x->num && key > x->keys[i]){
		i++;
	}

	if(i < x->num && key == x->keys[i]){
		*index = i; // 找到了关键字key在节点x中的索引位置
    	return;
	}else if(x->leaf){
    	*index = -1; // 关键字key不在B树中
    	return;
  	}else{
  		search_key(x->childrens[i], key, index); // 继续在子节点中查找关键字key
  		return ;
  	}
}


// 分裂满节点y的子节点x（y必须是非叶子节点）
void split_child(btree_node* y, int i, btree_node* x) {
	btree_node *z=(btree_node *)malloc(sizeof(btree_node)); 
	z->leaf=x->leaf;
	z -> num=M-1; 

	for(int j=0;j<M-1;j++){ 
   	    z -> keys[j]=x -> keys[j+M]; 
    }
   
	if(!x -> leaf){  
  	  for(int j=0;j<M;j++){  
    		z -> childrens[j]=x -> childrens[j+M];
  		}
	}

	x -> num=M-1; 

	for(int j=y->num;j>i;j--){
    	y -> childrens[j+1]=y -> childrens[j];
    }
  
  	y -> childrens[i+1]=z;
  
  	for(int j=y->num-1;j>=i;j--){ 
    	y->keys[j+1]=y->keys[j]; 
    } 
  	
	y->keys[i]=x->keys[M-1];
  	
	y->num++;
}


// 插入关键字key到B树的非满节点x中
void insert_nonfull(btree_node* x, int key) {
	int i=x -> num - 1;
	
	if(x -> leaf){ 
	 	while(i >=0 && key < x -> keys[i]){
   			x->keys[i+1] = x->keys[i];
   			i--;
  		}	
  		x->keys[i+1] = key;
	  	x -> num++;
	}else{
    	while(i >=0 && key < x -> keys[i]){
    		i--;
   		}
   		i++;
   		
  	  if(x -> childrens[i]->num ==2*M-1 ){ 
     		split_child(x,i,x -> childrens[i]);
     			
     		if(key > x -> keys[i]){
    			i++;   
			}
			
      	insert_nonfull(x->childrens[i],key);
	  }else{ 
   		insert_nonfull(x -> childrens[i], key);  
	  }
	 
	}
}

// 插入关键字key到B树中
void insert_b_tree(b_tree *T,int key){

	btree_node *r=T->root;
	
	if(r==NULL){ 	
		r=(btree_node *)malloc(sizeof(btree_node));
		r -> num = 1;
		r -> leaf = 1;
   		r-> keys[0] = key;
    
    	T -> root = r;
	}else{
  		if(r->num ==2*M-1){ 
   			btree_node *s=(btree_node *)malloc(sizeof(btree_node)); 
   			T->root=s; 
  		 	s -> childrens[0]=r;  
    		split_child(s,0,r); 
        	insert_nonfull(s,key);
 	 	}else{    
      		insert_nonfull(r,key);       
     }
   }
}




// 中序遍历B树
void inorder_traversal_b_tree(btree_node* x) {
    int i;

    for (i = 0; i < x->num; i++) {
        if (!x->leaf) {
            inorder_traversal_b_tree(x->childrens[i]);
        }        
        printf("%d ", x->keys[i]);       
    }

    if (!x->leaf) {
        inorder_traversal_b_tree(x->childrens[i]);
    }
}


int main() {
    b_tree* tree = create_b_tree();

	//KEY_VALUE keys[] = { {5}, {8}, {10}, {3}, {12},{1} ,{55},{45},{98},{888}};
    //int num_keys = sizeof(keys) / sizeof(KEY_VALUE);

	printf("请输入你要插入的数据的个数： \n");
	int n=0;
	scanf("%d",&n);
    int *keys = (int*)malloc(n * sizeof(int));

	printf("请输入你要插入的数据 : \n");
	for(int i=0;i<n;i++){
		scanf("%d",&keys[i]);
	}

    for (int i = 0; i < n; i++) {
        insert_b_tree(tree, keys[i]);
    }

	printf("请输入要查询的数据 : \n");
	int num;
	scanf("%d",&num);
	int ret;
	search_key(tree->root,num,&ret);
	if(ret!=-1){
		printf("查到这个节点了！\n");
	}else{
		printf("没有这个节点！\n");
	}



    printf("中序遍历B树结果: "); 
    inorder_traversal_b_tree(tree->root);
	printf("\n");
	return 0;
}
