#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#define DEGREE		3
typedef int KEY_VALUE;

typedef struct _btree_node {
	KEY_VALUE *keys;
	struct _btree_node **childrens;
	int num;
	int leaf;
} btree_node;

typedef struct _btree {
	btree_node *root;
	int t;
} btree;

btree_node *btree_create_node(int t, int leaf) {

	btree_node *node = (btree_node*)calloc(1, sizeof(btree_node));
	if (node == NULL) assert(0);

	node->leaf = leaf;
	node->keys = (KEY_VALUE*)calloc(1, (2*t-1)*sizeof(KEY_VALUE));
	node->childrens = (btree_node**)calloc(1, (2*t) * sizeof(btree_node*));
	node->num = 0;

	return node;
}

void btree_destroy_node(btree_node *node) {

	assert(node);

	free(node->childrens);
	free(node->keys);
	free(node);
	
}


void btree_create(btree *T, int t) {
	T->t = t;
	
	btree_node *x = btree_create_node(t, 1);
	T->root = x;
	
}

//////////////////////////////////////////////////////////////插入

void btree_split_child(btree *T, btree_node *x, int i) {
	int t = T->t;

	btree_node *y = x->childrens[i];
	btree_node *z = btree_create_node(t, y->leaf);

	z->num = t - 1;

	int j = 0;
	for (j = 0;j < t-1;j ++) {
		z->keys[j] = y->keys[j+t];
	}
	if (y->leaf == 0) {
		for (j = 0;j < t;j ++) {
			z->childrens[j] = y->childrens[j+t];
		}
	}

	y->num = t - 1;
	for (j = x->num;j >= i+1;j --) {
		x->childrens[j+1] = x->childrens[j];
	}

	x->childrens[i+1] = z;

	for (j = x->num-1;j >= i;j --) {
		x->keys[j+1] = x->keys[j];
	}
	x->keys[i] = y->keys[t-1];
	x->num += 1;
	
}

void btree_insert_nonfull(btree *T, btree_node *x, KEY_VALUE k) {

	int i = x->num - 1;

	if (x->leaf == 1) {
		
		while (i >= 0 && x->keys[i] > k) {
			x->keys[i+1] = x->keys[i];
			i --;
		}
		x->keys[i+1] = k;
		x->num += 1;
		
	} else {
		while (i >= 0 && x->keys[i] > k) i --;

		if (x->childrens[i+1]->num == (2*(T->t))-1) {
			btree_split_child(T, x, i+1);
			if (k > x->keys[i+1]) i++;
		}

		btree_insert_nonfull(T, x->childrens[i+1], k);
	}
}

void btree_insert(btree *T, KEY_VALUE key) {
	//int t = T->t;

	btree_node *r = T->root;
	if (r->num == 2 * T->t - 1) {
		
		btree_node *node = btree_create_node(T->t, 0);
		T->root = node;

		node->childrens[0] = r;

		btree_split_child(T, node, 0);

		int i = 0;
		if (node->keys[0] < key) i++;
		btree_insert_nonfull(T, node->childrens[i], key);
		
	} else {
		btree_insert_nonfull(T, r, key);
	}
}


////////////////////////////////////////////////////////////////////////遍历
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

/////////////////////////////////////////////////////////////////////////查找

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

///////////////////////////////////////////////////////////////////////////删除
//{child[idx], key[idx], child[idx+1]} 
void btree_merge(btree *T, btree_node *node, int idx) {

	btree_node *left = node->childrens[idx];
	btree_node *right = node->childrens[idx+1];

	int i = 0;

	/////data merge
	left->keys[T->t-1] = node->keys[idx];
	for (i = 0;i < T->t-1;i ++) {
		left->keys[T->t+i] = right->keys[i];
	}
	if (!left->leaf) {
		for (i = 0;i < T->t;i ++) {
			left->childrens[T->t+i] = right->childrens[i];
		}
	}
	left->num += T->t;

	//destroy right
	btree_destroy_node(right);

	//node 
	for (i = idx+1;i < node->num;i ++) {
		node->keys[i-1] = node->keys[i];
		node->childrens[i] = node->childrens[i+1];
	}
	node->childrens[i+1] = NULL;
	node->num -= 1;

	if (node->num == 0) {
		T->root = left;
		btree_destroy_node(node);
	}
}

void btree_delete_key(btree *T, btree_node *node, KEY_VALUE key) {

	if (node == NULL) return ;

	int idx = 0, i;

	while (idx < node->num && key > node->keys[idx]) {
		idx ++;
	}

	if (idx < node->num && key == node->keys[idx]) {

		if (node->leaf) {
			
			for (i = idx;i < node->num-1;i ++) {
				node->keys[i] = node->keys[i+1];
			}

			node->keys[node->num - 1] = 0;
			node->num--;
			
			if (node->num == 0) { //root
				free(node);
				T->root = NULL;
			}

			return ;
		} else if (node->childrens[idx]->num >= T->t) {

			btree_node *left = node->childrens[idx];
			node->keys[idx] = left->keys[left->num - 1];

			btree_delete_key(T, left, left->keys[left->num - 1]);
			
		} else if (node->childrens[idx+1]->num >= T->t) {

			btree_node *right = node->childrens[idx+1];
			node->keys[idx] = right->keys[0];

			btree_delete_key(T, right, right->keys[0]);
			
		} else {

			btree_merge(T, node, idx);
			btree_delete_key(T, node->childrens[idx], key);
			
		}
		
	} else {

		btree_node *child = node->childrens[idx];
		if (child == NULL) {
			printf("Cannot del key = %d\n", key);
			return ;
		}

		if (child->num == T->t - 1) {

			btree_node *left = NULL;
			btree_node *right = NULL;
			if (idx - 1 >= 0)
				left = node->childrens[idx-1];
			if (idx + 1 <= node->num) 
				right = node->childrens[idx+1];

			if ((left && left->num >= T->t) ||
				(right && right->num >= T->t)) {

				int richR = 0;
				if (right) richR = 1;
				if (left && right) richR = (right->num > left->num) ? 1 : 0;

				if (right && right->num >= T->t && richR) { //borrow from next
					child->keys[child->num] = node->keys[idx];
					child->childrens[child->num+1] = right->childrens[0];
					child->num ++;

					node->keys[idx] = right->keys[0];
					for (i = 0;i < right->num - 1;i ++) {
						right->keys[i] = right->keys[i+1];
						right->childrens[i] = right->childrens[i+1];
					}

					right->keys[right->num-1] = 0;
					right->childrens[right->num-1] = right->childrens[right->num];
					right->childrens[right->num] = NULL;
					right->num --;
					
				} else { //borrow from prev

					for (i = child->num;i > 0;i --) {
						child->keys[i] = child->keys[i-1];
						child->childrens[i+1] = child->childrens[i];
					}

					child->childrens[1] = child->childrens[0];
					child->childrens[0] = left->childrens[left->num];
					child->keys[0] = node->keys[idx-1];
					
					child->num ++;

					node->keys[idx-1] = left->keys[left->num-1];
					left->keys[left->num-1] = 0;
					left->childrens[left->num] = NULL;
					left->num --;
				}

			} else if ((!left || (left->num == T->t - 1))
				&& (!right || (right->num == T->t - 1))) {

				if (left && left->num == T->t - 1) {
					btree_merge(T, node, idx-1);					
					child = left;
				} else if (right && right->num == T->t - 1) {
					btree_merge(T, node, idx);
				}
			}
		}

		btree_delete_key(T, child, key);
	}
	
}


int btree_delete(btree *T, KEY_VALUE key) {
	if (!T->root) return -1;

	btree_delete_key(T, T->root, key);
	return 0;
}
//////////////////////////////////////////////

int main() {
	btree T = {0};
	btree_create(&T, 3);

    //插入
    printf("请输入要插入的数据的个数 : \n");
    int n;
    scanf("%d",&n);
    printf("请输入要插入的元素 : \n");

	int i = 0;
	int key;
	for (i = 0;i < n;i ++) {
		scanf("%d", &key);
		btree_insert(&T, key);
	}
   
    //删除
    printf("请输入要删除的个数 :\n");
    int m;
    scanf("%d",&m);
    if(m>=n){
        printf("error");
        exit(1);
    }
    printf("请输入要删除的数据 :\n");
    int del;
    for(i=0;i<m;i++){
        scanf("%d",&del);
        btree_delete(&T, del);
    }

    //查询
    printf("请输入要查询的数据 :\n");
    int sel;
    scanf("%d",&sel);
    int index;
    search_key(T.root,sel,&index);
    if(index==-1){
        printf("没有查到!\n");
    }else{
        printf("查到了!\n");
    }

    //遍历
	inorder_traversal_b_tree(T.root);
    printf("\n---------------------------------\n");

    return 0;
}
